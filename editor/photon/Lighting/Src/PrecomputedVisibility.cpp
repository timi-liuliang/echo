/*=============================================================================
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "LightingSystem.h"
#include "MonteCarlo.h"
#include "../Inc/Raster.h"

namespace Lightmass
{

struct FVisibilitySample
{
	FVector4 Position;
};

struct FCellHeights
{
	TArray<FVisibilitySample> Heights;
};

IMPLEMENT_COMPARE_CONSTREF( FVisibilitySample, PrecomputedVisibility, { return Sgn(A.Position.Z - B.Position.Z); } )

class FCellToHeightsMap
{
public:

	/** Initialization constructor. */
	FCellToHeightsMap(INT InSizeX,INT InSizeY):
		Data(InSizeX * InSizeY),
		SizeX(InSizeX),
		SizeY(InSizeY)
	{
		// Clear the map to zero.
		for(INT Y = 0;Y < SizeY;Y++)
		{
			for(INT X = 0;X < SizeX;X++)
			{
				FCellHeights& CurrentCell = (*this)(X,Y);
				appMemzero(&CurrentCell,sizeof(FCellHeights));
				CurrentCell.Heights.Empty(50);
			}
		}
	}

	// Accessors.
	FCellHeights& operator()(INT X,INT Y)
	{
		const UINT TexelIndex = Y * SizeX + X;
		return Data(TexelIndex);
	}
	const FCellHeights& operator()(INT X,INT Y) const
	{
		const INT TexelIndex = Y * SizeX + X;
		return Data(TexelIndex);
	}

	INT GetSizeX() const { return SizeX; }
	INT GetSizeY() const { return SizeY; }
	SIZE_T GetAllocatedSize() const { return Data.GetAllocatedSize(); }

private:

	/** The mapping data. */
	TChunkedArray<FCellHeights> Data;

	/** The width of the mapping data. */
	INT SizeX;

	/** The height of the mapping data. */
	INT SizeY;
};
	
class FCellPlacementRasterPolicy
{
public:

	typedef FVector4 InterpolantType;

	/** Initialization constructor. */
	FCellPlacementRasterPolicy(
		FCellToHeightsMap& InHeightsMap,
		const FScene& InScene, 
		FStaticLightingSystem& InSystem)
		:
		HeightsMap(InHeightsMap),
		Scene(InScene),
		System(InSystem)
	{
	}

protected:

	// FTriangleRasterizer policy interface.

	INT GetMinX() const { return 0; }
	INT GetMaxX() const { return HeightsMap.GetSizeX(); }
	INT GetMinY() const { return 0; }
	INT GetMaxY() const { return HeightsMap.GetSizeY(); }

	void ProcessPixel(INT X,INT Y,const InterpolantType& Interpolant,UBOOL BackFacing);

private:

	FCellToHeightsMap& HeightsMap;
	const FScene& Scene;
	FStaticLightingSystem& System;
};

void FCellPlacementRasterPolicy::ProcessPixel(INT X,INT Y,const InterpolantType& WorldPosition,UBOOL BackFacing)
{
	if (Scene.IsPointInVisibilityVolume(WorldPosition))
	{
		FCellHeights& Cell = HeightsMap(X, Y);
		FVisibilitySample Sample;
		Sample.Position = WorldPosition;
		Cell.Heights.AddItem(Sample);
	}
}

/** Determines visibility cell placement, called once at startup. */
void FStaticLightingSystem::SetupPrecomputedVisibility()
{
	const DOUBLE StartTime = appSeconds();
	FRandomStream RandomStream(0);

	const FBoxSphereBounds PrecomputedVisibilityBounds = Scene.GetVisibilityVolumeBounds();
	const FVector4 VolumeSizes = PrecomputedVisibilityBounds.BoxExtent * 2.0f / PrecomputedVisibilitySettings.CellSize;
	const INT SizeX = appTrunc(VolumeSizes.X + DELTA);
	const INT SizeY = appTrunc(VolumeSizes.Y + DELTA);
	const INT SizeZ = appTrunc(PrecomputedVisibilityBounds.BoxExtent.Z * 2.0f / PrecomputedVisibilitySettings.PlayAreaHeight + DELTA);

	if (PrecomputedVisibilitySettings.bPlaceCellsOnSurfaces)
	{
		FCellToHeightsMap HeightsMap(SizeX, SizeY);
		FTriangleRasterizer<FCellPlacementRasterPolicy> Rasterizer(
			FCellPlacementRasterPolicy(
			HeightsMap,
			Scene, 
			*this));

		check(Meshes.Num() == AllMappings.Num());

		// Rasterize the scene to determine potential cell heights
		for (INT MappingIndex = 0; MappingIndex < AllMappings.Num(); MappingIndex++)
		{
			const FStaticLightingMapping* CurrentMapping = AllMappings(MappingIndex);
			const FStaticLightingMesh* CurrentMesh = CurrentMapping->Mesh;

			// Rasterize all triangles in the mesh
			for (INT TriangleIndex = 0; TriangleIndex < CurrentMesh->NumTriangles; TriangleIndex++)
			{
				FStaticLightingVertex Vertices[3];
				INT ElementIndex;
				CurrentMesh->GetTriangle(TriangleIndex, Vertices[0], Vertices[1], Vertices[2], ElementIndex);

				// Only place cells on opaque surfaces if requested, which can save some memory for foliage maps
				if (!PrecomputedVisibilitySettings.bPlaceCellsOnOpaqueOnly 
					|| !CurrentMesh->IsMasked(ElementIndex) && !CurrentMesh->IsTranslucent(ElementIndex))
				{
					FVector2D XYPositions[3];
					for (INT VertIndex = 0; VertIndex < 3; VertIndex++)
					{
						// Transform world space positions from [PrecomputedVisibilityBounds.Origin - PrecomputedVisibilityBounds.BoxExtent, PrecomputedVisibilityBounds.Origin + PrecomputedVisibilityBounds.BoxExtent] into [0,1]
						const FVector4 TransformedPosition = (Vertices[VertIndex].WorldPosition - PrecomputedVisibilityBounds.Origin + PrecomputedVisibilityBounds.BoxExtent) / (2.0f * PrecomputedVisibilityBounds.BoxExtent);
						// Project positions onto the XY plane
						XYPositions[VertIndex] = FVector2D(TransformedPosition.X * SizeX, TransformedPosition.Y * SizeY);
					}

					const FVector4 TriangleNormal = (Vertices[2].WorldPosition - Vertices[0].WorldPosition) ^ (Vertices[1].WorldPosition - Vertices[0].WorldPosition);

					// Only rasterize upward facing triangles
					if (TriangleNormal.Z > 0.0f)
					{
						Rasterizer.DrawTriangle(
							Vertices[0].WorldPosition,
							Vertices[1].WorldPosition,
							Vertices[2].WorldPosition,
							XYPositions[0],
							XYPositions[1],
							XYPositions[2],
							FALSE
							);
					}
				}
			}
		}

		AllPrecomputedVisibilityCells.Empty(SizeX * SizeY * 2);
		TArray<FLOAT> FirstPassPlacedHeights;
		for (INT Y = 0; Y < SizeY; Y++)
		{
			for (INT X = 0; X < SizeX; X++)
			{
				FCellHeights& Cell = HeightsMap(X, Y);
				// Sort the heights from smallest to largest
				Sort<FVisibilitySample,COMPARE_CONSTREF_CLASS(FVisibilitySample,PrecomputedVisibility)>(Cell.Heights.GetData(), Cell.Heights.Num());
				FLOAT LastSampleHeight = -FLT_MAX;

				FirstPassPlacedHeights.Reset();
				// Pass 1 - only place cells in the largest holes which are most likely to be where the play area is
				for (INT HeightIndex = 0; HeightIndex < Cell.Heights.Num(); HeightIndex++)
				{
					const FVector4& CurrentPosition = Cell.Heights(HeightIndex).Position;
					// Place a new cell if this is the highest height
					if (HeightIndex + 1 == Cell.Heights.Num()
						// Or if there's a gap above this height of size PlayAreaHeight
						|| (Cell.Heights(HeightIndex + 1).Position.Z - CurrentPosition.Z) > PrecomputedVisibilitySettings.PlayAreaHeight
						// And this height is not within a cell that was just placed
						&& CurrentPosition.Z - LastSampleHeight > PrecomputedVisibilitySettings.PlayAreaHeight)
					{
						FPrecomputedVisibilityCell NewCell;
						NewCell.Bounds = FBox(
							FVector4(
								CurrentPosition.X - PrecomputedVisibilitySettings.CellSize / 2,
								CurrentPosition.Y - PrecomputedVisibilitySettings.CellSize / 2,
								// Place the bottom slightly above the surface, since cells that clip through the floor often have poor occlusion culling
								//@todo - determine the underlying slope and place the cell high enough that it never clips
								CurrentPosition.Z + .1f * PrecomputedVisibilitySettings.PlayAreaHeight),
							FVector4(
								CurrentPosition.X + PrecomputedVisibilitySettings.CellSize / 2,
								CurrentPosition.Y + PrecomputedVisibilitySettings.CellSize / 2,
								CurrentPosition.Z + 1.1f * PrecomputedVisibilitySettings.PlayAreaHeight));

						AllPrecomputedVisibilityCells.AddItem(NewCell);
						LastSampleHeight = CurrentPosition.Z;
						FirstPassPlacedHeights.AddItem(CurrentPosition.Z);
					}
				}

				LastSampleHeight = -FLT_MAX;
				// Pass 2 - place cells in smaller holes
				for (INT HeightIndex = 0; HeightIndex < Cell.Heights.Num() - 1; HeightIndex++)
				{
					const FVector4& CurrentPosition = Cell.Heights(HeightIndex).Position;
					if (Cell.Heights(HeightIndex + 1).Position.Z - CurrentPosition.Z > PrecomputedVisibilitySettings.PlayAreaHeight / 2.0f
						&& CurrentPosition.Z - LastSampleHeight > PrecomputedVisibilitySettings.PlayAreaHeight)
					{
						UBOOL bShouldPlaceCell = TRUE;
						for (INT FirstPassHeightIndex = 0; FirstPassHeightIndex < FirstPassPlacedHeights.Num(); FirstPassHeightIndex++)
						{
							// Don't place a new cell if the height intersects with an existing cell
							if (!(CurrentPosition.Z + PrecomputedVisibilitySettings.PlayAreaHeight < FirstPassPlacedHeights(FirstPassHeightIndex) 
								|| CurrentPosition.Z > FirstPassPlacedHeights(FirstPassHeightIndex) + PrecomputedVisibilitySettings.PlayAreaHeight))
							{
								bShouldPlaceCell = FALSE;
								break;
							}
						}
						if (bShouldPlaceCell)
						{
							FPrecomputedVisibilityCell NewCell;
							NewCell.Bounds = FBox(
								FVector4(
									CurrentPosition.X - PrecomputedVisibilitySettings.CellSize / 2,
									CurrentPosition.Y - PrecomputedVisibilitySettings.CellSize / 2,
									CurrentPosition.Z + .1f * PrecomputedVisibilitySettings.PlayAreaHeight),
								FVector4(
									CurrentPosition.X + PrecomputedVisibilitySettings.CellSize / 2,
									CurrentPosition.Y + PrecomputedVisibilitySettings.CellSize / 2,
									CurrentPosition.Z + 1.1f * PrecomputedVisibilitySettings.PlayAreaHeight));

							AllPrecomputedVisibilityCells.AddItem(NewCell);
							LastSampleHeight = CurrentPosition.Z;
						}
					}
				}
			}
		}
	}
	else
	{
		// Place cells in a 3d grid inside the volumes
		AllPrecomputedVisibilityCells.Empty(SizeX * SizeY * SizeZ);
		const FVector4 BoundsMin = PrecomputedVisibilityBounds.Origin - PrecomputedVisibilityBounds.BoxExtent;
		for (INT CellZ = 0; CellZ < SizeZ; CellZ++)
		{
			for (INT CellY = 0; CellY < SizeY; CellY++)
			{
				for (INT CellX = 0; CellX < SizeX; CellX++)
				{
					const FVector4 CurrentPosition(
						BoundsMin.X + CellX * PrecomputedVisibilitySettings.CellSize,
						BoundsMin.Y + CellY * PrecomputedVisibilitySettings.CellSize,
						BoundsMin.Z + CellZ * PrecomputedVisibilitySettings.PlayAreaHeight);
					if (Scene.IsPointInVisibilityVolume(CurrentPosition))
					{
						FPrecomputedVisibilityCell NewCell;
						NewCell.Bounds = FBox(
							FVector4(
								CurrentPosition.X - PrecomputedVisibilitySettings.CellSize / 2,
								CurrentPosition.Y - PrecomputedVisibilitySettings.CellSize / 2,
								CurrentPosition.Z),
							FVector4(
								CurrentPosition.X + PrecomputedVisibilitySettings.CellSize / 2,
								CurrentPosition.Y + PrecomputedVisibilitySettings.CellSize / 2,
								CurrentPosition.Z + PrecomputedVisibilitySettings.PlayAreaHeight));

						AllPrecomputedVisibilityCells.AddItem(NewCell);
					}
				}
			}
		}
	}

	// Place cells along camera tracks
	const INT NumCellsPlacedOnSurfaces = AllPrecomputedVisibilityCells.Num();
	for (INT CameraPositionIndex = 0; CameraPositionIndex < Scene.CameraTrackPositions.Num(); CameraPositionIndex++)
	{
		const FVector4& CurrentPosition = Scene.CameraTrackPositions(CameraPositionIndex);
		UBOOL bInsideCell = FALSE;
		for (INT CellIndex = 0; CellIndex < AllPrecomputedVisibilityCells.Num(); CellIndex++)
		{
			if (AllPrecomputedVisibilityCells(CellIndex).Bounds.IsInside(CurrentPosition))
			{
				bInsideCell = TRUE;
				break;
			}
		}

		if (!bInsideCell)
		{
			FPrecomputedVisibilityCell NewCell;

			// Snap the cell min to the nearest factor of CellSize from the visibility bounds min + CellSize / 2
			// The CellSize / 2 offset is necessary to match up with cells produced by the rasterizer, since pixels are rasterized at cell centers
			const FVector4 PreSnapTranslation = PrecomputedVisibilitySettings.CellSize / 2 + PrecomputedVisibilityBounds.Origin - PrecomputedVisibilityBounds.BoxExtent;
			const FVector4 TranslatedPosition = CurrentPosition - PreSnapTranslation;
			// appFmod gives the offset to round up for negative numbers, when we always want the offset to round down
			const FLOAT XOffset = TranslatedPosition.X > 0.0f ? 
				appFmod(TranslatedPosition.X, PrecomputedVisibilitySettings.CellSize) : 
				PrecomputedVisibilitySettings.CellSize - appFmod(-TranslatedPosition.X, PrecomputedVisibilitySettings.CellSize);
			const FLOAT YOffset = TranslatedPosition.Y > 0.0f ? 
				appFmod(TranslatedPosition.Y, PrecomputedVisibilitySettings.CellSize) : 
				PrecomputedVisibilitySettings.CellSize - appFmod(-TranslatedPosition.Y, PrecomputedVisibilitySettings.CellSize);
			const FVector4 SnappedPosition(CurrentPosition.X - XOffset, CurrentPosition.Y - YOffset, CurrentPosition.Z);

			NewCell.Bounds = FBox(
				FVector4(
					SnappedPosition.X,
					SnappedPosition.Y,
					SnappedPosition.Z - .5f * PrecomputedVisibilitySettings.PlayAreaHeight),
				FVector4(
					SnappedPosition.X + PrecomputedVisibilitySettings.CellSize,
					SnappedPosition.Y + PrecomputedVisibilitySettings.CellSize,
					SnappedPosition.Z + .5f * PrecomputedVisibilitySettings.PlayAreaHeight));

			AllPrecomputedVisibilityCells.AddItem(NewCell);

			// Verify that the camera track position is inside the placed cell
			checkSlow(NewCell.Bounds.IsInside(CurrentPosition));
		}
	}

	const SIZE_T NumVisDataBytes = AllPrecomputedVisibilityCells.Num() * VisibilityMeshes.Num() / 8;
	Stats.NumPrecomputedVisibilityCellsTotal = NumCellsPlacedOnSurfaces;
	Stats.NumPrecomputedVisibilityCellsCamaraTracks = AllPrecomputedVisibilityCells.Num() - NumCellsPlacedOnSurfaces;
	Stats.NumPrecomputedVisibilityMeshes = VisibilityMeshes.Num();
	Stats.PrecomputedVisibilityDataBytes = NumVisDataBytes;

	//LogSolverMessage(FString::Printf(TEXT("Setup vis %.1fs"), appSeconds() - StartTime));
}

static UBOOL IsMeshVisible(const TArray<BYTE>& VisibilityData, INT MeshId)
{
	return (VisibilityData(MeshId / 8) & 1 << (MeshId % 8)) != 0;
}

static void SetMeshVisible(TArray<BYTE>& VisibilityData, INT MeshId)
{
	VisibilityData(MeshId / 8) |= 1 << (MeshId % 8);
}

class FAxisAlignedCellFace
{
public:

	FAxisAlignedCellFace() {}

	FAxisAlignedCellFace(const FVector4& InFaceDirection, const FVector4& InFaceMin, const FVector4& InFaceExtent) :
		FaceDirection(InFaceDirection),
		FaceMin(InFaceMin),
		FaceExtent(InFaceExtent)
	{}

	FVector4 FaceDirection;
	FVector4 FaceMin;
	FVector4 FaceExtent;
};

/** Stores information about a single query sample between a visibility cell and a mesh. */
struct FVisibilityQuerySample
{
	/** Sample position generated from the mesh. */
	FVector4 MeshPosition;

	/** Sample position generated from the cell. */
	FVector4 CellPosition;

	/** Position of the intersection with the scene between the sample positions. */
	FVector4 IntersectionPosition;

	/** Distance along the vector perpendicular to the mesh->cell vector. */
	FLOAT PerpendicularDistance;
};

/** Sorts two samples based on perpendicular distance. */
IMPLEMENT_COMPARE_CONSTREF(FVisibilityQuerySample, PrecomputedVisibility, { return Sgn( A.PerpendicularDistance - B.PerpendicularDistance ); })

/** Calculates visibility for a given group of cells, called from all threads. */
void FStaticLightingSystem::CalculatePrecomputedVisibility(INT BucketIndex)
{
	const DOUBLE StartTime = appSeconds();
	check(BucketIndex >= 0 && BucketIndex < PrecomputedVisibilitySettings.NumCellDistributionBuckets);
	// Create a new link for the output of this task
	TList<FPrecomputedVisibilityData>* DataLink = new TList<FPrecomputedVisibilityData>(FPrecomputedVisibilityData(),NULL);
	DataLink->Element.Guid = Scene.VisibilityBucketGuids(BucketIndex);
	
	// Determine the range of cells to process from the bucket index
	const INT StartCellIndex = BucketIndex * AllPrecomputedVisibilityCells.Num() / PrecomputedVisibilitySettings.NumCellDistributionBuckets;
	const INT MaxCellIndex = BucketIndex + 1 == PrecomputedVisibilitySettings.NumCellDistributionBuckets ? 
		// Last bucket processes up to the end of the array
		AllPrecomputedVisibilityCells.Num() :
		(BucketIndex + 1) * AllPrecomputedVisibilityCells.Num() / PrecomputedVisibilitySettings.NumCellDistributionBuckets;

	DataLink->Element.PrecomputedVisibilityCells.Empty(MaxCellIndex - StartCellIndex);
	
	FStaticLightingMappingContext MappingContext(NULL, *this);

	TArray<const FPrecomputedVisibilityOverrideVolume*> AffectingOverrideVolumes;
	for (INT CellIndex = StartCellIndex; CellIndex < MaxCellIndex; CellIndex++)
	{
		const DOUBLE StartSampleTime = appSeconds();

		// Seed by absolute cell index for deterministic results regardless of how cell tasks are distributed
		FRandomStream RandomStream(CellIndex);

		// Reset cached information for this cell so that traces won't be affected by what previous cells did
		MappingContext.RayCache.Clear();

		DataLink->Element.PrecomputedVisibilityCells.AddItem(AllPrecomputedVisibilityCells(CellIndex));
		FPrecomputedVisibilityCell& CurrentCell = DataLink->Element.PrecomputedVisibilityCells.Last();

		const UBOOL bDebugThisCell = CurrentCell.Bounds.IsInside(Scene.DebugInput.CameraPosition) && PrecomputedVisibilitySettings.bVisualizePrecomputedVisibility;

		AffectingOverrideVolumes.Reset();
		for (INT VolumeIndex = 0; VolumeIndex < Scene.PrecomputedVisibilityOverrideVolumes.Num(); VolumeIndex++)
		{
			if (Scene.PrecomputedVisibilityOverrideVolumes(VolumeIndex).Bounds.Intersect(CurrentCell.Bounds))
			{
				AffectingOverrideVolumes.AddItem(&Scene.PrecomputedVisibilityOverrideVolumes(VolumeIndex));
			}
		}

		CurrentCell.VisibilityData.Empty(VisibilityMeshes.Num() / 8 + 1);
		CurrentCell.VisibilityData.AddZeroed(VisibilityMeshes.Num() / 8 + 1);

		FAxisAlignedCellFace CellFaces[6];
		CellFaces[0] = FAxisAlignedCellFace(FVector4(-1, 0, 0), FVector4(CurrentCell.Bounds.Min.X, CurrentCell.Bounds.Min.Y, CurrentCell.Bounds.Min.Z), FVector4(0, PrecomputedVisibilitySettings.CellSize, PrecomputedVisibilitySettings.PlayAreaHeight));
		CellFaces[1] = FAxisAlignedCellFace(FVector4(1, 0, 0), FVector4(CurrentCell.Bounds.Max.X, CurrentCell.Bounds.Min.Y, CurrentCell.Bounds.Min.Z), FVector4(0, PrecomputedVisibilitySettings.CellSize, PrecomputedVisibilitySettings.PlayAreaHeight));
		CellFaces[2] = FAxisAlignedCellFace(FVector4(0, -1, 0), FVector4(CurrentCell.Bounds.Min.X, CurrentCell.Bounds.Min.Y, CurrentCell.Bounds.Min.Z), FVector4(PrecomputedVisibilitySettings.CellSize, 0, PrecomputedVisibilitySettings.PlayAreaHeight));
		CellFaces[3] = FAxisAlignedCellFace(FVector4(0, 1, 0), FVector4(CurrentCell.Bounds.Min.X, CurrentCell.Bounds.Max.Y, CurrentCell.Bounds.Min.Z), FVector4(PrecomputedVisibilitySettings.CellSize, 0, PrecomputedVisibilitySettings.PlayAreaHeight));
		CellFaces[4] = FAxisAlignedCellFace(FVector4(0, 0, -1), FVector4(CurrentCell.Bounds.Min.X, CurrentCell.Bounds.Min.Y, CurrentCell.Bounds.Min.Z), FVector4(PrecomputedVisibilitySettings.CellSize, PrecomputedVisibilitySettings.CellSize, 0));
		CellFaces[5] = FAxisAlignedCellFace(FVector4(0, 0, 1), FVector4(CurrentCell.Bounds.Min.X, CurrentCell.Bounds.Min.Y, CurrentCell.Bounds.Max.Z), FVector4(PrecomputedVisibilitySettings.CellSize, PrecomputedVisibilitySettings.CellSize, 0));

		const FVector4 CenterCellPosition = .5f * (CurrentCell.Bounds.Min + CurrentCell.Bounds.Max);

		TArray<INT> VisibleCellFaces;
		TArray<FLOAT> VisibleCellFacePDFs;
		TArray<FLOAT> VisibleCellFaceCDFs;
		TArray<INT> VisibleMeshFaces;
		TArray<FVisibilityQuerySample> SamplePositions;
		TArray<const FVisibilityQuerySample*> FurthestSamples;
		for (INT VisibilityMeshIndex = 0; VisibilityMeshIndex < VisibilityMeshes.Num(); VisibilityMeshIndex++)
		{
			const DOUBLE SampleGenerationStartTime = appSeconds();
			FBox OriginalMeshBounds(0);
			// Combine mesh bounds, usually only BSP has multiple meshes per VisibilityId
			//@todo - could explicitly sample each bounds separately, but they tend to be pretty close together in world space
			for (INT OriginalMeshIndex = 0; OriginalMeshIndex < VisibilityMeshes(VisibilityMeshIndex).Num(); OriginalMeshIndex++)
			{
				OriginalMeshBounds += VisibilityMeshes(VisibilityMeshIndex)(OriginalMeshIndex)->BoundingBox;
			}

			const FBox MeshBox(
				OriginalMeshBounds.GetCenter() - OriginalMeshBounds.GetExtent() * PrecomputedVisibilitySettings.MeshBoundsScale,
				OriginalMeshBounds.GetCenter() + OriginalMeshBounds.GetExtent() * PrecomputedVisibilitySettings.MeshBoundsScale);

			const FVector4 MeshToCellCenter = CenterCellPosition - MeshBox.GetCenter();
			const FLOAT Distance = MeshToCellCenter.Size();
			const FVector4 MeshBoxExtent = MeshBox.GetExtent() * 2;

			const UBOOL bDebugThisMesh = VisibilityMeshIndex == Scene.DebugInput.DebugVisibilityId && bDebugThisCell;

			if (bDebugThisMesh)
			{
				// Draw the bounding boxes of each mesh and the combined bounds
				if (VisibilityMeshes(VisibilityMeshIndex).Num() > 1)
				{
					for (INT OriginalMeshIndex = 0; OriginalMeshIndex < VisibilityMeshes(VisibilityMeshIndex).Num(); OriginalMeshIndex++)
					{
						const FVector4 Min = VisibilityMeshes(VisibilityMeshIndex)(OriginalMeshIndex)->BoundingBox.Min;
						const FVector4 Max = VisibilityMeshes(VisibilityMeshIndex)(OriginalMeshIndex)->BoundingBox.Max;
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Min.Z), FVector4(Min.X, Max.Y, Min.Z), FALSE));
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Max.Y, Min.Z), FVector4(Min.X, Max.Y, Max.Z), FALSE));
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Min.Z), FVector4(Min.X, Min.Y, Max.Z), FALSE));
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Max.Z), FVector4(Min.X, Max.Y, Max.Z), FALSE));

						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Min.Y, Min.Z), FVector4(Max.X, Max.Y, Min.Z), FALSE));
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Max.Y, Min.Z), FVector4(Max.X, Max.Y, Max.Z), FALSE));
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Min.Y, Min.Z), FVector4(Max.X, Min.Y, Max.Z), FALSE));
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Min.Y, Max.Z), FVector4(Max.X, Max.Y, Max.Z), FALSE));

						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Min.Z), FVector4(Max.X, Min.Y, Min.Z), FALSE));
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Max.Z), FVector4(Max.X, Min.Y, Max.Z), FALSE));
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Max.Y, Min.Z), FVector4(Max.X, Max.Y, Min.Z), FALSE));
						DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Max.Y, Max.Z), FVector4(Max.X, Max.Y, Max.Z), FALSE));
					}
				}
				
				const FVector4 Min = MeshBox.Min;
				const FVector4 Max = MeshBox.Max;
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Min.Z), FVector4(Min.X, Max.Y, Min.Z), TRUE));
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Max.Y, Min.Z), FVector4(Min.X, Max.Y, Max.Z), TRUE));
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Min.Z), FVector4(Min.X, Min.Y, Max.Z), TRUE));
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Max.Z), FVector4(Min.X, Max.Y, Max.Z), TRUE));

				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Min.Y, Min.Z), FVector4(Max.X, Max.Y, Min.Z), TRUE));
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Max.Y, Min.Z), FVector4(Max.X, Max.Y, Max.Z), TRUE));
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Min.Y, Min.Z), FVector4(Max.X, Min.Y, Max.Z), TRUE));
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Min.Y, Max.Z), FVector4(Max.X, Max.Y, Max.Z), TRUE));

				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Min.Z), FVector4(Max.X, Min.Y, Min.Z), TRUE));
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Max.Z), FVector4(Max.X, Min.Y, Max.Z), TRUE));
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Max.Y, Min.Z), FVector4(Max.X, Max.Y, Min.Z), TRUE));
				DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Max.Y, Max.Z), FVector4(Max.X, Max.Y, Max.Z), TRUE));
			}

			FAxisAlignedCellFace MeshBoxFaces[6];
			MeshBoxFaces[0] = FAxisAlignedCellFace(FVector4(-1, 0, 0), FVector4(MeshBox.Min.X, MeshBox.Min.Y, MeshBox.Min.Z), FVector4(0, MeshBoxExtent.Y, MeshBoxExtent.Z));
			MeshBoxFaces[1] = FAxisAlignedCellFace(FVector4(1, 0, 0), FVector4(MeshBox.Min.X + MeshBoxExtent.X, MeshBox.Min.Y, MeshBox.Min.Z), FVector4(0, MeshBoxExtent.Y, MeshBoxExtent.Z));
			MeshBoxFaces[2] = FAxisAlignedCellFace(FVector4(0, -1, 0), FVector4(MeshBox.Min.X, MeshBox.Min.Y, MeshBox.Min.Z), FVector4(MeshBoxExtent.X, 0, MeshBoxExtent.Z));
			MeshBoxFaces[3] = FAxisAlignedCellFace(FVector4(0, 1, 0), FVector4(MeshBox.Min.X, MeshBox.Min.Y + MeshBoxExtent.Y, MeshBox.Min.Z), FVector4(MeshBoxExtent.X, 0, MeshBoxExtent.Z));
			MeshBoxFaces[4] = FAxisAlignedCellFace(FVector4(0, 0, -1), FVector4(MeshBox.Min.X, MeshBox.Min.Y, MeshBox.Min.Z), FVector4(MeshBoxExtent.X, MeshBoxExtent.Y, 0));
			MeshBoxFaces[5] = FAxisAlignedCellFace(FVector4(0, 0, 1), FVector4(MeshBox.Min.X, MeshBox.Min.Y, MeshBox.Min.Z + MeshBoxExtent.Z), FVector4(MeshBoxExtent.X, MeshBoxExtent.Y, 0));

			VisibleCellFaces.Reset();
			VisibleCellFacePDFs.Reset();
			for (INT i = 0; i < 6; i++)
			{
				const FLOAT DotProduct = -(MeshToCellCenter / Distance) | CellFaces[i].FaceDirection;
				if (DotProduct > 0.0f)
				{
					VisibleCellFaces.AddItem(i);
					VisibleCellFacePDFs.AddItem(DotProduct);
				}
			}

			// Ensure that some of the faces will be sampled
			if (VisibleCellFacePDFs.Num() == 0)
			{
				for (INT i = 0; i < 6; i++)
				{
					VisibleCellFaces.AddItem(i);
					VisibleCellFacePDFs.AddItem(i);
				}
			}

			FLOAT UnnormalizedIntegral;
			CalculateStep1dCDF(VisibleCellFacePDFs, VisibleCellFaceCDFs, UnnormalizedIntegral);

			VisibleMeshFaces.Reset();
			for (INT i = 0; i < 6; i++)
			{
				if ((MeshToCellCenter | MeshBoxFaces[i].FaceDirection) > 0.0f)
				{
					VisibleMeshFaces.AddItem(i);
				}
			}

			MappingContext.Stats.NumPrecomputedVisibilityQueries++;

			const FLOAT MeshSize = MeshBox.GetExtent().Size();
			const FLOAT SizeRatio = MeshSize / Distance;
			// Use MaxMeshSamples for meshes with a large projected angle, and MinMeshSamples for meshes with a small projected angle
			// Meshes with a large projected angle require more samples to determine visibility accurately
			const INT NumMeshSamples = Clamp(appTrunc(SizeRatio * PrecomputedVisibilitySettings.MaxMeshSamples), PrecomputedVisibilitySettings.MinMeshSamples, PrecomputedVisibilitySettings.MaxMeshSamples);

			// Treat meshes whose projected angle is greater than 90 degrees as visible, since it becomes overly costly to determine if these are visible
			// (consider a large close mesh that only has a tiny part visible)
			UBOOL bVisible = SizeRatio > 1.0f;
			UBOOL bForceInvisible = FALSE;
			for (INT VolumeIndex = 0; VolumeIndex < AffectingOverrideVolumes.Num(); VolumeIndex++)
			{
				if (AffectingOverrideVolumes(VolumeIndex)->OverrideVisiblityIds.ContainsItem(VisibilityMeshIndex))
				{
					bVisible = TRUE;
					break;
				}
				// This means forced visibility will override forced invisibility
				// Something to keep in mind when an LD complains that an actor 
				// they put into the OverrideInvisibility list is still showing up!
				if (AffectingOverrideVolumes(VolumeIndex)->OverrideInvisiblityIds.ContainsItem(VisibilityMeshIndex))
				{
					bVisible = FALSE;
					bForceInvisible = TRUE;
					break;
				}
			}

			if (bVisible)
			{
				MappingContext.Stats.NumQueriesVisibleByDistanceRatio++;
			}

			const FVector4 PerpendicularVector = MeshToCellCenter ^ FVector4(0, 0, 1);
			SamplePositions.Reset();

			// Generate samples for explicit visibility sampling of the mesh
			for (INT CellSampleIndex = 0; CellSampleIndex < PrecomputedVisibilitySettings.NumCellSamples; CellSampleIndex++)
			{
				for (INT MeshSampleIndex = 0; MeshSampleIndex < NumMeshSamples; MeshSampleIndex++)
				{
					FVisibilityQuerySample NewSample;
					{
						FLOAT PDF;
						FLOAT Sample;
						// Generate a sample on the visible faces of the cell, picking a face with probability proportional to the projected angle of the cell face onto the mesh's origin
						//@todo - weight by face area, since cells have a different height from their x and y sizes
						Sample1dCDF(VisibleCellFacePDFs, VisibleCellFaceCDFs, UnnormalizedIntegral, RandomStream, PDF, Sample);
						const INT ChosenCellFaceIndex = appTrunc(Sample * VisibleCellFaces.Num());
						const FAxisAlignedCellFace& ChosenFace = CellFaces[VisibleCellFaces(ChosenCellFaceIndex)];
						NewSample.CellPosition = ChosenFace.FaceMin + ChosenFace.FaceExtent * FVector4(RandomStream.GetFraction(), RandomStream.GetFraction(), RandomStream.GetFraction());
					}
					{
						// Generate a sample on the visible faces of the mesh
						const INT ChosenFaceIndex = appTrunc(RandomStream.GetFraction() * VisibleMeshFaces.Num());
						const FAxisAlignedCellFace& ChosenFace = MeshBoxFaces[VisibleMeshFaces(ChosenFaceIndex)];
						NewSample.MeshPosition = ChosenFace.FaceMin + ChosenFace.FaceExtent * FVector4(RandomStream.GetFraction(), RandomStream.GetFraction(), RandomStream.GetFraction());
					}
					const FVector4 HalfPosition = .5f * (NewSample.CellPosition + NewSample.MeshPosition);
					NewSample.PerpendicularDistance = HalfPosition | PerpendicularVector;
					SamplePositions.AddItem(NewSample);
				}
			}

			// Sort the samples to make them more coherent in kDOP tree traversals, which provides a small speedup
			Sort<FVisibilityQuerySample,COMPARE_CONSTREF_CLASS(FVisibilityQuerySample,PrecomputedVisibility)>(SamplePositions.GetData(), SamplePositions.Num());

			const DOUBLE SampleGenerationEndTime = appSeconds();
			MappingContext.Stats.PrecomputedVisibilitySampleSetupThreadTime += SampleGenerationEndTime - SampleGenerationStartTime;

			FLOAT FurthestDistanceSquared = 0;
			// Early out if any sample finds the mesh visible, unless we are debugging this cell and want to see all the samples
			for (INT CellSampleIndex = 0; (CellSampleIndex < PrecomputedVisibilitySettings.NumCellSamples) && (!bForceInvisible) && (!bVisible || bDebugThisCell); CellSampleIndex++)
			{
				for (INT MeshSampleIndex = 0; MeshSampleIndex < NumMeshSamples; MeshSampleIndex++)
				{
					FVisibilityQuerySample& CurrentSample = SamplePositions(CellSampleIndex * NumMeshSamples + MeshSampleIndex);
					const FVector4 CellSamplePosition = CurrentSample.CellPosition;
					const FVector4 MeshSamplePosition = CurrentSample.MeshPosition;

					FLightRay Ray(
						CellSamplePosition,
						MeshSamplePosition,
						NULL,
						NULL,
						// Masked materials often have small holes which increase visibility errors
						// This also allows us to use boolean visibility traces which are much faster than first hit traces
						// Only intersect with static objects since they will not move in game
						LIGHTRAY_STATIC_AND_OPAQUEONLY
						);
					
					FLightRayIntersection Intersection;
					// Use boolean visibility traces
					AggregateMesh.IntersectLightRay(Ray, FALSE, FALSE, FALSE, MappingContext.RayCache, Intersection);

					MappingContext.Stats.NumPrecomputedVisibilityRayTraces++;

					//Note: using intersection position even though we used a boolean ray trace, so the position may not be the closest
					CurrentSample.IntersectionPosition = Intersection.IntersectionVertex.WorldPosition;
					
					const FLOAT DistanceSquared = (CellSamplePosition - Intersection.IntersectionVertex.WorldPosition).SizeSquared();
					FurthestDistanceSquared = Max(FurthestDistanceSquared, DistanceSquared);

					if (bDebugThisMesh)
					{
						// Draw all the rays from the debug cell to the debug mesh
						FDebugStaticLightingRay DebugRay(CellSamplePosition, MeshSamplePosition, Intersection.bIntersects, FALSE);
						if (Intersection.bIntersects)
						{
							DebugRay.End = Intersection.IntersectionVertex.WorldPosition;
						}
						DataLink->Element.DebugVisibilityRays.AddItem(DebugRay);
					}

					if (!Intersection.bIntersects)
					{
						MappingContext.Stats.NumQueriesVisibleExplicitSampling++;
						bVisible = TRUE;
						if (!bDebugThisCell)
						{
							// The mesh is visible from the current cell, move on to the next mesh
							break;
						}
					}
				}
			}

			const DOUBLE RayTraceEndTime = appSeconds();
			MappingContext.Stats.PrecomputedVisibilityRayTraceThreadTime += RayTraceEndTime - SampleGenerationEndTime;

			// If the meshes has not been determined to be visible by explicit sampling, 
			// Do importance sampling to try and find visible meshes through cracks that have a low probability of being detected by explicit sampling.
			if (!bForceInvisible && !bVisible)
			{
				FurthestSamples.Reset();
				
				// Create an array of all the longest rays toward the mesh
				const FLOAT DistanceThreshold = appSqrt(FurthestDistanceSquared) * 7.0f / 8.0f;
				const FLOAT DistanceThresholdSq = DistanceThreshold * DistanceThreshold;
				for (INT SampleIndex = 0; SampleIndex < SamplePositions.Num(); SampleIndex++)
				{
					const FVisibilityQuerySample& CurrentSample = SamplePositions(SampleIndex);
					const FLOAT DistanceSquared = (CurrentSample.CellPosition - CurrentSample.IntersectionPosition).SizeSquared();
					if (DistanceSquared > DistanceThresholdSq)
					{
						FurthestSamples.AddItem(&CurrentSample);
					}
				}

				// Trace importance sampled rays to try and find visible meshes through small cracks
				// This is only slightly effective, but doesn't cost much compared to explicit sampling due to the small number of rays
				for (INT ImportanceSampleIndex = 0; ImportanceSampleIndex < PrecomputedVisibilitySettings.NumImportanceSamples && !bVisible; ImportanceSampleIndex++)
				{
					// Pick one of the furthest samples with uniform probability
					const INT SampleIndex = appTrunc(RandomStream.GetFraction() * FurthestSamples.Num());
					const FVisibilityQuerySample& CurrentSample = *FurthestSamples(SampleIndex);
					const FLOAT VectorLength = (CurrentSample.CellPosition - CurrentSample.MeshPosition).Size();
					const FVector4 CurrentDirection = (CurrentSample.MeshPosition - CurrentSample.CellPosition).SafeNormal();

					FVector4 XAxis;
					FVector4 YAxis;
					GenerateCoordinateSystem(CurrentDirection, XAxis, YAxis);

					// Generate a new direction in a cone 2 degrees from the original direction, to find cracks nearby
					const FVector4 SampleDirection = UniformSampleCone(
						RandomStream, 
						appCos(2.0f * PI / 180.0f), 
						XAxis, 
						YAxis, 
						CurrentDirection);

					const FVector4 EndPoint = CurrentSample.CellPosition + SampleDirection * VectorLength;

					FLightRay Ray(
						CurrentSample.CellPosition,
						EndPoint,
						NULL,
						NULL,
						LIGHTRAY_STATIC_AND_OPAQUEONLY
						);

					FLightRayIntersection Intersection;
					AggregateMesh.IntersectLightRay(Ray, FALSE, FALSE, FALSE, MappingContext.RayCache, Intersection);

					MappingContext.Stats.NumPrecomputedVisibilityRayTraces++;

					if (bDebugThisMesh)
					{
						// Draw all the rays from the debug cell to the debug mesh
						FDebugStaticLightingRay DebugRay(CurrentSample.CellPosition, EndPoint, Intersection.bIntersects, TRUE);
						if (Intersection.bIntersects)
						{
							DebugRay.End = Intersection.IntersectionVertex.WorldPosition;
						}
						DataLink->Element.DebugVisibilityRays.AddItem(DebugRay);
					}

					if (!Intersection.bIntersects)
					{
						MappingContext.Stats.NumQueriesVisibleImportanceSampling++;
						bVisible = TRUE;
						if (!bDebugThisCell)
						{
							// The mesh is visible from the current cell, move on to the next mesh
							break;
						}
					}
				}
				MappingContext.Stats.PrecomputedVisibilityImportanceSampleThreadTime += appSeconds() - RayTraceEndTime;
			}

			if (bVisible)
			{
				SetMeshVisible(CurrentCell.VisibilityData, VisibilityMeshIndex);
			}
		}

		if (bDebugThisCell)
		{
			// Draw the bounds of each cell processed
			const FVector4 Min = CurrentCell.Bounds.Min;
			const FVector4 Max = CurrentCell.Bounds.Max;
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Min.Z), FVector4(Min.X, Max.Y, Min.Z), FALSE));
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Max.Y, Min.Z), FVector4(Min.X, Max.Y, Max.Z), FALSE));
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Min.Z), FVector4(Min.X, Min.Y, Max.Z), FALSE));
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Max.Z), FVector4(Min.X, Max.Y, Max.Z), FALSE));

			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Min.Y, Min.Z), FVector4(Max.X, Max.Y, Min.Z), FALSE));
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Max.Y, Min.Z), FVector4(Max.X, Max.Y, Max.Z), FALSE));
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Min.Y, Min.Z), FVector4(Max.X, Min.Y, Max.Z), FALSE));
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Max.X, Min.Y, Max.Z), FVector4(Max.X, Max.Y, Max.Z), FALSE));

			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Min.Z), FVector4(Max.X, Min.Y, Min.Z), FALSE));
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Min.Y, Max.Z), FVector4(Max.X, Min.Y, Max.Z), FALSE));
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Max.Y, Min.Z), FVector4(Max.X, Max.Y, Min.Z), FALSE));
			DataLink->Element.DebugVisibilityRays.AddItem(FDebugStaticLightingRay(FVector4(Min.X, Max.Y, Max.Z), FVector4(Max.X, Max.Y, Max.Z), FALSE));
		}
	}
	
	MappingContext.Stats.PrecomputedVisibilityThreadTime = appSeconds() - StartTime;
	MappingContext.Stats.NumPrecomputedVisibilityCellsProcessed = MaxCellIndex - StartCellIndex;
	CompleteVisibilityTaskList.AddElement(DataLink);
}

}