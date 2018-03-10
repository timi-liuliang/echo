/*=============================================================================
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "LightingSystem.h"
#include "MonteCarlo.h"

namespace Lightmass
{

/** Prepares for multithreaded generation of VolumeDistanceField. */
void FStaticLightingSystem::BeginCalculateVolumeDistanceField()
{
	DistanceFieldVolumeBounds = Scene.ImportanceBoundingBox;
	if (DistanceFieldVolumeBounds.GetVolume() < KINDA_SMALL_NUMBER)
	{
		DistanceFieldVolumeBounds = AggregateMesh.GetBounds();
	}

	FBox UnclampedDistanceFieldVolumeBounds = DistanceFieldVolumeBounds;
	FVector4 DoubleExtent = UnclampedDistanceFieldVolumeBounds.GetExtent() * 2;
	DoubleExtent.X = DoubleExtent.X - appFmod(DoubleExtent.X, VolumeDistanceFieldSettings.VoxelSize) + VolumeDistanceFieldSettings.VoxelSize;
	DoubleExtent.Y = DoubleExtent.Y - appFmod(DoubleExtent.Y, VolumeDistanceFieldSettings.VoxelSize) + VolumeDistanceFieldSettings.VoxelSize;
	DoubleExtent.Z = DoubleExtent.Z - appFmod(DoubleExtent.Z, VolumeDistanceFieldSettings.VoxelSize) + VolumeDistanceFieldSettings.VoxelSize;
	// Round the max up to the next step boundary
	UnclampedDistanceFieldVolumeBounds.Max = UnclampedDistanceFieldVolumeBounds.Min + DoubleExtent;

	const FVector4 VolumeSizes = UnclampedDistanceFieldVolumeBounds.GetExtent() * 2.0f / VolumeDistanceFieldSettings.VoxelSize;
	VolumeSizeX = appTrunc(VolumeSizes.X + DELTA);
	VolumeSizeY = appTrunc(VolumeSizes.Y + DELTA);
	VolumeSizeZ = appTrunc(VolumeSizes.Z + DELTA);

	// Use a float to avoid 32 bit integer overflow with large volumes
	const FLOAT NumVoxels = VolumeSizeX * VolumeSizeY * VolumeSizeZ;

	if (NumVoxels > VolumeDistanceFieldSettings.MaxVoxels)
	{
		const INT OldSizeX = VolumeSizeX;
		const INT OldSizeY = VolumeSizeY;
		const INT OldSizeZ = VolumeSizeZ;
		const FLOAT SingleDimensionScale = appPow(NumVoxels / VolumeDistanceFieldSettings.MaxVoxels, 1.0f / 3.0f);
		DistanceFieldVoxelSize = VolumeDistanceFieldSettings.VoxelSize * SingleDimensionScale;

		DoubleExtent = DistanceFieldVolumeBounds.GetExtent() * 2;
		DoubleExtent.X = DoubleExtent.X - appFmod(DoubleExtent.X, DistanceFieldVoxelSize) + DistanceFieldVoxelSize;
		DoubleExtent.Y = DoubleExtent.Y - appFmod(DoubleExtent.Y, DistanceFieldVoxelSize) + DistanceFieldVoxelSize;
		DoubleExtent.Z = DoubleExtent.Z - appFmod(DoubleExtent.Z, DistanceFieldVoxelSize) + DistanceFieldVoxelSize;
		// Round the max up to the next step boundary with the clamped voxel size
		DistanceFieldVolumeBounds.Max = DistanceFieldVolumeBounds.Min + DoubleExtent;

		const FVector4 ClampedVolumeSizes = DistanceFieldVolumeBounds.GetExtent() * 2.0f / DistanceFieldVoxelSize;
		VolumeSizeX = appTrunc(ClampedVolumeSizes.X + DELTA);
		VolumeSizeY = appTrunc(ClampedVolumeSizes.Y + DELTA);
		VolumeSizeZ = appTrunc(ClampedVolumeSizes.Z + DELTA);
		
		LogSolverMessage(FString::Printf(TEXT("CalculateVolumeDistanceField %ux%ux%u, clamped to %ux%ux%u"), OldSizeX, OldSizeY, OldSizeZ, VolumeSizeX, VolumeSizeY, VolumeSizeZ));
	}
	else 
	{
		DistanceFieldVolumeBounds = UnclampedDistanceFieldVolumeBounds;
		DistanceFieldVoxelSize = VolumeDistanceFieldSettings.VoxelSize;
		LogSolverMessage(FString::Printf(TEXT("CalculateVolumeDistanceField %ux%ux%u"), VolumeSizeX, VolumeSizeY, VolumeSizeZ));
	}

	VolumeDistanceField.Empty(VolumeSizeX * VolumeSizeY * VolumeSizeZ);
	VolumeDistanceField.AddZeroed(VolumeSizeX * VolumeSizeY * VolumeSizeZ);

	appInterlockedExchange(&NumOutstandingVolumeDataLayers, VolumeSizeZ);
}

/** Generates a single z layer of VolumeDistanceField. */
void FStaticLightingSystem::CalculateVolumeDistanceFieldWorkRange(INT ZIndex)
{
	const DOUBLE StartTime = appSeconds();
	FStaticLightingMappingContext MappingContext(NULL, *this);

	TArray<FVector4> SampleDirections;
	const INT NumThetaSteps = appTrunc(appSqrt(VolumeDistanceFieldSettings.NumVoxelDistanceSamples / (2.0f * (FLOAT)PI)));
	const INT NumPhiSteps = appTrunc(NumThetaSteps * (FLOAT)PI);
	FRandomStream RandomStream(0);
	GenerateStratifiedUniformHemisphereSamples(NumThetaSteps, NumPhiSteps, RandomStream, SampleDirections);
	TArray<FVector4> OtherHemisphereSamples;
	GenerateStratifiedUniformHemisphereSamples(NumThetaSteps, NumPhiSteps, RandomStream, OtherHemisphereSamples);

	for (INT i = 0; i < OtherHemisphereSamples.Num(); i++)
	{
		FVector4 Sample = OtherHemisphereSamples(i);
		Sample.Z *= -1;
		SampleDirections.AddItem(Sample);
	}

	const FVector4 CellExtents = FVector4(DistanceFieldVoxelSize / 2, DistanceFieldVoxelSize / 2, DistanceFieldVoxelSize / 2);
	for (INT YIndex = 0; YIndex < VolumeSizeY; YIndex++)
	{
		for (INT XIndex = 0; XIndex < VolumeSizeX; XIndex++)
		{
			const FVector4 VoxelPosition = FVector4(XIndex, YIndex, ZIndex) * DistanceFieldVoxelSize + DistanceFieldVolumeBounds.Min + CellExtents;
			const INT Index = ZIndex * VolumeSizeY * VolumeSizeX + YIndex * VolumeSizeX + XIndex;
	
			FLOAT MinDistance[2];
			MinDistance[0] = FLT_MAX;
			MinDistance[1] = FLT_MAX;

			INT Hit[2];
			Hit[0] = 0;
			Hit[1] = 0;

			INT HitFront[2];
			HitFront[0] = 0;
			HitFront[1] = 0;

			// Generate two distance fields
			// The first is for mostly horizontal triangles, the second is for mostly vertical triangles
			// Keeping them separate allows reconstructing a cleaner surface,
			// Otherwise there would be holes in the surface where an unclosed wall mesh intersects an unclosed ground mesh
			for (INT i = 0; i < 2; i++)
			{
				for (INT SampleIndex = 0; SampleIndex < SampleDirections.Num(); SampleIndex++)
				{
					const FLOAT ExtentDistance = DistanceFieldVolumeBounds.GetExtent().GetMax() * 2.0f;
					FLightRay Ray(
						VoxelPosition,
						VoxelPosition + SampleDirections(SampleIndex) * VolumeDistanceFieldSettings.VolumeMaxDistance,
						NULL,
						NULL
						);

					// Trace rays in all directions to find the closest solid surface
					FLightRayIntersection Intersection;
					AggregateMesh.IntersectLightRay(Ray, TRUE, FALSE, FALSE, MappingContext.RayCache, Intersection);

					if (Intersection.bIntersects)
					{
						if ((i == 0 && Abs(Intersection.IntersectionVertex.WorldTangentZ.Z) >= .707f || i == 1 && Abs(Intersection.IntersectionVertex.WorldTangentZ.Z) < .707f))
						{
							Hit[i]++;
							if ((Ray.Direction | Intersection.IntersectionVertex.WorldTangentZ) < 0)
							{
								HitFront[i]++;
							}

							const FLOAT CurrentDistance = (VoxelPosition - Intersection.IntersectionVertex.WorldPosition).Size();
							if (CurrentDistance < MinDistance[i])
							{
								MinDistance[i] = CurrentDistance;
							}
						}
					}
				}
				// Consider this voxel 'outside' an object if more than 75% of the rays hit front faces
				MinDistance[i] *= (Hit[i] == 0 || HitFront[i] > Hit[i] * .75f) ? 1 : -1;
			}
			
			// Create a mask storing where an intersection can possibly take place
			// This allows the reconstruction to ignore areas where large positive and negative distances come together,
			// Which is caused by unclosed surfaces.
			const BYTE Mask0 = Abs(MinDistance[0]) < DistanceFieldVoxelSize * 2 ? 255 : 0; 
			// 0 will be -MaxDistance, .5 will be 0, 1 will be +MaxDistance
			const FLOAT NormalizedDistance0 = Clamp(MinDistance[0] / VolumeDistanceFieldSettings.VolumeMaxDistance + .5f, 0.0f, 1.0f);

			const BYTE Mask1 = Abs(MinDistance[1]) < DistanceFieldVoxelSize * 2 ? 255 : 0; 
			const FLOAT NormalizedDistance1 = Clamp(MinDistance[1] / VolumeDistanceFieldSettings.VolumeMaxDistance + .5f, 0.0f, 1.0f);

			const FColor FinalValue(
				Clamp<BYTE>(appTrunc(NormalizedDistance0 * 255), 0, 255), 
				Clamp<BYTE>(appTrunc(NormalizedDistance1 * 255), 0, 255), 
				Mask0,
				Mask1
				);

			VolumeDistanceField(Index) = FinalValue;
		}
	}
	MappingContext.Stats.VolumeDistanceFieldThreadTime = appSeconds() - StartTime;
}

}