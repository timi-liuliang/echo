/*=============================================================================
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "LightingSystem.h"
#include "../Inc/Raster.h"
#include "MonteCarlo.h"

namespace Lightmass
{

typedef FVolumeSampleInterpolationElement FVolumeSampleProximityElement;

typedef TOctree<FVolumeSampleProximityElement,struct FVolumeLightingProximityOctreeSemantics> FVolumeLightingProximityOctree;

struct FVolumeLightingProximityOctreeSemantics
{
	//@todo - evaluate different performance/memory tradeoffs with these
	enum { MaxElementsPerLeaf = 4 };
	enum { MaxNodeDepth = 12 };
	enum { LoosenessDenominator = 16 };

	typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

	static FBoxCenterAndExtent GetBoundingBox(const FVolumeSampleProximityElement& Element)
	{
		const FVolumeLightingSample& Sample = Element.VolumeSamples(Element.SampleIndex);
		return FBoxCenterAndExtent(FVector4(Sample.PositionAndRadius, 0.0f), FVector4(0,0,0));
	}
};

/** Constructs an SH environment from this lighting sample. */
void FVolumeLightingSample::ToSHVector(FSHVectorRGB& SHVector) const
{
	// Accumulate stored lighting terms
	// Assuming SHVector has already been initialized to 0
	SHVector.AddIncomingRadiance(IndirectRadiance, 1.0f, FVector4(0,0,0), IndirectDirection);
	SHVector.AddIncomingRadiance(EnvironmentRadiance, 1.0f, FVector4(0,0,0), EnvironmentDirection);
	SHVector.AddAmbient(AmbientRadiance);
}

/** Returns TRUE if there is an existing sample in VolumeOctree within SearchDistance of Position. */
static UBOOL FindNearbyVolumeSample(const FVolumeLightingProximityOctree& VolumeOctree, const FVector4& Position, FLOAT SearchDistance)
{
	const FBox SearchBox = FBox::BuildAABB(Position, FVector4(SearchDistance, SearchDistance, SearchDistance));
	for (FVolumeLightingProximityOctree::TConstIterator<> OctreeIt(VolumeOctree); OctreeIt.HasPendingNodes(); OctreeIt.Advance())
	{
		const FVolumeLightingProximityOctree::FNode& CurrentNode = OctreeIt.GetCurrentNode();
		const FOctreeNodeContext& CurrentContext = OctreeIt.GetCurrentContext();
		{
			// Push children onto the iterator stack if they intersect the query box
			if (!CurrentNode.IsLeaf())
			{
				FOREACH_OCTREE_CHILD_NODE(ChildRef)
				{
					if (CurrentNode.HasChild(ChildRef))
					{
						const FOctreeNodeContext ChildContext = CurrentContext.GetChildContext(ChildRef);
						if (ChildContext.Bounds.GetBox().Intersect(SearchBox))
						{
							OctreeIt.PushChild(ChildRef);
						}
					}
				}
			}
		}

		// Iterate over all samples in the nodes intersecting the query box
		for (FVolumeLightingProximityOctree::ElementConstIt It(CurrentNode.GetConstElementIt()); It; ++It)
		{
			const FVolumeSampleProximityElement& Element = *It;
			const FLOAT DistanceSquared = (Element.VolumeSamples(Element.SampleIndex).GetPosition() - Position).SizeSquared();
			if (DistanceSquared < SearchDistance * SearchDistance)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

class FVolumeSamplePlacementRasterPolicy
{
public:

	typedef FStaticLightingVertex InterpolantType;

	/** Initialization constructor. */
	FVolumeSamplePlacementRasterPolicy(
		INT InSizeX, 
		INT InSizeY, 
		FLOAT InMinSampleDistance, 
		FStaticLightingSystem& InSystem,
		FCoherentRayCache& InCoherentRayCache,
		FVolumeLightingProximityOctree& InProximityOctree)
		:
		SizeX(InSizeX),
		SizeY(InSizeY),
		MinSampleDistance(InMinSampleDistance),
		System(InSystem),
		CoherentRayCache(InCoherentRayCache),
		ProximityOctree(InProximityOctree)
	{
		LayerHeightOffsets.Empty(System.DynamicObjectSettings.NumSurfaceSampleLayers);
		LayerHeightOffsets.AddItem(System.DynamicObjectSettings.FirstSurfaceSampleLayerHeight);
		for (INT i = 0; i < System.DynamicObjectSettings.NumSurfaceSampleLayers; i++)
		{
			LayerHeightOffsets.AddItem(System.DynamicObjectSettings.FirstSurfaceSampleLayerHeight + i * System.DynamicObjectSettings.SurfaceSampleLayerHeightSpacing);
		}
	}

	void SetLevelId(INT InLevelId)
	{
		LevelId = InLevelId;
	}

protected:

	// FTriangleRasterizer policy interface.

	INT GetMinX() const { return 0; }
	INT GetMaxX() const { return SizeX; }
	INT GetMinY() const { return 0; }
	INT GetMaxY() const { return SizeY; }

	void ProcessPixel(INT X,INT Y,const InterpolantType& Interpolant,UBOOL BackFacing);

private:

	const INT SizeX;
	const INT SizeY;
	const FLOAT MinSampleDistance;
	INT LevelId;
	FStaticLightingSystem& System;
	FCoherentRayCache& CoherentRayCache;
	FVolumeLightingProximityOctree& ProximityOctree;
	TArray<FLOAT> LayerHeightOffsets;
};

void FVolumeSamplePlacementRasterPolicy::ProcessPixel(INT X,INT Y,const InterpolantType& Vertex,UBOOL BackFacing)
{
	// Only place samples inside the scene's bounds
	if (System.IsPointInImportanceVolume(Vertex.WorldPosition))
	{
		// Place a sample for each layer
		for (INT SampleIndex = 0; SampleIndex < LayerHeightOffsets.Num(); SampleIndex++)
		{
			const FVector4 SamplePosition = Vertex.WorldPosition + FVector4(0, 0, LayerHeightOffsets(SampleIndex));
			// Only place a sample if there isn't already one nearby, to avoid clumping
			if (!FindNearbyVolumeSample(ProximityOctree, SamplePosition, MinSampleDistance))
			{
				TArray<FVolumeLightingSample>* VolumeLightingSamples = System.VolumeLightingSamples.Find(LevelId);
				check(VolumeLightingSamples);
				// Add a new sample for this layer
				// Expand the radius to touch a diagonal sample on the grid for a little overlap
				VolumeLightingSamples->AddItem(FVolumeLightingSample(FVector4(SamplePosition, System.DynamicObjectSettings.SurfaceLightSampleSpacing * appSqrt(2.0f))));
				// Add the sample to the proximity octree so we can avoid placing any more samples nearby
				ProximityOctree.AddElement(FVolumeSampleProximityElement(VolumeLightingSamples->Num() - 1, *VolumeLightingSamples));
				if (System.DynamicObjectSettings.bVisualizeVolumeLightInterpolation)
				{
					System.VolumeLightingInterpolationOctree.AddElement(FVolumeSampleInterpolationElement(VolumeLightingSamples->Num() - 1, *VolumeLightingSamples));
				}
			}
		}
	}
}

/** Places volume lighting samples and calculates lighting for them. */
void FStaticLightingSystem::CalculateVolumeSamples()
{
	const DOUBLE SampleVolumeStart = appSeconds();
	if (VolumeLightingSamples.Num() == 0)
	{
		const DOUBLE VolumeSampleStartTime = appSeconds();
		VolumeBounds = GetImportanceBounds(FALSE);
		if (VolumeBounds.SphereRadius < DELTA)
		{
			VolumeBounds = FBoxSphereBounds(AggregateMesh.GetBounds());
		}

		// Only place samples if the volume has area
		if (VolumeBounds.BoxExtent.X > 0.0f && VolumeBounds.BoxExtent.Y > 0.0f && VolumeBounds.BoxExtent.Z > 0.0f)
		{
			FLOAT LandscapeEstimateNum = 0.f;
			// Estimate Light sample number near Landscape surfaces
			if (DynamicObjectSettings.bUseMaxSurfaceSampleNum && DynamicObjectSettings.MaxSurfaceLightSamples > 100)
			{
				FLOAT SquaredSpacing = Square(DynamicObjectSettings.SurfaceLightSampleSpacing);
				if (SquaredSpacing == 0.f) SquaredSpacing = 1.0f;
				for (INT MappingIndex = 0; MappingIndex < LandscapeMappings.Num(); MappingIndex++)
				{
					FStaticLightingVertex Vertices[3];
					INT ElementIndex;
					const FStaticLightingMapping* CurrentMapping = LandscapeMappings(MappingIndex);
					const FStaticLightingMesh* CurrentMesh = CurrentMapping->Mesh;
					CurrentMesh->GetTriangle((CurrentMesh->NumTriangles)>>1, Vertices[0], Vertices[1], Vertices[2], ElementIndex);
					// Only place inside the importance volume
					if (IsPointInImportanceVolume(Vertices[0].WorldPosition))
					{
						FVector4 TriangleNormal = (Vertices[2].WorldPosition - Vertices[0].WorldPosition) ^ (Vertices[1].WorldPosition - Vertices[0].WorldPosition);
						TriangleNormal.Z = 0.f; // approximate only for X-Y plane
						FLOAT TotalArea = 0.5f * TriangleNormal.Size() * CurrentMesh->NumTriangles;
						LandscapeEstimateNum += TotalArea / Square(DynamicObjectSettings.SurfaceLightSampleSpacing);
					}
				}
				LandscapeEstimateNum *= DynamicObjectSettings.NumSurfaceSampleLayers;

				if (LandscapeEstimateNum > DynamicObjectSettings.MaxSurfaceLightSamples)
				{
					// Increase DynamicObjectSettings.SurfaceLightSampleSpacing to reduce light sample number
					FLOAT OldMaxSurfaceLightSamples = DynamicObjectSettings.SurfaceLightSampleSpacing;
					DynamicObjectSettings.SurfaceLightSampleSpacing = DynamicObjectSettings.SurfaceLightSampleSpacing * appSqrt((FLOAT)LandscapeEstimateNum / DynamicObjectSettings.MaxSurfaceLightSamples);
					debugf(TEXT("Too many LightSamples : DynamicObjectSettings.SurfaceLightSampleSpacing is increased from %g to %g"), OldMaxSurfaceLightSamples, DynamicObjectSettings.SurfaceLightSampleSpacing);
					LandscapeEstimateNum = DynamicObjectSettings.MaxSurfaceLightSamples;
				}
			}

			//@todo - can this be presized more accurately?
			VolumeLightingSamples.Empty(Max<INT>(5000, LandscapeEstimateNum));
			FStaticLightingMappingContext MappingContext(NULL, *this);
			// Octree used to keep track of where existing samples have been placed
			FVolumeLightingProximityOctree VolumeLightingOctree(VolumeBounds.Origin, VolumeBounds.BoxExtent.GetMax());
			// Octree used for interpolating lighting for debugging
			VolumeLightingInterpolationOctree = FVolumeLightingInterpolationOctree(VolumeBounds.Origin, VolumeBounds.BoxExtent.GetMax());
			// Determine the resolution that the scene should be rasterized at based on SurfaceLightSampleSpacing and the scene's extent
			const INT RasterSizeX = appTrunc(2.0f * VolumeBounds.BoxExtent.X / DynamicObjectSettings.SurfaceLightSampleSpacing);
			const INT RasterSizeY = appTrunc(2.0f * VolumeBounds.BoxExtent.Y / DynamicObjectSettings.SurfaceLightSampleSpacing);

			FTriangleRasterizer<FVolumeSamplePlacementRasterPolicy> Rasterizer(
				FVolumeSamplePlacementRasterPolicy(
				RasterSizeX, 
				RasterSizeY, 
				// Use a minimum sample distance slightly less than the SurfaceLightSampleSpacing
				0.9f * DynamicObjectSettings.SurfaceLightSampleSpacing, 
				*this,
				MappingContext.RayCache,
				VolumeLightingOctree));

			check(Meshes.Num() == AllMappings.Num());
			// Rasterize all meshes in the scene and place high detail samples on their surfaces.
			// Iterate through mappings and retreive the mesh from that, so we can make decisions based on whether the mesh is using texture or vertex lightmaps.
			for (INT MappingIndex = 0; MappingIndex < AllMappings.Num(); MappingIndex++)
			{
				const FStaticLightingMapping* CurrentMapping = AllMappings(MappingIndex);
				const FStaticLightingVertexMapping* VertexMapping = CurrentMapping->GetVertexMapping();
				const FStaticLightingTextureMapping* TextureMapping = CurrentMapping->GetTextureMapping();
				const FStaticLightingMesh* CurrentMesh = CurrentMapping->Mesh;
				// Only place samples on shadow casting meshes.
				if (CurrentMesh->LightingFlags & GI_INSTANCE_CASTSHADOW)
				{
					// Create a new LevelId array if necessary
					if (!VolumeLightingSamples.Find(CurrentMesh->LevelId))
					{
						VolumeLightingSamples.Set(CurrentMesh->LevelId, TArray<FVolumeLightingSample>());
					}
					// Tell the rasterizer we are adding samples to this mesh's LevelId
					Rasterizer.SetLevelId(CurrentMesh->LevelId);
					// Rasterize all triangles in the mesh
					for (INT TriangleIndex = 0; TriangleIndex < CurrentMesh->NumTriangles; TriangleIndex++)
					{
						FStaticLightingVertex Vertices[3];
						INT ElementIndex;
						CurrentMesh->GetTriangle(TriangleIndex, Vertices[0], Vertices[1], Vertices[2], ElementIndex);

						if (CurrentMesh->IsElementCastingShadow(ElementIndex))
						{
							FVector2D XYPositions[3];
							for (INT VertIndex = 0; VertIndex < 3; VertIndex++)
							{
								// Transform world space positions from [VolumeBounds.Origin - VolumeBounds.BoxExtent, VolumeBounds.Origin + VolumeBounds.BoxExtent] into [0,1]
								const FVector4 TransformedPosition = (Vertices[VertIndex].WorldPosition - VolumeBounds.Origin + VolumeBounds.BoxExtent) / (2.0f * VolumeBounds.BoxExtent);
								// Project positions onto the XY plane and scale to the resolution determined by DynamicObjectSettings.SurfaceLightSampleSpacing
								XYPositions[VertIndex] = FVector2D(TransformedPosition.X * RasterSizeX, TransformedPosition.Y * RasterSizeY);
							}

							const FVector4 TriangleNormal = (Vertices[2].WorldPosition - Vertices[0].WorldPosition) ^ (Vertices[1].WorldPosition - Vertices[0].WorldPosition);
							const FLOAT TriangleArea = 0.5f * TriangleNormal.Size();

							if (TriangleArea > DELTA)
							{
								// Skip vertex lightmapped triangles whose area is greater than a right triangle formed by SurfaceLightSampleSpacing.
								// If surface lighting is being calculated at a low resolution, it's unlikely that the volume near that surface needs to have detailed lighting.
								if (VertexMapping && 0.5f * Square(DynamicObjectSettings.SurfaceLightSampleSpacing) < TriangleArea)
								{
									continue;
								}

								if (TextureMapping)
								{
									// Triangle vertices in lightmap UV space, scaled by the lightmap resolution
									const FVector2D Vertex0 = Vertices[0].TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->SizeX, TextureMapping->SizeY);
									const FVector2D Vertex1 = Vertices[1].TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->SizeX, TextureMapping->SizeY);
									const FVector2D Vertex2 = Vertices[2].TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->SizeX, TextureMapping->SizeY);

									// Area in lightmap space, or the number of lightmap texels covered by this triangle
									const FLOAT LightmapTriangleArea = Abs(
										Vertex0.X * (Vertex1.Y - Vertex2.Y)
										+ Vertex1.X * (Vertex2.Y - Vertex0.Y)
										+ Vertex2.X * (Vertex0.Y - Vertex1.Y));

									const FLOAT TexelDensity = LightmapTriangleArea / TriangleArea;
									// Skip texture lightmapped triangles whose texel density is less than one texel per the area of a right triangle formed by SurfaceLightSampleSpacing.
									// If surface lighting is being calculated at a low resolution, it's unlikely that the volume near that surface needs to have detailed lighting.
									if (TexelDensity < 2.0f / Square(DynamicObjectSettings.SurfaceLightSampleSpacing))
									{
										continue;
									}
								}

								// Only rasterize upward facing triangles
								if (TriangleNormal.Z > 0.0f)
								{
									Rasterizer.DrawTriangle(
										Vertices[0],
										Vertices[1],
										Vertices[2],
										XYPositions[0],
										XYPositions[1],
										XYPositions[2],
										FALSE
										);
								}
							}
						}
					}
				}
			}

			const FLOAT DetailVolumeSpacing = DynamicObjectSettings.DetailVolumeSampleSpacing;
			// Generate samples in a uniform 3d grid inside the detail volumes.  These will handle detail indirect lighting in areas that aren't directly above a surface.
			for (INT VolumeIndex = 0; VolumeIndex < Scene.CharacterIndirectDetailVolumes.Num(); VolumeIndex++)
			{
				const FBox& DetailVolumeBounds = Scene.CharacterIndirectDetailVolumes(VolumeIndex);
				for (FLOAT SampleX = DetailVolumeBounds.Min.X; SampleX < DetailVolumeBounds.Max.X; SampleX += DetailVolumeSpacing)
				{
					for (FLOAT SampleY = DetailVolumeBounds.Min.Y; SampleY < DetailVolumeBounds.Max.Y; SampleY += DetailVolumeSpacing)
					{
						for (FLOAT SampleZ = DetailVolumeBounds.Min.Z; SampleZ < DetailVolumeBounds.Max.Z; SampleZ += DetailVolumeSpacing)
						{
							const FVector4 SamplePosition(SampleX, SampleY, SampleZ);
							
							// Only place a sample if there are no surface lighting samples nearby
							if (!FindNearbyVolumeSample(VolumeLightingOctree, SamplePosition, DynamicObjectSettings.SurfaceLightSampleSpacing))
							{
								const FLightRay Ray(
									SamplePosition,
									SamplePosition - FVector4(0,0,VolumeBounds.BoxExtent.Z * 2.0f),
									NULL,
									NULL
									);
								FLightRayIntersection Intersection;
								// Trace a ray straight down to find which level's geometry we are over, 
								// Since this is how Dynamic Light Environments figure out which level to interpolate indirect lighting from.
								//@todo - could probably reuse the ray trace results for all samples of the same X and Y
								AggregateMesh.IntersectLightRay(Ray, TRUE, FALSE, FALSE, MappingContext.RayCache, Intersection);

								// Place the sample in the intersected level, or the persistent level if there was no intersection
								const INT LevelId = Intersection.bIntersects ? Intersection.Mesh->LevelId : INDEX_NONE;
								TArray<FVolumeLightingSample>* VolumeLightingSampleArray = VolumeLightingSamples.Find(LevelId);
								if (!VolumeLightingSampleArray)
								{
									VolumeLightingSampleArray = &VolumeLightingSamples.Set(LevelId, TArray<FVolumeLightingSample>());
								}

								// Add a sample and set its radius such that its influence touches a diagonal sample on the 3d grid.
								VolumeLightingSampleArray->AddItem(FVolumeLightingSample(FVector4(SamplePosition, DetailVolumeSpacing * appSqrt(3.0f))));
								VolumeLightingOctree.AddElement(FVolumeSampleProximityElement(VolumeLightingSampleArray->Num() - 1, *VolumeLightingSampleArray));
								if (DynamicObjectSettings.bVisualizeVolumeLightInterpolation)
								{
									VolumeLightingInterpolationOctree.AddElement(FVolumeSampleInterpolationElement(VolumeLightingSampleArray->Num() - 1, *VolumeLightingSampleArray));
								}
							}
						}
					}
				}
			}

			INT SurfaceSamples = 0;
			for (TMap<INT,TArray<FVolumeLightingSample> >::TIterator It(VolumeLightingSamples); It; ++It)
			{
				SurfaceSamples += It.Value().Num();
			}
			Stats.NumDynamicObjectSurfaceSamples = SurfaceSamples;

			TArray<FVolumeLightingSample>* UniformVolumeSamples = VolumeLightingSamples.Find(INDEX_NONE);
			if (!UniformVolumeSamples)
			{
				UniformVolumeSamples = &VolumeLightingSamples.Set(INDEX_NONE, TArray<FVolumeLightingSample>());
			}

			const FLOAT VolumeSpacingCubed = DynamicObjectSettings.VolumeLightSampleSpacing * DynamicObjectSettings.VolumeLightSampleSpacing * DynamicObjectSettings.VolumeLightSampleSpacing;
			INT RequestedVolumeSamples = appTrunc(8.0f * VolumeBounds.BoxExtent.X * VolumeBounds.BoxExtent.Y * VolumeBounds.BoxExtent.Z / VolumeSpacingCubed);
			RequestedVolumeSamples = RequestedVolumeSamples == appTruncErrorCode ? INT_MAX : RequestedVolumeSamples;
			FLOAT EffectiveVolumeSpacing = DynamicObjectSettings.VolumeLightSampleSpacing;

			// Clamp the number of volume samples generated to DynamicObjectSettings.MaxVolumeSamples if necessary by resizing EffectiveVolumeSpacing
			if (RequestedVolumeSamples > DynamicObjectSettings.MaxVolumeSamples)
			{
				EffectiveVolumeSpacing = appPow(8.0f * VolumeBounds.BoxExtent.X * VolumeBounds.BoxExtent.Y * VolumeBounds.BoxExtent.Z / DynamicObjectSettings.MaxVolumeSamples, .3333333f);
			}
			
			INT NumUniformVolumeSamples = 0;
			// Generate samples in a uniform 3d grid inside the importance volume.  These will be used for low resolution lighting in unimportant areas.
			for (FLOAT SampleX = VolumeBounds.Origin.X - VolumeBounds.BoxExtent.X; SampleX < VolumeBounds.Origin.X + VolumeBounds.BoxExtent.X; SampleX += EffectiveVolumeSpacing)
			{
				for (FLOAT SampleY = VolumeBounds.Origin.Y - VolumeBounds.BoxExtent.Y; SampleY < VolumeBounds.Origin.Y + VolumeBounds.BoxExtent.Y; SampleY += EffectiveVolumeSpacing)
				{
					for (FLOAT SampleZ = VolumeBounds.Origin.Z - VolumeBounds.BoxExtent.Z; SampleZ < VolumeBounds.Origin.Z + VolumeBounds.BoxExtent.Z; SampleZ += EffectiveVolumeSpacing)
					{
						const FVector4 SamplePosition(SampleX, SampleY, SampleZ);
						// Only place inside the importance volume
						if (IsPointInImportanceVolume(SamplePosition)
							// Only place a sample if there are no surface lighting samples nearby
							&& !FindNearbyVolumeSample(VolumeLightingOctree, SamplePosition, DynamicObjectSettings.SurfaceLightSampleSpacing))
						{
							NumUniformVolumeSamples++;
							// Add a sample and set its radius such that its influence touches a diagonal sample on the 3d grid.
							UniformVolumeSamples->AddItem(FVolumeLightingSample(FVector4(SamplePosition, EffectiveVolumeSpacing * appSqrt(3.0f))));
							VolumeLightingOctree.AddElement(FVolumeSampleProximityElement(UniformVolumeSamples->Num() - 1, *UniformVolumeSamples));
							if (DynamicObjectSettings.bVisualizeVolumeLightInterpolation)
							{
								VolumeLightingInterpolationOctree.AddElement(FVolumeSampleInterpolationElement(UniformVolumeSamples->Num() - 1, *UniformVolumeSamples));
							}
						}
					}
				}
			}
			Stats.NumDynamicObjectVolumeSamples = NumUniformVolumeSamples;

			FRandomStream RandomStream(0);

			TArray<FVector4> UniformHemisphereSamples;
			// Volume samples don't do any importance sampling so they need more samples for the same amount of variance as surface samples
			const FLOAT NumThetaStepsFloat = appSqrt(ImportanceTracingSettings.NumHemisphereSamples / (FLOAT)PI);
			const INT NumThetaSteps = appTrunc(NumThetaStepsFloat);
			const INT NumPhiSteps = appTrunc(NumThetaStepsFloat * (FLOAT)PI);

			GenerateStratifiedUniformHemisphereSamples(NumThetaSteps, NumPhiSteps, RandomStream, UniformHemisphereSamples);

			// Calculate incident radiance for each volume lighting sample
			for (TMap<INT,TArray<FVolumeLightingSample> >::TIterator It(VolumeLightingSamples); It; ++It)
			{
				TArray<FVolumeLightingSample>& CurrentLevelSamples = It.Value();
				for (INT SampleIndex = 0; SampleIndex < CurrentLevelSamples.Num(); SampleIndex++)
				{
					FVolumeLightingSample& CurrentSample = CurrentLevelSamples(SampleIndex);
					if (GeneralSettings.NumIndirectLightingBounces > 0 
						// Calculating incident radiance for volume samples requires final gathering, since photons are only stored on surfaces.
						&& (!PhotonMappingSettings.bUsePhotonMapping || PhotonMappingSettings.bUseFinalGathering))
					{
						 CalculateVolumeSampleIncidentRadiance(UniformHemisphereSamples, CurrentSample, RandomStream, MappingContext);
					}
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
					if (Scene.DebugMapping && DynamicObjectSettings.bVisualizeVolumeLightSamples)
					{
						FSHVectorRGB IncidentRadiance;
						CurrentSample.ToSHVector(IncidentRadiance);
						VolumeLightingDebugOutput.VolumeLightingSamples.AddItem(FDebugVolumeLightingSample(CurrentSample.PositionAndRadius, IncidentRadiance.CalcIntegral() / FSHVector::ConstantBasisIntegral));
					}
#endif
				}
			}

			MappingContext.Stats.TotalVolumeSampleLightingThreadTime += appSeconds() - SampleVolumeStart;
		}

		Stats.VolumeSampleThreadTime = appSeconds() - VolumeSampleStartTime;
	}
}

/** 
 * Interpolates lighting from the volume lighting samples to a vertex. 
 * This mirrors FPrecomputedLightVolume::InterpolateIncidentRadiance in UE3, used for visualizing interpolation from the lighting volume on surfaces.
 */
FGatheredLightSample FStaticLightingSystem::InterpolatePrecomputedVolumeIncidentRadiance(const FStaticLightingVertex& Vertex, FLOAT SampleRadius, FCoherentRayCache& RayCache, UBOOL bDebugThisTexel) const
{
	FGatheredLightSample IncidentRadiance;
	FSHVectorRGB TotalIncidentRadiance;
	FLOAT TotalWeight = 0.0f;

	if (bDebugThisTexel)
	{
		INT TempBreak = 0;
	}

	// Iterate over the octree nodes containing the query point.
	for (FVolumeLightingInterpolationOctree::TConstElementBoxIterator<> OctreeIt(VolumeLightingInterpolationOctree, FBoxCenterAndExtent(Vertex.WorldPosition, FVector4(0,0,0)));
		OctreeIt.HasPendingElements();
		OctreeIt.Advance())
	{
		const FVolumeSampleInterpolationElement& Element = OctreeIt.GetCurrentElement();
		const FVolumeLightingSample& VolumeSample = Element.VolumeSamples(Element.SampleIndex);

		const FLOAT DistanceSquared = (VolumeSample.GetPosition() - Vertex.WorldPosition).SizeSquared();
		if (DistanceSquared < Square(VolumeSample.GetRadius()))
		{
			/*
			FLightRayIntersection Intersection;
			const FLightRay SampleRay
				(Vertex.WorldPosition + Vertex.WorldTangentZ * SceneConstants.VisibilityNormalOffsetSampleRadiusScale * SampleRadius, 
				VolumeSample.GetPosition(), 
				NULL, 
				NULL);
			AggregateMesh.IntersectLightRay(SampleRay, FALSE, FALSE, FALSE, RayCache, Intersection);
			if (!Intersection.bIntersects)
			*/
			{
				const FLOAT SampleWeight = (1.0f - (Vertex.WorldPosition - VolumeSample.GetPosition()).Size() / VolumeSample.GetRadius()) / VolumeSample.GetRadius();
				//const FSHVectorRGB Temp = FSHVectorRGB(FQuantizedSHVectorRGB(VolumeSample.IncidentRadiance));
				//TotalIncidentRadiance += Temp * SampleWeight;
				TotalWeight += SampleWeight;
			}
		}
	}

	if (TotalWeight > DELTA)
	{
		// Normalize
		TotalIncidentRadiance = TotalIncidentRadiance * (1.0f / TotalWeight);
		// Convert from world space SH basis to tangent space FLightingSample
		for (INT CoeffIndex = 0; CoeffIndex < LM_NUM_GATHERED_LIGHTMAP_COEF; CoeffIndex++)
		{
			const FVector4& WorldLightmapBasis = Vertex.TransformTangentVectorToWorld(LightMapBasis[CoeffIndex]);
			const FSHVector CoefficientSH = SHBasisFunction(WorldLightmapBasis);
			const FLinearColor CoefficientIntensity = GetLightIntensity(TotalIncidentRadiance, CoefficientSH);
			checkSlow(CoefficientIntensity.AreFloatsValid());
			IncidentRadiance.Coefficients[CoeffIndex][0] = CoefficientIntensity.R;
			IncidentRadiance.Coefficients[CoeffIndex][1] = CoefficientIntensity.G;
			IncidentRadiance.Coefficients[CoeffIndex][2] = CoefficientIntensity.B;
		}
	}

	return IncidentRadiance;
}

}
