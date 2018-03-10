/*=============================================================================
	VertexMapping.cpp: Static lighting vertex mapping implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Scene.h"
#include "LightingSystem.h"
#include "LightmassSwarm.h"

namespace Lightmass
{

class FStaticLightingVertexMappingProcessor
{
public:

	/** The shadow-maps for the vertex mapping. */
	TMap<FLight*,FShadowMapData1D*> ShadowMapData;

	/** The light-map for the vertex mapping. */
	FGatheredLightMapData1D LightMapData;

	/** Initialization constructor. */
	FStaticLightingVertexMappingProcessor(FStaticLightingVertexMapping* InVertexMapping,FStaticLightingSystem* InSystem)
	:	LightMapData(InVertexMapping->Mesh->NumShadingVertices)
	,	VertexMapping(InVertexMapping)
	,	Mesh(InVertexMapping->Mesh)
	,	System(InSystem)
	,	CoherentRayCache()
	,	MappingContext(InVertexMapping->Mesh, *InSystem)
	{}

	/** Processses the vertex mapping. */
	FLightMapData1D* Process();

private:

	struct FAdjacentVertexInfo
	{
		INT VertexIndex;
		FLOAT Weight;
	};

	/** A sample for static vertex lighting. */
	struct FVertexLightingSample
	{
		FStaticLightingVertex SampleVertex;
		/** World space radius of the sample */
		FLOAT SampleRadius;
		TBitArray<> RelevantLightMask;
	};

	FStaticLightingVertexMapping* const VertexMapping;
	const FStaticLightingMesh* const Mesh;
	FStaticLightingSystem* const System;
	FCoherentRayCache CoherentRayCache;
	FStaticLightingMappingContext MappingContext;

	TArray<FVertexLightingSample> Samples;
	TMultiMap<INT,FAdjacentVertexInfo> SampleToAdjacentVertexMap;

	TArray<FStaticLightingVertex> Vertices;
	TArray<WORD> ElementIndices;
	TMultiMap<FVector4,INT> VertexPositionMap;
	TArray<FLOAT> VertexSampleWeightTotals;

	TArray<FShadowMapData1D*> ShadowMapDataByLightIndex;

	/**
	 * Caches the vertices of a mesh.
	 * @param Mesh - The mesh to cache vertices from.
	 * @param OutVertices - Upon return, contains the meshes vertices.
	 */
	void CacheVertices();

	/** Creates a list of samples for the mapping. */
	void CreateSamples();

	/** Calculates direct lighting for the vertex lighting samples, treating all lights as non-area lights. */
	void CalculateDirectLightingFiltered();

	/** 
	 * Estimates direct lighting for the vertex lighting samples from the direct photon map. 
	 * This is only useful for debugging what the final gather rays see.
	 */
	void CalculateDirectLightingPhotonMap();

	/** Calculates direct lighting from area lights */
	void CalculateDirectAreaLighting();

	/** Calculates indirect lighting for the vertex lighting samples. */
	void CalculateIndirectLighting();

	/** Calculates sky lighting for the vertex lighting samples. */
	void CalculateSkyLighting();

	/** Replaces the lighting values calculated so far with the material attribute specified by System->MaterialSettings.ViewMaterialAttribute. */
	void ViewMaterialAttributes();

	friend class FStaticLightingSystem;
};

FLightMapData1D* FStaticLightingVertexMappingProcessor::Process()
{
	const FStaticLightingMesh* const Mesh = VertexMapping->Mesh;
	const DOUBLE VertexSampleSetupStart = appSeconds();

	UBOOL bDebugThisMapping = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	bDebugThisMapping = VertexMapping == System->Scene.DebugMapping;
#endif

	// Cache the mesh's vertex data, and build a map from position to indices of vertices at that position.
	CacheVertices();

	// Allocate shadow-map data.
	ShadowMapDataByLightIndex.Empty(Mesh->RelevantLights.Num());
	ShadowMapDataByLightIndex.AddZeroed(Mesh->RelevantLights.Num());

	// Create the samples for the mesh.
	CreateSamples();

#if !LIGHTMASS_NOPROCESSING

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisMapping)
	{
		System->DebugOutput.bValid = TRUE;

		for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
		{
			const FVertexLightingSample& Sample = Samples(SampleIndex);
			const FLOAT DistanceToDebugVertexSq = (Sample.SampleVertex.WorldPosition - System->Scene.DebugInput.Position).SizeSquared();
			UBOOL bSampleOfDebugVertex = FALSE;
			for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				if (AdjacentVertexInfo.VertexIndex == System->Scene.DebugInput.VertexIndex)
				{
					bSampleOfDebugVertex = TRUE;
					break;
				}
			}

			if (DistanceToDebugVertexSq < 40000 || bSampleOfDebugVertex)
			{
				FDebugStaticLightingVertex DebugVertex;
				DebugVertex.VertexNormal = FVector4(Sample.SampleVertex.WorldTangentZ);
				DebugVertex.VertexPosition = Sample.SampleVertex.WorldPosition;
				System->DebugOutput.Vertices.AddItem(DebugVertex);
				if (bSampleOfDebugVertex)
				{
					System->DebugOutput.SelectedVertexIndices.AddItem(System->DebugOutput.Vertices.Num() - 1);
					System->DebugOutput.SampleRadius = 0;
				}
			}
		}
	}
#endif

	const DOUBLE DirectLightingStartTime = appSeconds();
	MappingContext.Stats.VertexSampleCreationTime += DirectLightingStartTime - VertexSampleSetupStart;
	if (System->ShadowSettings.bUseZeroAreaLightmapSpaceFilteredLights)
	{
		// Calculate direct lighting using non-area lights.
		CalculateDirectLightingFiltered();
	}
	else if (System->PhotonMappingSettings.bUsePhotonMapping && System->PhotonMappingSettings.bUsePhotonsForDirectLighting )
	{
		// Calculate direct lighting using the direct photon map.
		// This is only useful for debugging what the final gather rays see.
		CalculateDirectLightingPhotonMap();
	}
	else 
	{
		// Calculate direct lighting using area lights.
		CalculateDirectAreaLighting();
	}

	INT NumAffectingDominantLights = 0;
	for (INT LightIndex = 0; LightIndex < Mesh->RelevantLights.Num(); LightIndex++)
	{
		if (ShadowMapDataByLightIndex(LightIndex) && Mesh->RelevantLights(LightIndex)->LightFlags & GI_LIGHT_DOMINANT)
		{
			NumAffectingDominantLights++;
		}
	}

	if (NumAffectingDominantLights > 1)
	{
		const INT TypeId = Mesh->GetObjectType();
		GSwarm->SendAlertMessage(NSwarm::ALERT_LEVEL_ERROR, VertexMapping->Guid, SOURCEOBJECTTYPE_Mapping, TEXT("LightmassError_ObjectMultipleDominantLights"));
	}

	MappingContext.Stats.DirectLightingTime += appSeconds() - DirectLightingStartTime;

	CalculateIndirectLighting();

	// Calculate area lighting.
	CalculateSkyLighting();

	ViewMaterialAttributes();
#endif

	return LightMapData.ConvertToLightmap1D(bDebugThisMapping, System->Scene.DebugInput.VertexIndex);
}

void FStaticLightingVertexMappingProcessor::CacheVertices()
{
	Vertices.Empty(Mesh->NumShadingVertices);
	Vertices.Add(Mesh->NumShadingVertices);
	ElementIndices.Empty(Mesh->NumShadingVertices);
	ElementIndices.AddZeroed(Mesh->NumShadingVertices);
	FStaticLightingVertex DefaultVertex;
	appMemzero(&DefaultVertex, sizeof(DefaultVertex));
	DefaultVertex.WorldTangentX = FVector4(1,0,0);
	DefaultVertex.WorldTangentY = FVector4(0,1,0);
	DefaultVertex.WorldTangentZ = FVector4(0,0,1);
	for (INT i = 0; i < Vertices.Num(); i++)
	{
		// Initialize vertices to be valid,
		// Since some primitive types have vertices which are not referenced by any indices
		Vertices(i) = DefaultVertex;
	}
	
	for(INT TriangleIndex = 0;TriangleIndex < Mesh->NumShadingTriangles;TriangleIndex++)
	{
		// Query the mesh for the triangle's vertices.
		FStaticLightingVertex V0;
		FStaticLightingVertex V1;
		FStaticLightingVertex V2;
		INT Element;
		Mesh->GetShadingTriangle(TriangleIndex,V0,V1,V2,Element);

		checkSlow(V0.WorldTangentZ.IsUnit());
		checkSlow(V1.WorldTangentZ.IsUnit());
		checkSlow(V2.WorldTangentZ.IsUnit());

		INT I0 = 0;
		INT I1 = 0;
		INT I2 = 0;
		Mesh->GetShadingTriangleIndices(TriangleIndex,I0,I1,I2);

		// Cache the vertices by vertex index.
		Vertices(I0) = V0;
		Vertices(I1) = V1;
		Vertices(I2) = V2;

		ElementIndices(I0) = Element;
		ElementIndices(I1) = Element;
		ElementIndices(I2) = Element;

		// Also map the vertices by position.
		VertexPositionMap.AddUnique(V0.WorldPosition,I0);
		VertexPositionMap.AddUnique(V1.WorldPosition,I1);
		VertexPositionMap.AddUnique(V2.WorldPosition,I2);
	}
}

void FStaticLightingVertexMappingProcessor::CreateSamples()
{
	// Initialize the directly sampled vertex map.
	TBitArray<> DirectlySampledVertexMap;
	DirectlySampledVertexMap.Init(TRUE,Mesh->NumShadingVertices);

	TArray<FLOAT> VertexTriangleAreas;
	VertexTriangleAreas.Empty(Mesh->NumShadingVertices);
	VertexTriangleAreas.AddZeroed(Mesh->NumShadingVertices);
	
	// Allocate the vertex sample weight total map.
	VertexSampleWeightTotals.Empty(Mesh->NumShadingVertices);
	VertexSampleWeightTotals.AddZeroed(Mesh->NumShadingVertices);

	// Setup a thread-safe random stream with a fixed seed, so the sample points are deterministic.
	FRandomStream RandomStream(0);

	// Pre-allocate 4 samples per vertex 
	Samples.Empty(Mesh->NumShadingVertices * 4);

	// Calculate light visibility for each triangle.
	for(INT TriangleIndex = 0;TriangleIndex < Mesh->NumShadingTriangles;TriangleIndex++)
	{
		// Query the mesh for the triangle's vertex indices.
		INT TriangleVertexIndices[3];
		Mesh->GetShadingTriangleIndices(
			TriangleIndex,
			TriangleVertexIndices[0],
			TriangleVertexIndices[1],
			TriangleVertexIndices[2]
			);

		// Lookup the triangle's vertices.
		FStaticLightingVertex TriangleVertices[3];
		for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
		{
			TriangleVertices[VertexIndex] = Vertices(TriangleVertexIndices[VertexIndex]);
		}

		// Compute the triangle's normal.
		const FVector4 TriangleNormal = (TriangleVertices[2].WorldPosition - TriangleVertices[0].WorldPosition) ^ (TriangleVertices[1].WorldPosition - TriangleVertices[0].WorldPosition);

		const UBOOL bIsTwoSided = Mesh->IsTwoSided(ElementIndices(TriangleVertexIndices[0]));
		// Find the lights which are in front of this triangle.
		const TBitArray<> TriangleRelevantLightMask = CullBackfacingLights(
			bIsTwoSided,
			TriangleVertices[0].WorldPosition,
			TriangleNormal,
			Mesh->RelevantLights
			);

		// Compute the triangle's area.
		const FLOAT TriangleArea = 0.5f * TriangleNormal.Size();

		for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
		{
			VertexTriangleAreas(TriangleVertexIndices[VertexIndex]) = TriangleArea;
		}

		// Generate random samples on the faces of larger triangles.
		if(!VertexMapping->bSampleVertices)
		{
			// Compute the number of samples to use for the triangle, proportional to the triangle area.
			const INT NumSamples = Clamp(appTrunc(TriangleArea * VertexMapping->SampleToAreaRatio), 0, System->GeneralSettings.MaxTriangleLightingSamples);
			if(NumSamples)
			{
				// Look up the vertices adjacent to this triangle at each vertex.
				TArray<INT,TInlineAllocator<8> > AdjacentVertexIndices[3];
				for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
				{
					for(TMultiMap<FVector4,INT>::TConstKeyIterator VertexPositionMapIt(VertexPositionMap,TriangleVertices[VertexIndex].WorldPosition);
						VertexPositionMapIt;
						++VertexPositionMapIt)
					{
						AdjacentVertexIndices[VertexIndex].AddItem(VertexPositionMapIt.Value());
					}
				}

				// Weight the triangle's samples proportional to the triangle size, but independently of the number of samples.
				const FLOAT TriangleWeight = TriangleArea / NumSamples;

				// Sample the triangle's lighting.
				for(INT TriangleSampleIndex = 0;TriangleSampleIndex < NumSamples;TriangleSampleIndex++)
				{
					// Choose a uniformly distributed random point on the triangle.
					const FLOAT S = 1.0f - appSqrt(RandomStream.GetFraction());
					const FLOAT T = RandomStream.GetFraction() * (1.0f - S);
					const FLOAT U = 1 - S - T;

					// Index the sample's vertex indices and weights.
					const FLOAT TriangleVertexWeights[3] =
					{
						S * TriangleWeight,
						T * TriangleWeight,
						U * TriangleWeight
					};

					// Interpolate the triangle's vertex attributes at the sample point.
					FStaticLightingVertex SampleVertex =
						TriangleVertices[0] * S +
						TriangleVertices[1] * T +
						TriangleVertices[2] * U;

					// Handle opposite normals being interpolated and ending up with a 0 vector
					if (SampleVertex.WorldTangentZ.SizeSquared() < SMALL_NUMBER)
					{
						SampleVertex.WorldTangentZ = FVector4(0,0,1);
					}
					SampleVertex.WorldTangentZ = SampleVertex.WorldTangentZ.SafeNormal();
					SampleVertex.GenerateVertexTangents();

					// Create the sample.
					const INT SampleIndex = Samples.Num();
					FVertexLightingSample* Sample = new(Samples) FVertexLightingSample;
					Sample->SampleVertex = SampleVertex;
					// Approximate the sample radius as one side of a right triangle with the non-hypotenuse sides having equal length
					// And whose area is equal to the whole triangle's area / the number of samples.
					Sample->SampleRadius = Max(appSqrt(2.0f * TriangleWeight), System->SceneConstants.SmallestTexelRadius);
					Sample->RelevantLightMask = TriangleRelevantLightMask;

					// Build a list of vertices whose light-map values will affect this sample point.
					for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
					{
						for(INT AdjacentVertexIndex = 0;AdjacentVertexIndex < AdjacentVertexIndices[VertexIndex].Num();AdjacentVertexIndex++)
						{
							// Copy the adjacent vertex with its weight into the sample.
							FAdjacentVertexInfo AdjacentVertexInfo;
							AdjacentVertexInfo.VertexIndex = AdjacentVertexIndices[VertexIndex](AdjacentVertexIndex);
							AdjacentVertexInfo.Weight = TriangleVertexWeights[VertexIndex];
							SampleToAdjacentVertexMap.Add(SampleIndex,AdjacentVertexInfo);

							// Accumulate the vertex's sum of light-map sample weights.
							VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex) += TriangleVertexWeights[VertexIndex];

							// Indicate that the vertex doesn't need a direct sample.
							DirectlySampledVertexMap(AdjacentVertexInfo.VertexIndex) = FALSE;
						}
					}
				}
			}
		}
	}

	// Generate samples for vertices of small triangles.
	for(TConstSetBitIterator<> VertexIt(DirectlySampledVertexMap);VertexIt;++VertexIt)
	{
		const INT VertexIndex = VertexIt.GetIndex();
		const FStaticLightingVertex& SampleVertex = Vertices(VertexIndex);
		checkSlow(SampleVertex.WorldTangentZ.IsUnit());

		// Create the sample.
		const INT SampleIndex = Samples.Num();
		FVertexLightingSample* Sample = new(Samples) FVertexLightingSample;
		Sample->SampleVertex = SampleVertex;
		// Approximate the sample radius as one side of a right triangle with the non-hypotenuse sides having equal length
		// And whose area is equal to the area of one of the triangles the vertex belongs to.
		Sample->SampleRadius = Max(appSqrt(2.0f * VertexTriangleAreas(VertexIndex)), System->SceneConstants.SmallestTexelRadius);
		const UBOOL bIsTwoSided = Mesh->IsTwoSided(ElementIndices(VertexIndex));
		Sample->RelevantLightMask = CullBackfacingLights(
			bIsTwoSided,
			SampleVertex.WorldPosition,
			SampleVertex.WorldTangentZ,
			Mesh->RelevantLights
			);
		FAdjacentVertexInfo AdjacentVertexInfo;
		AdjacentVertexInfo.VertexIndex = VertexIndex;
		AdjacentVertexInfo.Weight = 1.0f;
		SampleToAdjacentVertexMap.Add(SampleIndex,AdjacentVertexInfo);

		// Set the vertex sample weight.
		VertexSampleWeightTotals(VertexIndex) = 1.0f;
	}

	MappingContext.Stats.NumVertexSamples = Samples.Num();
}

/** Calculates direct lighting for the vertex lighting samples, treating all lights as non-area lights. */
void FStaticLightingVertexMappingProcessor::CalculateDirectLightingFiltered()
{
	// Calculate direct lighting at the generated sample points.
	for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
	{
		const FVertexLightingSample& Sample = Samples(SampleIndex);
		const FStaticLightingVertex& SampleVertex = Sample.SampleVertex;

		// Add the sample's contribution to the vertex's shadow-map values.
		for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
		{
			if(Sample.RelevantLightMask(LightIndex))
			{
				FLight* Light = Mesh->RelevantLights(LightIndex);

				// Skip sky lights, since their static lighting is computed separately.
				if(Light->GetSkyLight())
				{
					continue;
				}

				// Compute the shadowing of this sample point from the light.
				const UBOOL bIsShadowed = System->CalculatePointShadowing(
					VertexMapping,
					SampleVertex.WorldPosition,
					Light,
					MappingContext,
					FALSE
					);

				if(!bIsShadowed)
				{
					// Accumulate the sample lighting and shadowing at the adjacent vertices.
					for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
						AdjacentVertexIt;
						++AdjacentVertexIt
						)
					{
						const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
						const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
						const FLOAT NormalizedWeight = AdjacentVertexInfo.Weight / VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex);

						// Determine whether to use a shadow-map or the light-map for this light.
						const UBOOL bUseStaticLighting = Light->UseStaticLighting(VertexMapping->bForceDirectLightMap);
						if(bUseStaticLighting)
						{
							// Use the adjacent vertex's tangent basis to calculate this sample's contribution to its light-map value.
							FStaticLightingVertex AdjacentSampleVertex = SampleVertex;
							AdjacentSampleVertex.WorldTangentX = AdjacentVertex.WorldTangentX;
							AdjacentSampleVertex.WorldTangentY = AdjacentVertex.WorldTangentY;
							AdjacentSampleVertex.WorldTangentZ = AdjacentVertex.WorldTangentZ;

							// Calculate the sample's direct lighting from this light.
							const FLinearColor LightIntensity = Light->GetDirectIntensity(AdjacentSampleVertex.WorldPosition, FALSE);
							const FGatheredLightSample DirectLighting = System->CalculatePointLighting(
								VertexMapping,
								AdjacentSampleVertex,
								ElementIndices(AdjacentVertexInfo.VertexIndex),
								Light,
								LightIntensity,
								FLinearColor::White
							);

							// Add the sampled direct lighting to the vertex's light-map value.
							LightMapData(AdjacentVertexInfo.VertexIndex).AddWeighted(DirectLighting,NormalizedWeight);

							// Add the light to the light-map's light list.
							LightMapData.Lights.AddUniqueItem(Light);
						}
						else
						{
							// Lookup the shadow-map used by this light.
							FShadowMapData1D* CurrentLightShadowMapData = ShadowMapDataByLightIndex(LightIndex);
							if(!CurrentLightShadowMapData)
							{
								// If this the first sample unshadowed from this light, create a shadow-map for it.
								CurrentLightShadowMapData = new FShadowMapData1D(Mesh->NumShadingVertices);
								ShadowMapDataByLightIndex(LightIndex) = CurrentLightShadowMapData;
								ShadowMapData.Set(Light,CurrentLightShadowMapData);
							}
								
							// Accumulate the sample shadowing.
							(*CurrentLightShadowMapData)(AdjacentVertexInfo.VertexIndex) += NormalizedWeight;
						}
					}
				}
			}
		}
	}
}

/** 
 * Estimates direct lighting for the vertex lighting samples from the direct photon map. 
 * This is only useful for debugging what the final gather rays see.
 */
void FStaticLightingVertexMappingProcessor::CalculateDirectLightingPhotonMap()
{
	TArray<FIrradiancePhoton*> TempIrradiancePhotons;

	for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
	{
		FLight* Light = Mesh->RelevantLights(LightIndex);
		if (Light->GetMeshAreaLight() == NULL)
		{
			LightMapData.AddLight(Light);
		}
	}

	// Calculate direct lighting at the generated sample points.
	for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
	{
		const FVertexLightingSample& Sample = Samples(SampleIndex);
		const FStaticLightingVertex& SampleVertex = Sample.SampleVertex;

		// Accumulate the sample lighting and shadowing at the adjacent vertices.
		for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
			AdjacentVertexIt;
			++AdjacentVertexIt
			)
		{
			const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
			const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
			const FLOAT NormalizedWeight = AdjacentVertexInfo.Weight / VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex);

			// Use the adjacent vertex's tangent basis to calculate this sample's contribution to its light-map value.
			FStaticLightingVertex AdjacentSampleVertex = SampleVertex;
			AdjacentSampleVertex.WorldTangentX = AdjacentVertex.WorldTangentX;
			AdjacentSampleVertex.WorldTangentY = AdjacentVertex.WorldTangentY;
			AdjacentSampleVertex.WorldTangentZ = AdjacentVertex.WorldTangentZ;

			FGatheredLightSample DirectLighting;
			if (System->PhotonMappingSettings.bUseIrradiancePhotons)
			{
				const FIrradiancePhoton* NearestPhoton = NULL;
				if (System->PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces)
				{
					// Trace a ray into the current vertex to get a good representation of what the final gather will see.
					// Speed does not matter here since bUsePhotonsForDirectLighting is only used for debugging.
					const FLightRay VertexRay(
						AdjacentSampleVertex.WorldPosition + AdjacentSampleVertex.WorldTangentZ * Sample.SampleRadius,
						AdjacentSampleVertex.WorldPosition - AdjacentSampleVertex.WorldTangentZ * Sample.SampleRadius,
						VertexMapping,
						NULL
						);

					FLightRayIntersection Intersection;
					System->AggregateMesh.IntersectLightRay(VertexRay, TRUE, FALSE, FALSE, MappingContext.RayCache, Intersection);

					INT CachedVertexIndex = AdjacentVertexInfo.VertexIndex;
					if (Intersection.bIntersects && VertexMapping == Intersection.Mapping)
					{
						AdjacentSampleVertex = Intersection.IntersectionVertex;
						CachedVertexIndex = Intersection.VertexIndex;
					}
					// Find the nearest irradiance photon that was cached on this surface
					NearestPhoton = VertexMapping->GetCachedIrradiancePhoton(CachedVertexIndex, AdjacentSampleVertex, *System, FALSE);
				}
				else
				{
					// Find the nearest irradiance photon by searching the irradiance photon map
					NearestPhoton = System->FindNearestIrradiancePhoton(AdjacentSampleVertex, MappingContext, TempIrradiancePhotons, FALSE, FALSE);
				}
				const FLinearColor& PhotonIrradiance = NearestPhoton ? NearestPhoton->GetIrradiance() : FLinearColor::Black;
				if (System->GeneralSettings.ViewSingleBounceNumber < 1)
				{
					DirectLighting.AddWeighted(FGatheredLightSample::PointLight(PhotonIrradiance, FVector4(0,0,1)), 1.0f);
				}
			}
			else
			{
				// Estimate incident radiance from the photons in the direct photon map
				const FGatheredLightSample PhotonIncidentRadiance = System->CalculatePhotonIncidentRadiance(
					System->DirectPhotonMap, 
					System->NumPhotonsEmittedDirect, 
					System->PhotonMappingSettings.DirectPhotonSearchDistance, 
					AdjacentSampleVertex, 
					FALSE);

				if (System->GeneralSettings.ViewSingleBounceNumber < 1)
				{
					DirectLighting.AddWeighted(PhotonIncidentRadiance, 1.0f);
				}
			}

			// Add the sampled direct lighting to the vertex's light-map value.
			LightMapData(AdjacentVertexInfo.VertexIndex).AddWeighted(DirectLighting, NormalizedWeight);
		}
	}
}

/** Calculates direct lighting from area lights */
void FStaticLightingVertexMappingProcessor::CalculateDirectAreaLighting()
{
	FRandomStream SampleGenerator(0);
	for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
	{
		FLight* Light = Mesh->RelevantLights(LightIndex);

		// Skip sky lights, since their static lighting is computed separately.
		if(Light->GetSkyLight())
		{
			continue;
		}

		const UBOOL bUseStaticLighting = Light->UseStaticLighting(VertexMapping->bForceDirectLightMap);
		if (bUseStaticLighting)
		{
			if (Light->GetMeshAreaLight() == NULL)
			{
				LightMapData.AddLight(Light);
			}
		}
		else
		{
			// Only allow for shadow maps if shadow casting is enabled.
			if ( (Light->LightFlags & GI_LIGHT_CASTSHADOWS) && (Light->LightFlags & GI_LIGHT_CASTSTATICSHADOWS) )
			{
				// Lookup the shadow-map used by this light.
				FShadowMapData1D* CurrentLightShadowMapData = ShadowMapDataByLightIndex(LightIndex);
				CurrentLightShadowMapData = new FShadowMapData1D(Mesh->NumShadingVertices);
				ShadowMapDataByLightIndex(LightIndex) = CurrentLightShadowMapData;
				ShadowMapData.Set(Light,CurrentLightShadowMapData);
			}
			else
			{
				// Using neither static lighting nor shadow maps; nothing to do.
				continue;
			}
		}

		UBOOL bIsCompletelyOccluded = TRUE;
		INT NumUnoccludedSamples = 0;
		// Calculate direct lighting at the generated sample points.
		for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
		{
			const FVertexLightingSample& Sample = Samples(SampleIndex);
			const FStaticLightingVertex& SampleVertex = Sample.SampleVertex;

			FVector4 AverageNormal(0,0,0);
			for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
				// Use the average adjacent triangle normal, weighted by area, to be representative for this sample
				AverageNormal += AdjacentVertex.WorldTangentZ * AdjacentVertexInfo.Weight;
			}
			
			FStaticLightingVertex RepresentativeVertex = SampleVertex;
			RepresentativeVertex.WorldTangentZ = AverageNormal.SizeSquared() > SMALL_NUMBER ? AverageNormal.SafeNormal() : RepresentativeVertex.WorldTangentZ;
			RepresentativeVertex.GenerateVertexTangents();

			INT ElementIndex = 0;
			for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				// Use the element index of one of this sample's adjacent vertices
				ElementIndex = ElementIndices(AdjacentVertexInfo.VertexIndex);
				break;
			}

			const UBOOL bLightIsInFrontOfTriangle = !Light->BehindSurface(RepresentativeVertex.WorldPosition, RepresentativeVertex.WorldTangentZ);
			const UBOOL bIsTwoSided = VertexMapping->Mesh->IsTwoSided(ElementIndex);
			if (bLightIsInFrontOfTriangle || bIsTwoSided)
			{
				// Approximate the integral over the light's surface to calculate incident direct radiance
				// As AverageVisibility * AverageIncidentRadiance
				//@todo - switch to the physically correct formulation which will allow us to handle area lights correctly,
				// Especially area lights with spatially varying emission
				const TArray<FLightSurfaceSample>& LightSurfaceSamples = Light->GetCachedSurfaceSamples(0, FALSE);
				FLinearColor UnnormalizedTransmission;
				const INT UnShadowedRays = System->CalculatePointAreaShadowing(
					VertexMapping, 
					RepresentativeVertex, 
					ElementIndex,
					Sample.SampleRadius,
					Light, 
					MappingContext, 
					SampleGenerator, 
					UnnormalizedTransmission,
					LightSurfaceSamples,
					FALSE);

				if (UnShadowedRays > 0)
				{
					NumUnoccludedSamples++;
					bIsCompletelyOccluded = FALSE;
					FLOAT ShadowFactor = 0.0f;
					FLinearColor Transmission;
					if (UnShadowedRays < LightSurfaceSamples.Num())
					{
						// Trace more shadow rays if we are in the penumbra
						const TArray<FLightSurfaceSample>& PenumbraLightSurfaceSamples = Light->GetCachedSurfaceSamples(0, TRUE);
						FLinearColor UnnormalizedPenumbraTransmission;
						const INT UnShadowedPenumbraRays = System->CalculatePointAreaShadowing(
							VertexMapping, 
							RepresentativeVertex, 
							ElementIndex,
							Sample.SampleRadius,
							Light, 
							MappingContext, 
							SampleGenerator, 
							UnnormalizedPenumbraTransmission,
							PenumbraLightSurfaceSamples,
							FALSE);

						// Linear combination of uniform and penumbra shadow samples
						//@todo - weight the samples by their solid angle PDF, not uniformly
						ShadowFactor = (UnShadowedRays + UnShadowedPenumbraRays) / (FLOAT)(LightSurfaceSamples.Num() + PenumbraLightSurfaceSamples.Num());
						// Weight each transmission by the fraction of total unshadowed rays that contributed to it
						Transmission = (UnnormalizedTransmission + UnnormalizedPenumbraTransmission) / (UnShadowedRays + UnShadowedPenumbraRays);
					}
					else
					{
						ShadowFactor = 1.0f;
						Transmission = UnnormalizedTransmission / UnShadowedRays;
					}
					
					// Accumulate the sample lighting and shadowing at the adjacent vertices.
					for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
						AdjacentVertexIt;
						++AdjacentVertexIt
						)
					{
						const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
						const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
						const FLOAT NormalizedWeight = AdjacentVertexInfo.Weight / VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex);

						const FLOAT AdjustedShadowFactor = appPow(ShadowFactor, Light->ShadowExponent);
						// Determine whether to use a shadow-map or the light-map for this light.
						if (bUseStaticLighting)
						{
							// Use the adjacent vertex's tangent basis to calculate this sample's contribution to its light-map value.
							FStaticLightingVertex AdjacentSampleVertex = SampleVertex;
							AdjacentSampleVertex.WorldTangentX = AdjacentVertex.WorldTangentX;
							AdjacentSampleVertex.WorldTangentY = AdjacentVertex.WorldTangentY;
							AdjacentSampleVertex.WorldTangentZ = AdjacentVertex.WorldTangentZ;

							const FLinearColor LightIntensity = Light->GetDirectIntensity(AdjacentSampleVertex.WorldPosition, FALSE);
							const FGatheredLightSample DirectLighting = System->CalculatePointLighting(
								VertexMapping,
								AdjacentSampleVertex,
								ElementIndices(AdjacentVertexInfo.VertexIndex),
								Light,
								LightIntensity,
								Transmission
							);
							
							if (System->GeneralSettings.ViewSingleBounceNumber < 1)
							{
								// Add the sampled direct lighting to the vertex's light-map value.
								LightMapData(AdjacentVertexInfo.VertexIndex).AddWeighted(DirectLighting, NormalizedWeight * AdjustedShadowFactor);
							}
						}
						else
						{
							// Lookup the shadow-map used by this light.
							FShadowMapData1D* CurrentLightShadowMapData = ShadowMapDataByLightIndex(LightIndex);
							// Accumulate the sample shadowing.
							(*CurrentLightShadowMapData)(AdjacentVertexInfo.VertexIndex) += NormalizedWeight * AdjustedShadowFactor * Transmission.LinearRGBToXYZ().G;
						}
					}
				}
			}
		}

		const UBOOL bIsShadowMappedLight = !bUseStaticLighting && (Light->LightFlags & GI_LIGHT_CASTSHADOWS) && (Light->LightFlags & GI_LIGHT_CASTSTATICSHADOWS);
		if (bIsShadowMappedLight && (bIsCompletelyOccluded || NumUnoccludedSamples < Samples.Num() * System->ShadowSettings.MinUnoccludedFraction))
		{
			delete ShadowMapDataByLightIndex(LightIndex);
			ShadowMapDataByLightIndex(LightIndex) = NULL;
			ShadowMapData.Remove(Light);
		}
	}
}

/** Calculates indirect lighting for the vertex lighting samples. */
void FStaticLightingVertexMappingProcessor::CalculateIndirectLighting()
{
	if (System->GeneralSettings.NumIndirectLightingBounces > 0 || System->AmbientOcclusionSettings.bUseAmbientOcclusion)
	{
		const DOUBLE IndirectLightingStartTime = appSeconds();
		FRandomStream SampleGenerator(0);

		// Calculate incident radiance from indirect lighting
		// With irradiance caching this is just the first pass, the results are added to the cache
		for (INT SampleIndex = 0; SampleIndex < Samples.Num(); SampleIndex++)
		{
			const FVertexLightingSample& Sample = Samples(SampleIndex);
			const FStaticLightingVertex& SampleVertex = Sample.SampleVertex;

			FVector4 AverageNormal(0,0,0);
			INT VertexIndex = INDEX_NONE;
			for (TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap, SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
				// Use the average adjacent triangle normal, weighted by area, to be representative for this sample
				AverageNormal += AdjacentVertex.WorldTangentZ * AdjacentVertexInfo.Weight;
				// Get the index of one of the vertices adjacent to this sample
				VertexIndex = AdjacentVertexInfo.VertexIndex;
			}
			checkSlow(VertexIndex >= 0);

			FStaticLightingVertex RepresentativeVertex = SampleVertex;
			RepresentativeVertex.WorldTangentZ = AverageNormal.SizeSquared() > SMALL_NUMBER ? AverageNormal.SafeNormal() : RepresentativeVertex.WorldTangentZ;
			RepresentativeVertex.GenerateVertexTangents();

			INT ElementIndex = 0;
			for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				// Use the element index of one of this sample's adjacent vertices
				ElementIndex = ElementIndices(AdjacentVertexInfo.VertexIndex);
				break;
			}

			// Calculate incoming radiance
			const FGatheredLightSample IndirectLightingSample = System->CachePointIncomingRadiance(
				VertexMapping, 
				RepresentativeVertex, 
				ElementIndex,
				VertexIndex,
				Sample.SampleRadius, 
				MappingContext, 
				SampleGenerator, 
				FALSE);

			FGatheredLightSample CausticLighting;
			// Only add caustic photon contribution if it was not already done through irradiance photons
			if (System->PhotonMappingSettings.bUsePhotonMapping
				&& System->GeneralSettings.NumIndirectLightingBounces > 0
				&& (!System->PhotonMappingSettings.bUseIrradiancePhotons || System->PhotonMappingSettings.bUseFinalGathering))
			{
				LIGHTINGSTAT(FScopedRDTSCTimer CausticPhotonGatherTimer(MappingContext.Stats.CausticPhotonGatherTime));
				// Calculate incident radiance from caustic photons
				CausticLighting = System->CalculatePhotonIncidentRadiance(
					System->CausticPhotonMap, 
					System->NumPhotonsEmittedCaustic, 
					System->PhotonMappingSettings.CausticPhotonSearchDistance, 
					RepresentativeVertex, 
					FALSE);
			}

			for (TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap, SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
				const FLOAT NormalizedWeight = AdjacentVertexInfo.Weight / VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex);

				FGatheredLightSample& CurrentLightSample = LightMapData(AdjacentVertexInfo.VertexIndex);
				if (System->GeneralSettings.ViewSingleBounceNumber != 0)
				{
					CurrentLightSample.AddWeighted(CausticLighting, NormalizedWeight);
				}
				if (!System->IrradianceCachingSettings.bAllowIrradianceCaching)
				{
					CurrentLightSample.AddWeighted(IndirectLightingSample, NormalizedWeight);
				}
			}
		}

		const DOUBLE IrradianceCacheInterpolationStartTime = appSeconds();
		MappingContext.Stats.IndirectLightingTime += IrradianceCacheInterpolationStartTime - IndirectLightingStartTime;
		if (System->IrradianceCachingSettings.bAllowIrradianceCaching)
		{
			// Reseed the sample generator
			SampleGenerator = FRandomStream(0);
			// Store indirect lighting and direct occlusion for each vertex separate from LightMapData for now,
			// So that we can composite indirect lighting and direct lighting masked by direct occlusion 
			// From multiple samples in the right order.
			TArray<FGatheredLightSample> IndirectLightingSamples;
			IndirectLightingSamples.Empty(Mesh->NumShadingVertices);
			IndirectLightingSamples.AddZeroed(Mesh->NumShadingVertices);

			TArray<FLOAT> DirectLightingOcclusion;
			DirectLightingOcclusion.Empty(Mesh->NumShadingVertices);
			DirectLightingOcclusion.AddZeroed(Mesh->NumShadingVertices);

			TArray<FLOAT> RawOcclusion;
			RawOcclusion.Empty(Mesh->NumShadingVertices);
			RawOcclusion.AddZeroed(Mesh->NumShadingVertices);

			// Interpolate irradiance cache samples in a separate shading pass
			// This avoids interpolating to positions where more samples will be added later, which would create a discontinuity
			// Also allows us to use more lenient restrictions in this pass, which effectively smooths the irradiance cache results
			for (INT SampleIndex = 0; SampleIndex < Samples.Num(); SampleIndex++)
			{
				const FVertexLightingSample& Sample = Samples(SampleIndex);
				const FStaticLightingVertex& SampleVertex = Sample.SampleVertex;

				FVector4 AverageNormal(0,0,0);
				for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
					AdjacentVertexIt;
					++AdjacentVertexIt
					)
				{
					const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
					const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
					// Use the average adjacent triangle normal, weighted by area, to be representative for this sample
					AverageNormal += AdjacentVertex.WorldTangentZ * AdjacentVertexInfo.Weight;
				}

				FStaticLightingVertex RepresentativeVertex = SampleVertex;
				RepresentativeVertex.WorldTangentZ = AverageNormal.SizeSquared() > SMALL_NUMBER ? AverageNormal.SafeNormal() : RepresentativeVertex.WorldTangentZ;
				RepresentativeVertex.GenerateVertexTangents();

				FFinalGatherSample IndirectLighting;
				// Interpolate the indirect lighting from the irradiance cache
				// Interpolation must succeed since this is the second pass
				verify(MappingContext.FirstBounceCache.InterpolateLighting(RepresentativeVertex, FALSE, FALSE, IndirectLighting));

				for (TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap, SampleIndex);
					AdjacentVertexIt;
					++AdjacentVertexIt
					)
				{
					const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
					const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
					const FLOAT NormalizedWeight = AdjacentVertexInfo.Weight / VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex);
					FGatheredLightSample& IndirectLightSample = IndirectLightingSamples(AdjacentVertexInfo.VertexIndex);
					FLOAT IndirectOcclusion = 1.0f;
					if (System->AmbientOcclusionSettings.bUseAmbientOcclusion)
					{
						const FLOAT DirectOcclusion = 1.0f - System->AmbientOcclusionSettings.DirectIlluminationOcclusionFraction * IndirectLighting.Occlusion;
						// Apply occlusion to direct lighting, assuming CurrentLightSample only contains direct lighting
						//@todo - currently this affects caustics as well
						FLOAT& VertexDirectOcclusion = DirectLightingOcclusion(AdjacentVertexInfo.VertexIndex);
						VertexDirectOcclusion += DirectOcclusion * NormalizedWeight;
						IndirectOcclusion = 1.0f - System->AmbientOcclusionSettings.IndirectIlluminationOcclusionFraction * IndirectLighting.Occlusion;
					}
					else
					{
						DirectLightingOcclusion(AdjacentVertexInfo.VertexIndex) = 1.0f;
					}

					// Apply occlusion to indirect lighting and add this texel's indirect lighting to its running total
					IndirectLightSample.AddWeighted(IndirectLighting, IndirectOcclusion * NormalizedWeight);
					if (System->AmbientOcclusionSettings.bUseAmbientOcclusion && System->AmbientOcclusionSettings.bVisualizeAmbientOcclusion)
					{
						RawOcclusion(AdjacentVertexInfo.VertexIndex) += IndirectLighting.Occlusion * NormalizedWeight;
					}
				}
			}

			for (INT VertexIndex = 0; VertexIndex < Mesh->NumShadingVertices; VertexIndex++)
			{
				// Lookup the current light sample for this vertex, which only has direct lighting so far
				FGatheredLightSample& CurrentLightingSample = LightMapData(VertexIndex);
				if (System->AmbientOcclusionSettings.bUseAmbientOcclusion && System->AmbientOcclusionSettings.bVisualizeAmbientOcclusion)
				{
					FLOAT RawOcclusionValue = RawOcclusion(VertexIndex);
					//@todo - this will only be the correct intensity for simple lightmaps
					const FGatheredLightSample OcclusionVisualization = FGatheredLightSample::PointLight(
						FLinearColor(1.0f - RawOcclusionValue, 1.0f - RawOcclusionValue, 1.0f - RawOcclusionValue) * 0.5f,
						FVector4(0,0,1));
					// Overwrite the lighting accumulated so far
					CurrentLightingSample = OcclusionVisualization;
					CurrentLightingSample.bIsMapped = TRUE;
				}
				else
				{
					// Lookup the direct lighting occlusion for this vertex, accumulated from all affecting samples
					FLOAT DirectOcclusion = DirectLightingOcclusion(VertexIndex);
					// Lookup the indirect lighting for this vertex, accumulated from all affecting samples
					const FGatheredLightSample& IndirectLightingSample = IndirectLightingSamples(VertexIndex);
					// Apply direct occlusion to the direct lighting and add indirect lighting
					CurrentLightingSample = CurrentLightingSample * DirectOcclusion + IndirectLightingSample;
				}
			}
		}

		const DOUBLE IrradianceCacheInterpolationEndTime = appSeconds();
		MappingContext.Stats.SecondPassIrradianceCacheInterpolationTime += IrradianceCacheInterpolationEndTime - IrradianceCacheInterpolationStartTime;
	}
}

void FStaticLightingVertexMappingProcessor::CalculateSkyLighting()
{
	// Add the sky lights to the light-map's light list.
	UBOOL bHasAreaLights = FALSE;
	UBOOL bHasShadowedAreaLights = FALSE;
	for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
	{
		FLight* Light = Mesh->RelevantLights(LightIndex);
		if(Light->GetSkyLight())
		{
			LightMapData.Lights.AddUniqueItem(Light);
			bHasAreaLights = TRUE;
			if ((Light->LightFlags & GI_LIGHT_CASTSHADOWS) && (Light->LightFlags & GI_LIGHT_CASTSTATICSHADOWS))
			{
				bHasShadowedAreaLights = TRUE;
			}
		}
	}

	FRandomStream SampleGenerator(0);
	// Populate the area lighting cache for the mesh.
	if(bHasShadowedAreaLights)
	{
		for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
		{
			const FVertexLightingSample& Sample = Samples(SampleIndex);
			System->CalculateSkyLighting(VertexMapping,Sample.SampleVertex,MappingContext,SampleGenerator);
		}
	}

	// Map the fully populated area lighting cache onto the mesh.
	if(bHasAreaLights)
	{
		for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
		{
			const FVertexLightingSample& Sample = Samples(SampleIndex);
			const FGatheredLightSample AreaLightingSample = System->CalculateSkyLighting(VertexMapping,Sample.SampleVertex,MappingContext,SampleGenerator);
			for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				const FLOAT NormalizedWeight = AdjacentVertexInfo.Weight / VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex);
				LightMapData(AdjacentVertexInfo.VertexIndex).AddWeighted(AreaLightingSample,NormalizedWeight);
			}
		}
	}
}

/** Replaces the lighting values calculated so far with the material attribute specified by System->MaterialSettings.ViewMaterialAttribute. */
void FStaticLightingVertexMappingProcessor::ViewMaterialAttributes()
{
	if (System->MaterialSettings.ViewMaterialAttribute != VMA_None)
	{
		for (INT VertexIndex = 0; VertexIndex < Mesh->NumShadingVertices; VertexIndex++)
		{
			// Clear whatever lighting has been calculated so far
			LightMapData(VertexIndex) = FGatheredLightSample();
		}
		for(INT SampleIndex = 0;SampleIndex < Samples.Num();SampleIndex++)
		{
			const FVertexLightingSample& Sample = Samples(SampleIndex);
			const FStaticLightingVertex& SampleVertex = Sample.SampleVertex;

			FVector4 AverageNormal(0,0,0);
			for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
				// Use the average adjacent triangle normal, weighted by area, to be representative for this sample
				AverageNormal += AdjacentVertex.WorldTangentZ * AdjacentVertexInfo.Weight;
			}

			FStaticLightingVertex RepresentativeVertex = SampleVertex;
			RepresentativeVertex.WorldTangentZ = AverageNormal.SizeSquared() > SMALL_NUMBER ? AverageNormal.SafeNormal() : RepresentativeVertex.WorldTangentZ;
			RepresentativeVertex.GenerateVertexTangents();

			// Trace a ray into the current vertex to get a good representation of what material lookups from ray intersections will see.
			// Speed does not matter here since this visualization is only used for debugging.
			const FLightRay VertexRay(
				RepresentativeVertex.WorldPosition + RepresentativeVertex.WorldTangentZ * Sample.SampleRadius,
				RepresentativeVertex.WorldPosition - RepresentativeVertex.WorldTangentZ * Sample.SampleRadius,
				VertexMapping,
				NULL
				);

			FLightRayIntersection Intersection;
			System->AggregateMesh.IntersectLightRay(VertexRay, TRUE, TRUE, FALSE, MappingContext.RayCache, Intersection);
			const FGatheredLightSample MaterialColorSample = System->GetVisualizedMaterialAttribute(VertexMapping, Intersection);

			for(TMultiMap<INT,FAdjacentVertexInfo>::TConstKeyIterator AdjacentVertexIt(SampleToAdjacentVertexMap,SampleIndex);
				AdjacentVertexIt;
				++AdjacentVertexIt
				)
			{
				const FAdjacentVertexInfo& AdjacentVertexInfo = AdjacentVertexIt.Value();
				const FStaticLightingVertex& AdjacentVertex = Vertices(AdjacentVertexInfo.VertexIndex);
				const FLOAT NormalizedWeight = AdjacentVertexInfo.Weight / VertexSampleWeightTotals(AdjacentVertexInfo.VertexIndex);

				FGatheredLightSample& CurrentLightSample = LightMapData(AdjacentVertexInfo.VertexIndex);
				CurrentLightSample.AddWeighted(MaterialColorSample, NormalizedWeight);
			}
		}
	}
}

/** Caches irradiance photons on a single vertex mapping. */
void FStaticLightingSystem::CacheIrradiancePhotonsVertexMapping(FStaticLightingVertexMapping* VertexMapping)
{
	checkSlow(VertexMapping);
	const FStaticLightingMesh* const Mesh = VertexMapping->Mesh;
	FStaticLightingMappingContext MappingContext(Mesh, *this);
	LIGHTINGSTAT(FScopedRDTSCTimer CachingTime(MappingContext.Stats.IrradiancePhotonCachingThreadTime));
	const FBoxSphereBounds ImportanceBounds = Scene.GetImportanceBounds();

	UBOOL bDebugThisMapping = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	bDebugThisMapping = VertexMapping == Scene.DebugMapping;
#endif

	// Setup a thread-safe random stream with a fixed seed, so the sample points are deterministic.
	FRandomStream RandomStream(0);

	// Initialize the directly sampled vertex map.
	TBitArray<> DirectlySampledVertexMap;
	DirectlySampledVertexMap.Init(TRUE, Mesh->NumVertices);

	// Allocate space for the cached irradiance photons
	VertexMapping->CachedIrradiancePhotons.Empty(Mesh->NumVertices);
	VertexMapping->CachedIrradiancePhotons.AddZeroed(Mesh->NumVertices);

	// Presize OutsideVolumeCachedIrradiancePhotons for the worst case so that it won't reallocate since we are storing pointers to its contents
	VertexMapping->OutsideVolumeCachedIrradiancePhotons.Empty(VertexMapping->CachedIrradiancePhotons.Num());

	TArray<FIrradiancePhoton*> TempIrradiancePhotons;
	for (INT TriangleIndex = 0; TriangleIndex < Mesh->NumTriangles; TriangleIndex++)
	{
		// Query the mesh for the triangle's vertex indices.
		INT TriangleVertexIndices[3];
		Mesh->GetTriangleIndices(
			TriangleIndex,
			TriangleVertexIndices[0],
			TriangleVertexIndices[1],
			TriangleVertexIndices[2]
		);

		FStaticLightingVertex TriangleVertices[3];
		INT DummyElement;
		Mesh->GetTriangle(TriangleIndex, TriangleVertices[0], TriangleVertices[1], TriangleVertices[2], DummyElement);

		// Compute the triangle's normal.
		const FVector4 TriangleNormal = (TriangleVertices[2].WorldPosition - TriangleVertices[0].WorldPosition) ^ (TriangleVertices[1].WorldPosition - TriangleVertices[0].WorldPosition);
		// Compute the triangle's area.
		const FLOAT TriangleArea = 0.5f * TriangleNormal.Size();
		// Compute the number of samples to use for the triangle, proportional to the triangle area.
		const INT NumSamples = Clamp(appTrunc(TriangleArea * VertexMapping->SampleToAreaRatio), 0, GeneralSettings.MaxTriangleIrradiancePhotonCacheSamples);

		// Generate random samples on the faces of larger triangles.
		if (!VertexMapping->bSampleVertices && NumSamples > 0)
		{
			// Sample the triangle's lighting.
			for(INT TriangleSampleIndex = 0; TriangleSampleIndex < NumSamples; TriangleSampleIndex++)
			{
				UBOOL bSearchForThisSample = FALSE;
				for (INT i = 0; i < 3; i++)
				{
					const FIrradiancePhoton* NearestPhoton = VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]);
					if (!NearestPhoton)
					{
						// Only search for this sample if any of the associated vertices have not found an irradiance photon
						bSearchForThisSample = TRUE;
						break;
					}
				}

				if (bSearchForThisSample)
				{
					// Weight the triangle's samples proportional to the triangle size, but independently of the number of samples.
					const FLOAT TriangleWeight = TriangleArea / NumSamples;

					// Choose a uniformly distributed random point on the triangle.
					const FLOAT S = 1.0f - appSqrt(RandomStream.GetFraction());
					const FLOAT T = RandomStream.GetFraction() * (1.0f - S);
					const FLOAT U = 1 - S - T;

					// Index the sample's vertex indices and weights.
					const FLOAT TriangleVertexWeights[3] =
					{
						S * TriangleWeight,
						T * TriangleWeight,
						U * TriangleWeight
					};

					// Interpolate the triangle's vertex attributes at the sample point.
					FStaticLightingVertex SampleVertex =
						TriangleVertices[0] * S +
						TriangleVertices[1] * T +
						TriangleVertices[2] * U;

					// Handle opposite normals being interpolated and ending up with a 0 vector
					if (SampleVertex.WorldTangentZ.SizeSquared() < SMALL_NUMBER)
					{
						SampleVertex.WorldTangentZ = FVector4(0,0,1);
					}
					SampleVertex.WorldTangentZ = SampleVertex.WorldTangentZ.SafeNormal();
					SampleVertex.GenerateVertexTangents();

					UBOOL bDebugThisSample = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
					if (bDebugThisMapping 
						&& (TriangleVertexIndices[0] == Scene.DebugInput.VertexIndex || TriangleVertexIndices[1] == Scene.DebugInput.VertexIndex || TriangleVertexIndices[2] == Scene.DebugInput.VertexIndex))
					{
						bDebugThisSample = TRUE;
					}
#endif

					// Only search the irradiance photon map if the surface cache position is inside the importance volume,
					// Since irradiance photons are only deposited inside the importance volume.
					if (ImportanceBounds.GetBox().IsInside(SampleVertex.WorldPosition))
					{
						// Find the nearest irradiance photon to this sample and store it in each associated vertex
						FIrradiancePhoton* NearestPhoton = FindNearestIrradiancePhoton(SampleVertex, MappingContext, TempIrradiancePhotons, TRUE, bDebugThisSample);
						if (NearestPhoton)
						{
							if (!NearestPhoton->IsUsed())
							{
								// An irradiance photon was found that hadn't been marked used yet
								MappingContext.Stats.NumFoundIrradiancePhotons++;
								NearestPhoton->SetUsed();
							}
							for (INT i = 0; i < 3; i++)
							{
								// Don't overwrite an existing irradiance photon assignment
								//@todo - filter the sample results in some way, or at least use the sample with the largest weight
								if (!VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]))
								{
									MappingContext.Stats.NumCachedIrradianceSamples++;
									DirectlySampledVertexMap(TriangleVertexIndices[i]) = FALSE;
									VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]) = NearestPhoton;
								}
							}
						}
					}
					else if (PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 0
						|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces >= 0)
					{
						// Create a dummy irradiance photon using direct illumination only.
						// This allows areas outside the importance volume to get one bounce of low quality GI.

						//@todo - why is the .5 factor needed to match up with deposited irradiance photons?
						const FLinearColor AccumulatedDirectIllumination = .5f * CalculateIncidentDirectIllumination(VertexMapping, SampleVertex, 1, MappingContext, RandomStream, TRUE, bDebugThisSample);

						FIrradiancePhoton NewIrradiancePhoton(SampleVertex.WorldPosition, SampleVertex.WorldTangentZ, FALSE);
						if (GeneralSettings.ViewSingleBounceNumber < 0
							|| PhotonMappingSettings.bUsePhotonsForDirectLighting && GeneralSettings.ViewSingleBounceNumber == 0 
							|| PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 1)
						{
							NewIrradiancePhoton.SetIrradiance(AccumulatedDirectIllumination);
						}
						VertexMapping->OutsideVolumeCachedIrradiancePhotons.AddItem((FIrradiancePhotonData&)NewIrradiancePhoton);
						for (INT i = 0; i < 3; i++)
						{
							// Don't overwrite an existing irradiance photon assignment
							//@todo - filter the sample results in some way, or at least use the sample with the largest weight
							if (!VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]))
							{
								DirectlySampledVertexMap(TriangleVertexIndices[i]) = FALSE;
								VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]) = (const FIrradiancePhoton*)&VertexMapping->OutsideVolumeCachedIrradiancePhotons.Last();
							}
						}

						MappingContext.Stats.NumIrradiancePhotonsOutsideVolume++;
					}
				}
			}
		}
	}

	for (INT TriangleIndex = 0; TriangleIndex < Mesh->NumTriangles; TriangleIndex++)
	{
		// Query the mesh for the triangle's vertex indices.
		INT TriangleVertexIndices[3];
		Mesh->GetTriangleIndices(
			TriangleIndex,
			TriangleVertexIndices[0],
			TriangleVertexIndices[1],
			TriangleVertexIndices[2]
		);

		UBOOL bSearchForThisTriangle = FALSE;
		for (INT i = 0; i < 3; i++)
		{
			if (DirectlySampledVertexMap(TriangleVertexIndices[i]))
			{
				const FIrradiancePhoton* NearestPhoton = VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]);
				if (!NearestPhoton)
				{
					bSearchForThisTriangle = TRUE;
					break;
				}
			}
		}

		if (bSearchForThisTriangle)
		{
			FStaticLightingVertex V0;
			FStaticLightingVertex V1;
			FStaticLightingVertex V2;
			INT DummyElement;
			Mesh->GetTriangle(TriangleIndex, V0, V1, V2, DummyElement);

			UBOOL bDebugThisSample = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisMapping 
				&& (TriangleVertexIndices[0] == Scene.DebugInput.VertexIndex || TriangleVertexIndices[1] == Scene.DebugInput.VertexIndex || TriangleVertexIndices[2] == Scene.DebugInput.VertexIndex))
			{
				bDebugThisSample = TRUE;
			}
#endif
			// Only search the irradiance photon map if the surface cache position is inside the importance volume,
			// Since irradiance photons are only deposited inside the importance volume.
			if (ImportanceBounds.GetBox().IsInside(V0.WorldPosition))
			{
				// Find the nearest irradiance photon for each vertex and store it in that vertex
				FIrradiancePhoton* NearestPhoton = FindNearestIrradiancePhoton(V0, MappingContext, TempIrradiancePhotons, TRUE, bDebugThisSample);
				if (NearestPhoton)
				{
					if (!NearestPhoton->IsUsed())
					{
						// An irradiance photon was found that hadn't been marked used yet
						MappingContext.Stats.NumFoundIrradiancePhotons++;
						NearestPhoton->SetUsed();
					}
					for (INT i = 0; i < 3; i++)
					{
						// Don't overwrite an existing irradiance photon assignment
						if (DirectlySampledVertexMap(TriangleVertexIndices[i]) && !VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]))
						{
							MappingContext.Stats.NumCachedIrradianceSamples++;
							VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]) = NearestPhoton;
						}
					}
				}
			}
			else if (PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 0
				|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces >= 0)
			{
				// Create a dummy irradiance photon using direct illumination only.
				// This allows areas outside the importance volume to get one bounce of low quality GI.

				//@todo - why is the .5 factor needed to match up with deposited irradiance photons?
				const FLinearColor AccumulatedDirectIllumination = .5f * CalculateIncidentDirectIllumination(VertexMapping, V0, 1, MappingContext, RandomStream, TRUE, bDebugThisSample);

				FIrradiancePhoton NewIrradiancePhoton(V0.WorldPosition, V0.WorldTangentZ, FALSE);
				if (GeneralSettings.ViewSingleBounceNumber < 0
					|| PhotonMappingSettings.bUsePhotonsForDirectLighting && GeneralSettings.ViewSingleBounceNumber == 0 
					|| PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 1)
				{
					NewIrradiancePhoton.SetIrradiance(AccumulatedDirectIllumination);
				}
				VertexMapping->OutsideVolumeCachedIrradiancePhotons.AddItem((FIrradiancePhotonData&)NewIrradiancePhoton);
				for (INT i = 0; i < 3; i++)
				{
					// Don't overwrite an existing irradiance photon assignment
					//@todo - filter the sample results in some way, or at least use the sample with the largest weight
					if (DirectlySampledVertexMap(TriangleVertexIndices[i]) && !VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]))
					{
						VertexMapping->CachedIrradiancePhotons(TriangleVertexIndices[i]) = (const FIrradiancePhoton*)&VertexMapping->OutsideVolumeCachedIrradiancePhotons.Last();
					}
				}

				MappingContext.Stats.NumIrradiancePhotonsOutsideVolume++;
			}
		}
	}
}

void FStaticLightingSystem::ProcessVertexMapping(FStaticLightingVertexMapping* VertexMapping)
{
	// time the processing
	const DOUBLE StartTime = appSeconds();

	// Process the vertex mapping.
	FStaticLightingVertexMappingProcessor Processor(VertexMapping,this);
	FLightMapData1D* LightMapData = Processor.Process();

	const DOUBLE ExecutionTime = appSeconds() - StartTime;

	// Enqueue the static lighting for application in the main thread.
	TList<FVertexMappingStaticLightingData>* StaticLightingLink = new TList<FVertexMappingStaticLightingData>(FVertexMappingStaticLightingData(),NULL);
	StaticLightingLink->Element.Mapping = VertexMapping;
	StaticLightingLink->Element.LightMapData = LightMapData;
	StaticLightingLink->Element.ShadowMaps = Processor.ShadowMapData;
	StaticLightingLink->Element.ExecutionTime = ExecutionTime;
	if (Processor.MappingContext.NumUniformSamples > 0)
	{
		StaticLightingLink->Element.PreviewEnvironmentShadowing = (BYTE)Clamp<INT>(appTrunc(appSqrt(Processor.MappingContext.NumUnshadowedEnvironmentSamples / (FLOAT)Processor.MappingContext.NumUniformSamples) * 255.0f), 0, 255);
	}
	else
	{
		StaticLightingLink->Element.PreviewEnvironmentShadowing = 0;
	}
	Processor.MappingContext.Stats.TotalVertexMappingLightingThreadTime = ExecutionTime;

#if LM_COMPRESS_ON_EACH_THREAD
	// compress on the thread if desired
	StaticLightingLink->Element.LightMapData->Compress(INDEX_NONE);
#endif

	CompleteVertexMappingList.AddElement(StaticLightingLink);

	const INT OldNumVerticesCompleted = appInterlockedAdd(&NumVerticesCompleted, VertexMapping->Mesh->NumShadingVertices);
	UpdateInternalStatus(NumTexelsCompleted, OldNumVerticesCompleted);
}


}