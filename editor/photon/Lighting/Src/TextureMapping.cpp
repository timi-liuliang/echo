/*=============================================================================
	TextureMapping.cpp: Static lighting texture mapping implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "../Inc/Raster.h"
#include "LightingSystem.h"
#include "LightmassSwarm.h"

namespace Lightmass
{

/** A map from light-map texels to the world-space surface points which map the texels. */
class FTexelToVertexMap
{
public:

	/** A map from a texel to the world-space surface point which maps the texel. */
	struct FTexelToVertex
	{
		FVector4 WorldPosition;
		FVector4 WorldTangentX;
		FVector4 WorldTangentY;
		FVector4 WorldTangentZ;

		/** Weight used when combining super sampled attributes and determining if the texel has been mapped. */
		FLOAT TotalSampleWeight;

		/** World space radius of the texel. */
		FLOAT TexelRadius;

		WORD ElementIndex;

		/** Texture coordinates */
		FVector2D TextureCoordinates[MAX_TEXCOORDS];

		/** Create a static lighting vertex to represent the texel. */
		FStaticLightingVertex GetVertex() const
		{
			FStaticLightingVertex Vertex;
			Vertex.WorldPosition = WorldPosition;
			Vertex.WorldTangentX = WorldTangentX;
			Vertex.WorldTangentY = WorldTangentY;
			Vertex.WorldTangentZ = WorldTangentZ;
			for( INT CurCoordIndex = 0; CurCoordIndex < MAX_TEXCOORDS; ++CurCoordIndex )
			{
				Vertex.TextureCoordinates[ CurCoordIndex ] = TextureCoordinates[ CurCoordIndex ];
			}
			return Vertex;
		}
	};

	/** Initialization constructor. */
	FTexelToVertexMap(INT InSizeX,INT InSizeY):
		Data(InSizeX * InSizeY),
		SizeX(InSizeX),
		SizeY(InSizeY)
	{
		// Clear the map to zero.
		for(INT Y = 0;Y < SizeY;Y++)
		{
			for(INT X = 0;X < SizeX;X++)
			{
				appMemzero(&(*this)(X,Y),sizeof(FTexelToVertex));
			}
		}
	}

	// Accessors.
	FTexelToVertex& operator()(INT X,INT Y)
	{
		const UINT TexelIndex = Y * SizeX + X;
		return Data(TexelIndex);
	}
	const FTexelToVertex& operator()(INT X,INT Y) const
	{
		const INT TexelIndex = Y * SizeX + X;
		return Data(TexelIndex);
	}

	INT GetSizeX() const { return SizeX; }
	INT GetSizeY() const { return SizeY; }
	SIZE_T GetAllocatedSize() const { return Data.GetAllocatedSize(); }

private:

	/** The mapping data. */
	TChunkedArray<FTexelToVertex> Data;

	/** The width of the mapping data. */
	INT SizeX;

	/** The height of the mapping data. */
	INT SizeY;
};

struct FStaticLightingInterpolant
{
	FStaticLightingVertex Vertex;
	WORD ElementIndex;

	FStaticLightingInterpolant() {}

	FStaticLightingInterpolant(const FStaticLightingVertex& InVertex, WORD InElementIndex) :
		Vertex(InVertex),
		ElementIndex(InElementIndex)
	{}

	// Operators used for linear combinations of static lighting interpolants.
	friend FStaticLightingInterpolant operator+(const FStaticLightingInterpolant& A,const FStaticLightingInterpolant& B)
	{
		FStaticLightingInterpolant Result;
		Result.Vertex = A.Vertex + B.Vertex; 
		Result.ElementIndex = A.ElementIndex;
		return Result;
	}

	friend FStaticLightingInterpolant operator-(const FStaticLightingInterpolant& A,const FStaticLightingInterpolant& B)
	{
		FStaticLightingInterpolant Result;
		Result.Vertex = A.Vertex - B.Vertex; 
		Result.ElementIndex = A.ElementIndex;
		return Result;
	}

	friend FStaticLightingInterpolant operator*(const FStaticLightingInterpolant& A,FLOAT B)
	{
		FStaticLightingInterpolant Result;
		Result.Vertex = A.Vertex * B; 
		Result.ElementIndex = A.ElementIndex;
		return Result;
	}

	friend FStaticLightingInterpolant operator/(const FStaticLightingInterpolant& A,FLOAT B)
	{
		FStaticLightingInterpolant Result;
		Result.Vertex = A.Vertex / B; 
		Result.ElementIndex = A.ElementIndex;
		return Result;
	}
};

/** Used to map static lighting texels to vertices. */
class FStaticLightingRasterPolicy
{
public:

	typedef FStaticLightingInterpolant InterpolantType;

    /** Initialization constructor. */
	FStaticLightingRasterPolicy(
		const FScene& InScene,
		FTexelToVertexMap& InTexelToVertexMap,
		FLOAT InSampleWeight,
		UBOOL bInDebugThisMapping,
		UBOOL bInUseMaxWeight
		) :
		Scene(InScene),
		TexelToVertexMap(InTexelToVertexMap),
		SampleWeight(InSampleWeight),
		bDebugThisMapping(bInDebugThisMapping),
		bUseMaxWeight(bInUseMaxWeight)
	{}

protected:

	// FTriangleRasterizer policy interface.

	INT GetMinX() const { return 0; }
	INT GetMaxX() const { return TexelToVertexMap.GetSizeX() - 1; }
	INT GetMinY() const { return 0; }
	INT GetMaxY() const { return TexelToVertexMap.GetSizeY() - 1; }

	void ProcessPixel(INT X,INT Y,const InterpolantType& Interpolant,UBOOL BackFacing);

private:

	const FScene& Scene;

	/** The texel to vertex map which is being rasterized to. */
	FTexelToVertexMap& TexelToVertexMap;

	/** The weight of the current sample. */
	const FLOAT SampleWeight;
	const UBOOL bDebugThisMapping;
	const UBOOL bUseMaxWeight;
};

void FStaticLightingRasterPolicy::ProcessPixel(INT X,INT Y,const InterpolantType& Interpolant,UBOOL BackFacing)
{
	FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
	UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisMapping
		&& X == Scene.DebugInput.LocalX
		&& Y == Scene.DebugInput.LocalY)
	{
		bDebugThisTexel = TRUE;
	}
#endif

	if (bUseMaxWeight && SampleWeight > TexelToVertex.TotalSampleWeight)
	{
		// Use the sample with the largest weight.  
		// This has the disadvantage compared averaging based on weight that it won't be well centered for texels on a UV seam,
		// And a texel spanning multiple triangles will only use the normal from one of those triangles,
		// But it has the advantage that the final position is guaranteed to be valid (ie actually on a triangle),
		// Even for split texels which are mapped to triangles in different parts of the mesh.
		TexelToVertex.TotalSampleWeight = SampleWeight;
		TexelToVertex.WorldPosition = Interpolant.Vertex.WorldPosition;
		TexelToVertex.WorldTangentX = Interpolant.Vertex.WorldTangentX;
		TexelToVertex.WorldTangentY = Interpolant.Vertex.WorldTangentY;
		TexelToVertex.WorldTangentZ = Interpolant.Vertex.WorldTangentZ;
		TexelToVertex.ElementIndex = Interpolant.ElementIndex;

		for( INT CurCoordIndex = 0; CurCoordIndex < MAX_TEXCOORDS; ++CurCoordIndex )
		{
			TexelToVertex.TextureCoordinates[ CurCoordIndex ] = Interpolant.Vertex.TextureCoordinates[ CurCoordIndex ];
		}
	}
	else if (!bUseMaxWeight)
	{
		// Update the sample weight, and compute the scales used to update the sample's averages.
		const FLOAT NewTotalSampleWeight = TexelToVertex.TotalSampleWeight + SampleWeight;		
		const FLOAT OldSampleWeight = TexelToVertex.TotalSampleWeight / NewTotalSampleWeight;	
		const FLOAT NewSampleWeight = SampleWeight / NewTotalSampleWeight;						
		TexelToVertex.TotalSampleWeight = NewTotalSampleWeight;	

		// Add this sample to the mapping.
		TexelToVertex.WorldPosition = TexelToVertex.WorldPosition * OldSampleWeight + Interpolant.Vertex.WorldPosition * NewSampleWeight;
		TexelToVertex.WorldTangentX = FVector4(TexelToVertex.WorldTangentX) * OldSampleWeight + Interpolant.Vertex.WorldTangentX * NewSampleWeight;
		TexelToVertex.WorldTangentY = FVector4(TexelToVertex.WorldTangentY) * OldSampleWeight + Interpolant.Vertex.WorldTangentY * NewSampleWeight;
		TexelToVertex.WorldTangentZ = FVector4(TexelToVertex.WorldTangentZ) * OldSampleWeight + Interpolant.Vertex.WorldTangentZ * NewSampleWeight;
		TexelToVertex.ElementIndex = Interpolant.ElementIndex;
		
		for( INT CurCoordIndex = 0; CurCoordIndex < MAX_TEXCOORDS; ++CurCoordIndex )
		{
			TexelToVertex.TextureCoordinates[ CurCoordIndex ] = TexelToVertex.TextureCoordinates[ CurCoordIndex ] * OldSampleWeight + Interpolant.Vertex.TextureCoordinates[ CurCoordIndex ] * NewSampleWeight;
		}
	}
}

/** Caches irradiance photons on a single texture mapping. */
void FStaticLightingSystem::CacheIrradiancePhotonsTextureMapping(FStaticLightingTextureMapping* TextureMapping)
{
	checkSlow(TextureMapping);
	FStaticLightingMappingContext MappingContext(TextureMapping->Mesh,*this);
	LIGHTINGSTAT(FScopedRDTSCTimer CachingTime(MappingContext.Stats.IrradiancePhotonCachingThreadTime));
	const FBoxSphereBounds ImportanceBounds = Scene.GetImportanceBounds();

	// Cache irradiance photons at a lower resolution than what lighting is being calculated for, since the extra resolution is usually not noticeable
	TextureMapping->IrradiancePhotonCacheSizeX = Max(appTrunc(TextureMapping->CachedSizeX / PhotonMappingSettings.CachedIrradiancePhotonDownsampleFactor), 6);
	TextureMapping->IrradiancePhotonCacheSizeY = Max(appTrunc(TextureMapping->CachedSizeY / PhotonMappingSettings.CachedIrradiancePhotonDownsampleFactor), 6);

	FTexelToVertexMap TexelToVertexMap(TextureMapping->IrradiancePhotonCacheSizeX, TextureMapping->IrradiancePhotonCacheSizeY);

	UBOOL bDebugThisMapping = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	bDebugThisMapping = TextureMapping == Scene.DebugMapping;
	INT IrradiancePhotonCacheDebugX = -1;
	INT IrradiancePhotonCacheDebugY = -1;
	if (bDebugThisMapping)
	{
		IrradiancePhotonCacheDebugX = appTrunc(Scene.DebugInput.LocalX / (FLOAT)TextureMapping->CachedSizeX * TextureMapping->IrradiancePhotonCacheSizeX);
		IrradiancePhotonCacheDebugY = appTrunc(Scene.DebugInput.LocalY / (FLOAT)TextureMapping->CachedSizeY * TextureMapping->IrradiancePhotonCacheSizeY);
	}
#endif

	const FLOAT SampleWeight = 1.0f;
	for (INT TriangleIndex = 0; TriangleIndex < TextureMapping->Mesh->NumTriangles; TriangleIndex++)
	{
		// Query the mesh for the triangle's vertices.
		FStaticLightingInterpolant V0;
		FStaticLightingInterpolant V1;
		FStaticLightingInterpolant V2;
		INT Element;
		TextureMapping->Mesh->GetTriangle(TriangleIndex, V0.Vertex, V1.Vertex, V2.Vertex, Element);
		V0.ElementIndex = V1.ElementIndex = V2.ElementIndex = Element;

		// Rasterize the triangle using the mapping's texture coordinate channel.
		FTriangleRasterizer<FStaticLightingRasterPolicy> TexelMappingRasterizer(FStaticLightingRasterPolicy(
				Scene,
				TexelToVertexMap,
				SampleWeight,
				FALSE,
				FALSE
				));
		TexelMappingRasterizer.DrawTriangle(
			V0,
			V1,
			V2,
			V0.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->IrradiancePhotonCacheSizeX,TextureMapping->IrradiancePhotonCacheSizeY) + FVector2D(-0.5f,-0.5f),
			V1.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->IrradiancePhotonCacheSizeX,TextureMapping->IrradiancePhotonCacheSizeY) + FVector2D(-0.5f,-0.5f),
			V2.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->IrradiancePhotonCacheSizeX,TextureMapping->IrradiancePhotonCacheSizeY) + FVector2D(-0.5f,-0.5f),
			FALSE
			);
	}

	// Allocate space for the cached irradiance photons
	TextureMapping->CachedIrradiancePhotons.Empty(TextureMapping->IrradiancePhotonCacheSizeX * TextureMapping->IrradiancePhotonCacheSizeY);
	TextureMapping->CachedIrradiancePhotons.AddZeroed(TextureMapping->IrradiancePhotonCacheSizeX * TextureMapping->IrradiancePhotonCacheSizeY);
	// Presize OutsideVolumeCachedIrradiancePhotons for the worst case so that it won't reallocate since we are storing pointers to its contents
	TextureMapping->OutsideVolumeCachedIrradiancePhotons.Empty(TextureMapping->CachedIrradiancePhotons.Num());
	TArray<FIrradiancePhoton*> TempIrradiancePhotons;
	FRandomStream RandomStream(0);
	for (INT Y = 0; Y < TextureMapping->IrradiancePhotonCacheSizeY; Y++)
	{
		for (INT X = 0; X < TextureMapping->IrradiancePhotonCacheSizeX; X++)
		{
			UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisMapping
				&& Y == IrradiancePhotonCacheDebugY
				&& X == IrradiancePhotonCacheDebugX)
			{
				bDebugThisTexel = TRUE;
			}
#endif
			const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
			if (TexelToVertex.TotalSampleWeight > 0.0f)
			{
				MappingContext.Stats.NumCachedIrradianceSamples++;
				const FStaticLightingVertex CurrentVertex = TexelToVertex.GetVertex();
				
				// Only search the irradiance photon map if the surface cache position is inside the importance volume,
				// Since irradiance photons are only deposited inside the importance volume.
				if (ImportanceBounds.GetBox().IsInside(CurrentVertex.WorldPosition))
				{
					// Find the nearest irradiance photon and store it on the surface of the mapping
					// Only find visible irradiance photons to prevent light leaking through thin surfaces
					//Note: It's still possible for light to leak if a single texel spans two disjoint lighting areas, for example two planes coming together at a 90 degree angle.
					FIrradiancePhoton* NearestPhoton = FindNearestIrradiancePhoton(CurrentVertex, MappingContext, TempIrradiancePhotons, TRUE, bDebugThisTexel);
					if (NearestPhoton)
					{
						if (!NearestPhoton->IsUsed())
						{
							// An irradiance photon was found that hadn't been marked used yet
							MappingContext.Stats.NumFoundIrradiancePhotons++;
							NearestPhoton->SetUsed();
						}
						TextureMapping->CachedIrradiancePhotons(Y * TextureMapping->IrradiancePhotonCacheSizeX + X) = NearestPhoton;
					}
				}
				else if (PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 0
					|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces >= 0)
				{
					// Search for a nearby irradiance photon to use
					FLOAT NearestDistanceSquared = FLT_MAX;
					const FIrradiancePhoton* NearestPhoton = NULL;
					for (INT SearchOffsetY = 0; SearchOffsetY > -15 && (Y + SearchOffsetY >= 0); SearchOffsetY--)
					{
						for (INT SearchOffsetX = 0; SearchOffsetX > -15 && (X + SearchOffsetX >= 0); SearchOffsetX--)
						{
							const FIrradiancePhoton* SearchPhoton = TextureMapping->CachedIrradiancePhotons((Y + SearchOffsetY) * TextureMapping->IrradiancePhotonCacheSizeX + X + SearchOffsetX);
							if (SearchPhoton)
							{
								const FLOAT DistanceSquared = (SearchPhoton->GetPosition() - TexelToVertex.WorldPosition).SizeSquared();
								if (DistanceSquared < Square(PhotonMappingSettings.OutsideVolumeIrradiancePhotonDistanceThreshold) 
									&& DistanceSquared < NearestDistanceSquared
									&& (SearchPhoton->GetSurfaceNormal() | TexelToVertex.WorldTangentZ) > .707f)
								{
									NearestDistanceSquared = DistanceSquared;
									NearestPhoton = SearchPhoton;
								}
							}
						}
					}
					if (NearestPhoton)
					{
						TextureMapping->CachedIrradiancePhotons(Y * TextureMapping->IrradiancePhotonCacheSizeX + X) = NearestPhoton;
					}
					else
					{
						// Create a dummy irradiance photon using direct illumination only.
						// This allows areas outside the importance volume to get one bounce of low quality GI.

						//@todo - why is the .5 factor needed to match up with deposited irradiance photons?
						const FLinearColor AccumulatedDirectIllumination = .5f * CalculateIncidentDirectIllumination(TextureMapping, CurrentVertex, 1, MappingContext, RandomStream, TRUE, bDebugThisTexel);
						
						FIrradiancePhoton NewIrradiancePhoton(TexelToVertex.WorldPosition, TexelToVertex.WorldTangentZ, FALSE);
						if (GeneralSettings.ViewSingleBounceNumber < 0
							|| PhotonMappingSettings.bUsePhotonsForDirectLighting && GeneralSettings.ViewSingleBounceNumber == 0 
							|| PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 1)
						{
							NewIrradiancePhoton.SetIrradiance(AccumulatedDirectIllumination);
						}
						TextureMapping->OutsideVolumeCachedIrradiancePhotons.AddItem((FIrradiancePhotonData&)NewIrradiancePhoton);
						TextureMapping->CachedIrradiancePhotons(Y * TextureMapping->IrradiancePhotonCacheSizeX + X) = (const FIrradiancePhoton*)&TextureMapping->OutsideVolumeCachedIrradiancePhotons.Last();
						MappingContext.Stats.NumIrradiancePhotonsOutsideVolume++;
					}
				}
			}
		}
	}
}

/**
 * Builds lighting for a texture mapping.
 * @param TextureMapping - The mapping to build lighting for.
 */
void FStaticLightingSystem::ProcessTextureMapping(FStaticLightingTextureMapping* TextureMapping)
{
	checkSlow(TextureMapping);
	// calculate the total time just for processing
	DOUBLE StartTime = appSeconds();

	UBOOL bDebugThisMapping = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	bDebugThisMapping = TextureMapping == Scene.DebugMapping;
#endif

	// light guid to shadow map mapping
	TMap<const FLight*, FShadowMapData2D*> ShadowMaps;
	TMap<const FLight*, FSignedDistanceFieldShadowMapData2D*> SignedDistanceFieldShadowMaps;
	FStaticLightingMappingContext MappingContext(TextureMapping->Mesh, *this);

	// Allocate light-map data.
	FGatheredLightMapData2D LightMapData(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);

	// if we have a debug texel, then only compute the lighting for this mapping
	UBOOL bCalculateThisMapping = TRUE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	// we want to skip mappings if the setting is enabled, and we have a debug mapping, and it's not this one
	bCalculateThisMapping = !(Scene.bOnlyCalcDebugTexelMappings && Scene.DebugMapping != NULL && !bDebugThisMapping);
#endif

	// Allocate the texel to vertex map.
	FTexelToVertexMap TexelToVertexMap(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);

	const DOUBLE TexelRasterizationStart = appSeconds();
	// Allocate a map from texel to the corners of that texel
	FTexelToCornersMap TexelToCornersMap(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);
	SetupTextureMapping(TextureMapping, LightMapData, TexelToVertexMap, TexelToCornersMap, MappingContext, bDebugThisMapping);
	MappingContext.Stats.TexelRasterizationTime += appSeconds() - TexelRasterizationStart;

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisMapping)
	{
		DebugOutput.bValid = TRUE;
		DebugOutput.Vertices.Empty(TextureMapping->CachedSizeY * TextureMapping->CachedSizeX);
		for (INT Y = 0;Y < TextureMapping->CachedSizeY;Y++)
		{
			for (INT X = 0;X < TextureMapping->CachedSizeX;X++)
			{
				const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
				if (TexelToVertex.TotalSampleWeight > 0.0f)
				{
					// Verify that vertex normals are normalized (within some error that is large because of FPackedNormal)
					checkSlow(TexelToVertex.WorldTangentZ.IsUnit(.1f));

					const FLOAT DistanceToDebugTexelSq = (TexelToVertex.WorldPosition - Scene.DebugInput.Position).SizeSquared();
					if (DistanceToDebugTexelSq < 40000
						|| X == Scene.DebugInput.LocalX && Y == Scene.DebugInput.LocalY)
					{
						FDebugStaticLightingVertex DebugVertex;
						DebugVertex.VertexNormal = FVector4(TexelToVertex.WorldTangentZ);
						DebugVertex.VertexPosition = TexelToVertex.WorldPosition;
						DebugOutput.Vertices.AddItem(DebugVertex);
						if (X == Scene.DebugInput.LocalX && Y == Scene.DebugInput.LocalY)
						{
							DebugOutput.SelectedVertexIndices.AddItem(DebugOutput.Vertices.Num() - 1);
							DebugOutput.SampleRadius = TexelToVertex.TexelRadius;
						}
					}
				}
			}
		}
	}
#endif

#if !LIGHTMASS_NOPROCESSING

	if (bCalculateThisMapping)
	{
		const DOUBLE DirectLightingStartTime = appSeconds();
		const UBOOL bCalculateDirectLightingFromPhotons = PhotonMappingSettings.bUsePhotonMapping && PhotonMappingSettings.bUsePhotonsForDirectLighting;
		// Only continue if photon mapping will not be used for direct lighting
		if (!bCalculateDirectLightingFromPhotons)
		{
			// Iterate over each light that is relevant to the direct lighting of the mesh
			for (INT LightIndex = 0; LightIndex < TextureMapping->Mesh->RelevantLights.Num(); LightIndex++)
			{
				const FLight* Light = TextureMapping->Mesh->RelevantLights(LightIndex);

				// skip sky lights for now
				if (Light->GetSkyLight())
				{
					continue;
				}

				if (!Light->AffectsBounds(FBoxSphereBounds(TextureMapping->Mesh->BoundingBox)))
				{
					continue;
				}

				if ( ShadowSettings.bUseZeroAreaLightmapSpaceFilteredLights)
				{
					// Calculate direct lighting from lights as if they have no area, and then filter in texture space to create approximate penumbras.
					CalculateDirectLightingTextureMappingFiltered(TextureMapping, MappingContext, LightMapData, ShadowMaps, TexelToVertexMap, bDebugThisMapping, Light);
				}
				else 
				{
					if ( !Light->UseStaticLighting(TextureMapping->bForceDirectLightMap)
						&& (Light->LightFlags & GI_LIGHT_CASTSHADOWS) 
						&& (Light->LightFlags & GI_LIGHT_CASTSTATICSHADOWS)
						&& (Light->LightFlags & GI_LIGHT_USESIGNEDDISTANCEFIELDSHADOWS)
						&& ShadowSettings.bAllowSignedDistanceFieldShadows)
					{
						// Calculate distance field shadows, where the distance to the nearest shadow transition is stored instead of just a [0,1] shadow factor.
						CalculateDirectSignedDistanceFieldLightingTextureMapping(TextureMapping, MappingContext, LightMapData, SignedDistanceFieldShadowMaps, TexelToVertexMap, TexelToCornersMap, bDebugThisMapping, Light);

						// Also calculate static lighting for simple light maps.  We'll force the shadows into simple light maps, but
						// won't actually add the lights to the light guid list.  Instead, at runtime we'll check the shadow map guids
						// for lights that are baked into light maps on platforms that don't support shadow mapping.
						const UBOOL bForceShadowIntoSimpleLightMap = TRUE;
						CalculateDirectAreaLightingTextureMapping(TextureMapping, MappingContext, LightMapData, ShadowMaps, TexelToVertexMap, bDebugThisMapping, Light, bForceShadowIntoSimpleLightMap);
					}
					else
					{
						// Calculate direct lighting from area lights, no filtering in texture space.  
						// Shadow penumbras will be correctly shaped and will be softer for larger light sources and distant shadow casters.
						const UBOOL bForceShadowIntoSimpleLightMap = FALSE;
						CalculateDirectAreaLightingTextureMapping(TextureMapping, MappingContext, LightMapData, ShadowMaps, TexelToVertexMap, bDebugThisMapping, Light, bForceShadowIntoSimpleLightMap);
					}
				}
			}
		}

		// Release corner information as it is no longer needed
		TexelToCornersMap.Empty();

		INT NumAffectingDominantLights = 0;
		for (TMap<const FLight*,FSignedDistanceFieldShadowMapData2D*>::TIterator It(SignedDistanceFieldShadowMaps); It; ++It)
		{
			if (It.Key()->LightFlags & GI_LIGHT_DOMINANT)
			{
				NumAffectingDominantLights++;
			}
		}

		for (TMap<const FLight*,FShadowMapData2D*>::TIterator It(ShadowMaps); It; ++It)
		{
			if (It.Key()->LightFlags & GI_LIGHT_DOMINANT)
			{
				NumAffectingDominantLights++;
			}
		}

		if (NumAffectingDominantLights > 1) 
		{
			GSwarm->SendAlertMessage(NSwarm::ALERT_LEVEL_ERROR, TextureMapping->Guid, SOURCEOBJECTTYPE_Mapping, TEXT("LightmassError_ObjectMultipleDominantLights"));
		}

		// Calculate direct lighting using the direct photon map.
		// This is only useful for debugging what the final gather rays see.
		if (bCalculateDirectLightingFromPhotons)
		{
			CalculateDirectLightingTextureMappingPhotonMap(TextureMapping, MappingContext, LightMapData, ShadowMaps, TexelToVertexMap, bDebugThisMapping);
		}
		MappingContext.Stats.DirectLightingTime += appSeconds() - DirectLightingStartTime;

		CalculateIndirectLightingTextureMapping(TextureMapping, MappingContext, LightMapData, TexelToVertexMap, bDebugThisMapping);

		const DOUBLE ErrorAndMaterialColoringStart = appSeconds();
		ViewMaterialAttributesTextureMapping(TextureMapping, MappingContext, LightMapData, TexelToVertexMap, bDebugThisMapping);
		ColorInvalidLightmapUVs(TextureMapping, LightMapData, bDebugThisMapping);

		// Count the time doing material coloring and invalid lightmap UV color toward texel setup for now
		MappingContext.Stats.TexelRasterizationTime += appSeconds() - ErrorAndMaterialColoringStart;
	}
#endif

	const DOUBLE PaddingStart = appSeconds();
	
	FGatheredLightMapData2D PaddedLightMapData(TextureMapping->SizeX, TextureMapping->SizeY);
	PadTextureMapping(TextureMapping, LightMapData, PaddedLightMapData, ShadowMaps, SignedDistanceFieldShadowMaps);
	LightMapData.Empty();

	// calculate the total time just for processing
	const DOUBLE ExecutionTimeForColoring = appSeconds() - StartTime;

	if (!bCalculateThisMapping || Scene.bColorBordersGreen || Scene.bColorByExecutionTime || Scene.bUseRandomColors)
	{
		UBOOL bColorNonBorders = Scene.bColorByExecutionTime || Scene.bUseRandomColors;

		// calculate what color to put in each spot, if overriding
		FLinearColor OverrideColor(0, 0, 0);
		if (Scene.bColorByExecutionTime)
		{
			OverrideColor.R = ExecutionTimeForColoring / (Scene.ExecutionTimeDivisor ? Scene.ExecutionTimeDivisor : 15.0f);
		}
		else if (Scene.bUseRandomColors)
		{
			// make each mapping solid, random colors
			static FRandomStream RandomStream(0);

			// make a random color
			OverrideColor.R = RandomStream.GetFraction();
			OverrideColor.G = RandomStream.GetFraction();
			OverrideColor.B = RandomStream.GetFraction();

			if (Scene.bColorBordersGreen)
			{
				// not too green tho so borders show up
				OverrideColor.G /= 2.0f;
			}
		}
		else if (!bCalculateThisMapping)
		{
			OverrideColor = FLinearColor::White;
		}

		FLinearColor Green(0, 1.0, 0);

		for (UINT Y = 0; Y < PaddedLightMapData.GetSizeY(); Y++)
		{
			for (UINT X = 0; X < PaddedLightMapData.GetSizeX(); X++)
			{
				FGatheredLightSample& Sample = PaddedLightMapData(X, Y);
				UBOOL bIsBorder = (X <= 1 || Y <= 1 || X >= PaddedLightMapData.GetSizeX() - 2 || Y >= PaddedLightMapData.GetSizeY() - 2);
				if (!bCalculateThisMapping || (Sample.bIsMapped && bColorNonBorders) || (bIsBorder && Scene.bColorBordersGreen))
				{
					FLinearColor& SampleColor = (bIsBorder && Scene.bColorBordersGreen) ? Green : OverrideColor;

					for (INT CoefficientIndex = 0; CoefficientIndex < LM_NUM_GATHERED_LIGHTMAP_COEF; CoefficientIndex++)
					{
						Sample.Coefficients[CoefficientIndex][0] = SampleColor.R;
						Sample.Coefficients[CoefficientIndex][1] = SampleColor.G;
						Sample.Coefficients[CoefficientIndex][2] = SampleColor.B;
					}
				}
			}
		}
	}

	const INT PaddedDebugX = TextureMapping->bPadded ? Scene.DebugInput.LocalX + 1 : Scene.DebugInput.LocalX;
	const INT PaddedDebugY = TextureMapping->bPadded ? Scene.DebugInput.LocalY + 1 : Scene.DebugInput.LocalY;
	FLightMapData2D* FinalLightmapData = PaddedLightMapData.ConvertToLightmap2D(bDebugThisMapping, PaddedDebugX, PaddedDebugY);

	// Count the time doing padding and lightmap coloring toward texel setup
	const DOUBLE CurrentTime = appSeconds();
	MappingContext.Stats.TexelRasterizationTime += CurrentTime - PaddingStart;
	const DOUBLE ExecutionTime = CurrentTime - StartTime;

	// Enqueue the static lighting for application in the main thread.
	TList<FTextureMappingStaticLightingData>* StaticLightingLink = new TList<FTextureMappingStaticLightingData>(FTextureMappingStaticLightingData(),NULL);
	StaticLightingLink->Element.Mapping = TextureMapping;
	StaticLightingLink->Element.LightMapData = FinalLightmapData;
	StaticLightingLink->Element.ShadowMaps = ShadowMaps;
	StaticLightingLink->Element.SignedDistanceFieldShadowMaps = SignedDistanceFieldShadowMaps;
	StaticLightingLink->Element.ExecutionTime = ExecutionTime;
	if (MappingContext.NumUniformSamples > 0)
	{
		StaticLightingLink->Element.PreviewEnvironmentShadowing = (BYTE)Clamp<INT>(appTrunc(appSqrt(MappingContext.NumUnshadowedEnvironmentSamples / (FLOAT)MappingContext.NumUniformSamples) * 255.0f), 0, 255);
	}
	else
	{
		StaticLightingLink->Element.PreviewEnvironmentShadowing = 0;
	}
	MappingContext.Stats.TotalTextureMappingLightingThreadTime = ExecutionTime;

	const INT PaddedOffset = TextureMapping->bPadded ? 1 : 0;
	const INT DebugSampleIndex = (Scene.DebugInput.LocalY + PaddedOffset) * TextureMapping->SizeX + Scene.DebugInput.LocalX + PaddedOffset;
#if LM_COMPRESS_ON_EACH_THREAD
	// compress on the thread if desired
	StaticLightingLink->Element.LightMapData->Compress(bDebugThisMapping ? DebugSampleIndex : INDEX_NONE);
#endif

	CompleteTextureMappingList.AddElement(StaticLightingLink);

	const INT OldNumTexelsCompleted = appInterlockedAdd(&NumTexelsCompleted, TextureMapping->CachedSizeX * TextureMapping->CachedSizeY);
	UpdateInternalStatus(OldNumTexelsCompleted, NumVerticesCompleted);
}

class FTexelCornerRasterPolicy
{
public:

	typedef FStaticLightingVertex InterpolantType;

    /** Initialization constructor. */
	FTexelCornerRasterPolicy(
		const FScene& InScene,
		FTexelToCornersMap& InTexelToCornersMap,
		INT InCornerIndex,
		UBOOL bInDebugThisMapping
		):
		Scene(InScene),
		TexelToCornersMap(InTexelToCornersMap),
		CornerIndex(InCornerIndex),
		bDebugThisMapping(bInDebugThisMapping)
	{
	}

protected:

	// FTriangleRasterizer policy interface.

	INT GetMinX() const { return 0; }
	INT GetMaxX() const { return TexelToCornersMap.GetSizeX() - 1; }
	INT GetMinY() const { return 0; }
	INT GetMaxY() const { return TexelToCornersMap.GetSizeY() - 1; }

	void ProcessPixel(INT X, INT Y, const InterpolantType& Interpolant, UBOOL BackFacing);

private:

	const FScene& Scene;
	
	/** The texel to vertex map which is being rasterized to. */
	FTexelToCornersMap& TexelToCornersMap;

	/** Index of the current corner being rasterized */
	const INT CornerIndex;

	const UBOOL bDebugThisMapping;
};

void FTexelCornerRasterPolicy::ProcessPixel(INT X, INT Y, const InterpolantType& Vertex, UBOOL BackFacing)
{
	FTexelToCorners& TexelToCorners = TexelToCornersMap(X, Y);

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisMapping
		&& X == Scene.DebugInput.LocalX
		&& Y == Scene.DebugInput.LocalY)
	{
		INT TempBreak = 0;
	}
#endif

	TexelToCorners.Corners[CornerIndex].WorldPosition = Vertex.WorldPosition;
	TexelToCorners.WorldTangentX = Vertex.WorldTangentX;
	TexelToCorners.WorldTangentY = Vertex.WorldTangentY;
	TexelToCorners.WorldTangentZ = Vertex.WorldTangentZ;
	TexelToCorners.bValid[CornerIndex] = TRUE;
}

/** Calculates TexelToVertexMap and initializes each texel's light sample as mapped or not. */
void FStaticLightingSystem::SetupTextureMapping(
	FStaticLightingTextureMapping* TextureMapping, 
	FGatheredLightMapData2D& LightMapData, 
	FTexelToVertexMap& TexelToVertexMap, 
	FTexelToCornersMap& TexelToCornersMap,
	FStaticLightingMappingContext& MappingContext,
	UBOOL bDebugThisMapping) const
{
	CalculateTexelCorners(TextureMapping->Mesh, TexelToCornersMap, TextureMapping->LightmapTextureCoordinateIndex, bDebugThisMapping);

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisMapping)
	{
		const FTexelToCorners& TexelToCorners = TexelToCornersMap(Scene.DebugInput.LocalX, Scene.DebugInput.LocalY);
		for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
		{
			DebugOutput.TexelCorners[CornerIndex] = TexelToCorners.Corners[CornerIndex].WorldPosition;
			DebugOutput.bCornerValid[CornerIndex] = TexelToCorners.bValid[CornerIndex];
		}
	}
#endif

	// Rasterize the triangles into the texel to vertex map.
	if (GeneralSettings.bUseConservativeTexelRasterization && TextureMapping->bBilinearFilter == TRUE)
	{
		// Using conservative rasterization, which uses super sampling to try to detect all texels that should be mapped.
		for(INT TriangleIndex = 0;TriangleIndex < TextureMapping->Mesh->NumTriangles;TriangleIndex++)
		{
			// Query the mesh for the triangle's vertices.
			FStaticLightingInterpolant V0;
			FStaticLightingInterpolant V1;
			FStaticLightingInterpolant V2;
			INT Element;
			TextureMapping->Mesh->GetTriangle(TriangleIndex,V0.Vertex,V1.Vertex,V2.Vertex,Element);
			V0.ElementIndex = V1.ElementIndex = V2.ElementIndex = Element;

			const FVector2D UV0 = V0.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY);
			const FVector2D UV1 = V1.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY);
			const FVector2D UV2 = V2.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY);

			// Odd number of samples so that the center of the pyramid is on one of the samples
			const UINT NumSamplesX = 7;
			const UINT NumSamplesY = 7;

			// Rasterize multiple sub-texel samples and linearly combine the results
			// Don't rasterize the first or last row and column as the weight will be 0
			for(INT Y = 1; Y < NumSamplesY - 1; Y++)
			{
				const FLOAT SampleYOffset = -Y / (FLOAT)(NumSamplesY - 1);
				for(INT X = 1; X < NumSamplesX - 1; X++)
				{
					const FLOAT SampleXOffset = -X / (FLOAT)(NumSamplesX - 1);
					// Weight the sample based on a pyramid centered on the texel.  
					// The sample with the maximum weight is used, which will be the center if it lies on a triangle.
					const FLOAT SampleWeight = (1 - Abs(1 + SampleXOffset * 2)) * (1 - Abs(1 + SampleYOffset * 2));
					checkSlow(SampleWeight > 0);
					// Rasterize the triangle using the mapping's texture coordinate channel.
					FTriangleRasterizer<FStaticLightingRasterPolicy> TexelMappingRasterizer(FStaticLightingRasterPolicy(
						Scene,
						TexelToVertexMap,
						SampleWeight,
						bDebugThisMapping,
						GeneralSettings.bUseMaxWeight
						));

					TexelMappingRasterizer.DrawTriangle(
						V0,
						V1,
						V2,
						UV0 + FVector2D(SampleXOffset, SampleYOffset),
						UV1 + FVector2D(SampleXOffset, SampleYOffset),
						UV2 + FVector2D(SampleXOffset, SampleYOffset),
						FALSE
						);
				}
			}
		}
	}
	else
	{
		// Only rasterizing one sample at the texel's center.  If the center does not lie on a triangle, the texel will not be mapped.
		const FLOAT SampleWeight = 1.0f;
		// Rasterize the triangles offset by the random sample location.
		for(INT TriangleIndex = 0;TriangleIndex < TextureMapping->Mesh->NumTriangles;TriangleIndex++)
		{
			// Query the mesh for the triangle's vertices.
			FStaticLightingInterpolant V0;
			FStaticLightingInterpolant V1;
			FStaticLightingInterpolant V2;
			INT Element;
			TextureMapping->Mesh->GetTriangle(TriangleIndex,V0.Vertex,V1.Vertex,V2.Vertex,Element);
			V0.ElementIndex = V1.ElementIndex = V2.ElementIndex = Element;

			// Rasterize the triangle using the mapping's texture coordinate channel.
			FTriangleRasterizer<FStaticLightingRasterPolicy> TexelMappingRasterizer(FStaticLightingRasterPolicy(
				Scene,
				TexelToVertexMap,
				SampleWeight,
				bDebugThisMapping,
				FALSE
				));

			// Only rasterize the center of the texel, any texel whose center does not lie on a triangle will not be mapped.
			TexelMappingRasterizer.DrawTriangle(
				V0,
				V1,
				V2,
				V0.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY) + FVector2D(-0.5f,-0.5f),
				V1.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY) + FVector2D(-0.5f,-0.5f),
				V2.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY) + FVector2D(-0.5f,-0.5f),
				FALSE
				);
		}
	}

	// Iterate over each texel and normalize vectors, calculate texel radius
	for(INT Y = 0;Y < TextureMapping->CachedSizeY;Y++)
	{
		for(INT X = 0;X < TextureMapping->CachedSizeX;X++)
		{
			UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisMapping
				&& Y == Scene.DebugInput.LocalY
				&& X == Scene.DebugInput.LocalX)
			{
				bDebugThisTexel = TRUE;
			}
#endif
			FGatheredLightSample& CurrentLightSample = LightMapData(X,Y);

			UBOOL bFoundValidCorner = FALSE;
			const FTexelToCorners& TexelToCorners = TexelToCornersMap(X, Y);
			for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
			{
				bFoundValidCorner = bFoundValidCorner || TexelToCorners.bValid[CornerIndex];
			}

			FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
			if (TexelToVertex.TotalSampleWeight > 0.0f || bFoundValidCorner)
			{
				// Use a corner if none of the other samples were valid
				if (TexelToVertex.TotalSampleWeight < DELTA)
				{
					for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
					{
						if (TexelToCorners.bValid[CornerIndex])
						{
							TexelToVertex.TotalSampleWeight = 1.0f;
							TexelToVertex.WorldPosition = TexelToCorners.Corners[CornerIndex].WorldPosition;
							TexelToVertex.WorldTangentX = TexelToCorners.WorldTangentX;
							TexelToVertex.WorldTangentY = TexelToCorners.WorldTangentY;
							TexelToVertex.WorldTangentZ = TexelToCorners.WorldTangentZ;
							break;
						}
					}
				}
				// Mark the texel as mapped to some geometry in the scene
				CurrentLightSample.bIsMapped = TRUE;
				// Normalize the tangent basis and ensure it is orthonormal
				TexelToVertex.WorldTangentZ = TexelToVertex.WorldTangentZ.UnsafeNormal();

				const FVector4 OriginalTangentX = TexelToVertex.WorldTangentX;
				const FVector4 OriginalTangentY = TexelToVertex.WorldTangentY;
				
				TexelToVertex.WorldTangentY = (TexelToVertex.WorldTangentZ ^ TexelToVertex.WorldTangentX).UnsafeNormal();
				// Maintain handedness
				if ((TexelToVertex.WorldTangentY | OriginalTangentY) < 0)
				{
					TexelToVertex.WorldTangentY *= -1.0f;
				}
				TexelToVertex.WorldTangentX = TexelToVertex.WorldTangentY ^ TexelToVertex.WorldTangentZ;
				if ((TexelToVertex.WorldTangentX | OriginalTangentX) < 0)
				{
					TexelToVertex.WorldTangentX *= -1.0f;
				}

				// ´íÎóÆÁ±Î
				if( !TexelToVertex.WorldTangentX.IsUnit() || !TexelToVertex.WorldTangentY.IsUnit() || !TexelToVertex.WorldTangentY.IsUnit())
				{
					TexelToVertex.WorldTangentX = FVector4( 1.f, 0.f, 0.f, 0.f);
					TexelToVertex.WorldTangentY = FVector4( 0.f, 1.f, 0.f, 0.f);
					TexelToVertex.WorldTangentZ = FVector4( 0.f, 0.f, 1.f, 0.f);
				}


				checkSlow(TexelToVertex.WorldTangentX.IsUnit());
				checkSlow(TexelToVertex.WorldTangentY.IsUnit());
				checkSlow(TexelToVertex.WorldTangentZ.IsUnit());
				checkSlow((TexelToVertex.WorldTangentZ | TexelToVertex.WorldTangentY) < KINDA_SMALL_NUMBER);
				checkSlow((TexelToVertex.WorldTangentX | TexelToVertex.WorldTangentY) < KINDA_SMALL_NUMBER);
				checkSlow((TexelToVertex.WorldTangentX | TexelToVertex.WorldTangentZ) < KINDA_SMALL_NUMBER);

				// Calculate the bounding radius of the texel
				// Use the closest corner as it's likely that's on the same section of a split texel 
				// (A texel shared by multiple UV charts that has sub samples on triangles in different smoothing groups)
				FLOAT MinDistanceSquared = FLT_MAX;
				if (bFoundValidCorner)
				{
					for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
					{
						if (TexelToCorners.bValid[CornerIndex])
						{
							const FLOAT CornerDistSquared = (TexelToCorners.Corners[CornerIndex].WorldPosition - TexelToVertex.WorldPosition).SizeSquared();
							if (CornerDistSquared < MinDistanceSquared)
							{
								MinDistanceSquared = CornerDistSquared;
							}
						}
					}
				}
				else
				{
					MinDistanceSquared = SceneConstants.SmallestTexelRadius;
				}
				TexelToVertex.TexelRadius = Max(appSqrt(MinDistanceSquared), SceneConstants.SmallestTexelRadius);
				MappingContext.Stats.NumMappedTexels++;
			}
			else
			{
				// Mark unmapped texels with the supplied 'UnmappedTexelColor'.
				CurrentLightSample.AddWeighted(FGatheredLightSample::PointLight(Scene.GeneralSettings.UnmappedTexelColor, FVector4(0, 0, 1)), 1.0f);
			}
		}
	}
}

/** Calculates direct lighting as if all lights were non-area lights, then filters the results in texture space to create approximate soft shadows. */
void FStaticLightingSystem::CalculateDirectLightingTextureMappingFiltered(
	FStaticLightingTextureMapping* TextureMapping, 
	FStaticLightingMappingContext& MappingContext,
	FGatheredLightMapData2D& LightMapData, 
	TMap<const FLight*, FShadowMapData2D*>& ShadowMaps,
	const FTexelToVertexMap& TexelToVertexMap, 
	UBOOL bDebugThisMapping,
	const FLight* Light) const
{
	// Raytrace the texels of the shadow-map that map to vertices on a world-space surface.
	FShadowMapData2D ShadowMapData(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY);
	for (INT Y = 0;Y < TextureMapping->CachedSizeY;Y++)
	{
		for (INT X = 0;X < TextureMapping->CachedSizeX;X++)
		{
			UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisMapping
				&& Y == Scene.DebugInput.LocalY
				&& X == Scene.DebugInput.LocalX)
			{
				bDebugThisTexel = TRUE;
			}
#endif
			const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
			if (TexelToVertex.TotalSampleWeight > 0.0f)
			{
				FShadowSample& ShadowSample = ShadowMapData(X,Y);
				ShadowSample.bIsMapped = TRUE;

				// Check if the light is in front of the surface.
				const UBOOL bLightIsInFrontOfTriangle = !IsLightBehindSurface(TexelToVertex.WorldPosition,FVector4(TexelToVertex.WorldTangentZ),Light);
				if (bLightIsInFrontOfTriangle || TextureMapping->Mesh->IsTwoSided(TexelToVertex.ElementIndex))
				{
					// Compute the shadow factors for this sample from the shadow-mapped lights.
					ShadowSample.Visibility = CalculatePointShadowing(TextureMapping,TexelToVertex.WorldPosition,Light,MappingContext,bDebugThisTexel) 
						? 0.0f : 1.0f;
				}
			}
		}
	}

	// Filter the shadow-map, and detect completely occluded lights.
	FShadowMapData2D* FilteredShadowMapData = new FShadowMapData2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY);;
	UBOOL bIsCompletelyOccluded = TRUE;
	for (INT Y = 0;Y < TextureMapping->CachedSizeY;Y++)
	{
		for (INT X = 0;X < TextureMapping->CachedSizeX;X++)
		{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisMapping
				&& Y == Scene.DebugInput.LocalY
				&& X == Scene.DebugInput.LocalX)
			{
				INT TempBreak = 0;
			}
#endif
			if (ShadowMapData(X,Y).bIsMapped)
			{
				UINT Visibility = 0;
				UINT Coverage = 0;
				// The shadow-map filter.
				static const UINT FilterSizeX = 5;
				static const UINT FilterSizeY = 5;
				static const UINT FilterMiddleX = (FilterSizeX - 1) / 2;
				static const UINT FilterMiddleY = (FilterSizeY - 1) / 2;
				static const UINT Filter[5][5] =
				{
					{ 58,  85,  96,  85, 58 },
					{ 85, 123, 140, 123, 85 },
					{ 96, 140, 159, 140, 96 },
					{ 85, 123, 140, 123, 85 },
					{ 58,  85,  96,  85, 58 }
				};
				// Gather the filtered samples for this texel.
				for (UINT FilterY = 0;FilterY < FilterSizeX;FilterY++)
				{
					for (UINT FilterX = 0;FilterX < FilterSizeY;FilterX++)
					{
						INT	SubX = (INT)X - FilterMiddleX + FilterX,
							SubY = (INT)Y - FilterMiddleY + FilterY;
						if (SubX >= 0 && SubX < (INT)TextureMapping->CachedSizeX && SubY >= 0 && SubY < (INT)TextureMapping->CachedSizeY)
						{
							if (ShadowMapData(SubX,SubY).bIsMapped)
							{
								Visibility += appTrunc(Filter[FilterX][FilterY] * ShadowMapData(SubX,SubY).Visibility);
								Coverage += Filter[FilterX][FilterY];
							}
						}
					}
				}

				// Keep track of whether any texels have an unoccluded view of the light.
				if (Visibility > 0)
				{
					bIsCompletelyOccluded = FALSE;
				}

				// Write the filtered shadow-map texel.
				(*FilteredShadowMapData)(X,Y).Visibility = (FLOAT)Visibility / (FLOAT)Coverage;
				(*FilteredShadowMapData)(X,Y).bIsMapped = TRUE;
			}
			else
			{
				(*FilteredShadowMapData)(X,Y).bIsMapped = FALSE;
			}
		}
	}

	if(bIsCompletelyOccluded && !TextureMapping->Mesh->bInstancedStaticMesh)
	{
		// If the light is completely occluded, discard the shadow-map.
		delete FilteredShadowMapData;
		FilteredShadowMapData = NULL;
	}
	else
	{
		// Check whether the light should use a light-map or shadow-map.
		const UBOOL bUseStaticLighting = Light->UseStaticLighting(TextureMapping->bForceDirectLightMap);
		if (bUseStaticLighting)
		{
			// Convert the shadow-map into a light-map.
			for (INT Y = 0;Y < TextureMapping->CachedSizeY;Y++)
			{
				for (INT X = 0;X < TextureMapping->CachedSizeX;X++)
				{
					UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
					if (bDebugThisMapping
						&& Y == Scene.DebugInput.LocalY
						&& X == Scene.DebugInput.LocalX)
					{
						bDebugThisTexel = TRUE;
					}
#endif
					if ((*FilteredShadowMapData)(X,Y).bIsMapped)
					{
						const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
						LightMapData(X,Y).bIsMapped = TRUE;

						// Compute the light sample for this texel based on the corresponding vertex and its shadow factor.
						FLOAT ShadowFactor = (*FilteredShadowMapData)(X,Y).Visibility;
						if (ShadowFactor > 0.0f)
						{
							// Calculate the lighting for the texel.
							check(TexelToVertex.TotalSampleWeight > 0.0f);
							const FStaticLightingVertex CurrentVertex = TexelToVertex.GetVertex();
							const FLinearColor LightIntensity = Light->GetDirectIntensity(CurrentVertex.WorldPosition, FALSE);
							const FGatheredLightSample DirectLighting = CalculatePointLighting(TextureMapping, CurrentVertex, TexelToVertex.ElementIndex, Light, LightIntensity, FLinearColor::White);
							if (GeneralSettings.ViewSingleBounceNumber < 1)
							{
								LightMapData(X,Y).AddWeighted(DirectLighting, ShadowFactor);
							}
						}
					}
				}
			}

			// Add the light to the light-map's light list.
			LightMapData.AddLight(Light);

			// Free the shadow-map.
			delete FilteredShadowMapData;
		}
		// only allow for shadow maps if shadow casting is enabled
		else if ((Light->LightFlags & GI_LIGHT_CASTSHADOWS) && (Light->LightFlags & GI_LIGHT_CASTSTATICSHADOWS))
		{
			ShadowMaps.Set(Light,FilteredShadowMapData);
		}
		else
		{
			delete FilteredShadowMapData;
			FilteredShadowMapData = NULL;
		}
	}
}

/**
 * Calculate lighting from area lights, with filtering in texture space only optionally across severe gradients
 * in the shadow factor. Shadow penumbras will be correctly shaped and will be softer for larger light sources
 * and distant shadow casters.
 */
void FStaticLightingSystem::CalculateDirectAreaLightingTextureMapping(
	FStaticLightingTextureMapping* TextureMapping, 
	FStaticLightingMappingContext& MappingContext,
	FGatheredLightMapData2D& LightMapData, 
	TMap<const FLight*, FShadowMapData2D*>& ShadowMaps,
	const FTexelToVertexMap& TexelToVertexMap, 
	UBOOL bDebugThisMapping,
	const FLight* Light,
	const UBOOL bForceShadowIntoSimpleLightMap ) const
{
	LIGHTINGSTAT(FScopedRDTSCTimer AreaShadowsTimer(MappingContext.Stats.AreaShadowsThreadTime));
	FShadowMapData2D* ShadowMapData = NULL;
	const UBOOL bUseStaticLighting =
		Light->UseStaticLighting(TextureMapping->bForceDirectLightMap) ||
		bForceShadowIntoSimpleLightMap;	// Force static light maps if we're computing for simple light maps only

	UBOOL what = Light->UseStaticLighting( TextureMapping->bForceDirectLightMap);

	UBOOL bIsCompletelyOccluded = TRUE;

	if (!bUseStaticLighting)
	{
		// Only allow for shadow maps if shadow casting is enabled.
		if ( (Light->LightFlags & GI_LIGHT_CASTSHADOWS) && (Light->LightFlags & GI_LIGHT_CASTSTATICSHADOWS) )
		{
			ShadowMapData = new FShadowMapData2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY);
		}
		else
		{
			// Using neither static lighting nor shadow maps; nothing to do.
			return;
		}
	}

	FRandomStream SampleGenerator(0);

	// Used for the optional lightmap gradient filtering pass
	UBOOL bShadowFactorFilterPassEnabled = FALSE;
	FShadowMapData2D UnfilteredShadowFactorData(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);
	FShadowMapData2D FilteredShadowFactorData(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);
	TArray<FLinearColor> TransmissionCache;
	TransmissionCache.Empty(TextureMapping->CachedSizeX * TextureMapping->CachedSizeY);
	TransmissionCache.AddZeroed(TextureMapping->CachedSizeX * TextureMapping->CachedSizeY);
	TArray<FLinearColor> LightIntensityCache;
	LightIntensityCache.Empty(TextureMapping->CachedSizeX * TextureMapping->CachedSizeY);
	LightIntensityCache.AddZeroed(TextureMapping->CachedSizeX * TextureMapping->CachedSizeY);

	for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
	{
		for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
		{
			UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisMapping
				&& Y == Scene.DebugInput.LocalY
				&& X == Scene.DebugInput.LocalX)
			{
				bDebugThisTexel = TRUE;
			}
#endif
			FGatheredLightSample& CurrentLightSample = LightMapData(X,Y);
			if ( ShadowMapData )
			{
				FShadowSample& CurrentShadowSample = (*ShadowMapData)(X,Y);
				CurrentShadowSample.bIsMapped = CurrentLightSample.bIsMapped;
			}

			if ( CurrentLightSample.bIsMapped )
			{
				// Only continue if some part of the light is in front of the surface
				const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);

				const FStaticLightingVertex Vertex = TexelToVertex.GetVertex();

				// @todo: Because we test for rays backfacing the smoothed triangle normal, this code
				// will not skip lighting texels whose tangent space normals are still light-facing,
				// potentially yielding a lighting seam.  We should change this code to only cull
				// rays that are backfacing both the tangent space normal and the smoothed vertex normal
				// by a reasonably small threshold, and then make sure the lighting code handles rays
				// that aren't necessarily in front of the triangle robustly.
				//
				//		const FVector4 Normal = Vertex.TransformTangentVectorToWorld(TextureMapping->Mesh->EvaluateNormal(Vertex.TextureCoordinates[0], TexelToVertex.ElementIndex)) :*/
				const FVector4 Normal = Vertex.WorldTangentZ;

				const UBOOL bLightIsInFrontOfTriangle = !Light->BehindSurface(TexelToVertex.WorldPosition, Normal);
				if (bLightIsInFrontOfTriangle || TextureMapping->Mesh->IsTwoSided(TexelToVertex.ElementIndex))
				{
					const FStaticLightingVertex CurrentVertex = TexelToVertex.GetVertex();
					UBOOL bTraceShadowRays = TRUE;

					const FVector4 LightVector = (Light->Position - TexelToVertex.WorldPosition * Light->Position.W).SafeNormal();
					const FLOAT NormalDotLight = TexelToVertex.WorldTangentZ | LightVector;
					if (PhotonMappingSettings.bUsePhotonMapping 
						&& PhotonMappingSettings.bOptimizeDirectLightingWithPhotons
						// Only apply the direct photon optimization for surfaces whose angle between the normal and the light,
						// Is less than a specified cutoff, since the probability of receiving a direct photon decreases at the angle approaches 90.
						&& NormalDotLight > PhotonMappingSettings.MaxCosDirectLightingPhotonOptimizationAngle)
					{
						// The texel can only be unshadowed if direct photons were found
						//@todo - can also use direct photons to know when texels are fully lit
						//@todo - emitted photons need to take area lights into account to handle large penumbras
						//@todo - need to handle multiple lights, right now we are searching for any direct photon
						bTraceShadowRays = FindAnyNearbyPhoton(
							DirectPhotonMap, 
							CurrentVertex.WorldPosition, 
							PhotonMappingSettings.DirectPhotonSearchDistance, 
							bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == 0);
					}

					// Potentially avoid additional work below if this light has no meaningful contribution
					FLinearColor LightIntensity;
					if (bTraceShadowRays)
					{
						// Compute the incident lighting of the light on the vertex.
						LightIntensity = Light->GetDirectIntensity(CurrentVertex.WorldPosition, FALSE);
						if ((LightIntensity.R <= KINDA_SMALL_NUMBER) &&
							(LightIntensity.G <= KINDA_SMALL_NUMBER) &&
							(LightIntensity.B <= KINDA_SMALL_NUMBER) &&
							(LightIntensity.A <= KINDA_SMALL_NUMBER))
						{
							bTraceShadowRays = FALSE;
						}
					}

					if (bTraceShadowRays)
					{
						// Approximate the integral over the light's surface to calculate incident direct radiance
						// As AverageVisibility * AverageIncidentRadiance
						//@todo - switch to the physically correct formulation which will allow us to handle area lights correctly,
						// Especially area lights with spatially varying emission
						FLOAT ShadowFactor = 0.0f;
						FLinearColor Transmission;
						const TArray<FLightSurfaceSample>& LightSurfaceSamples = Light->GetCachedSurfaceSamples(0, FALSE);
						FLinearColor UnnormalizedTransmission;

						const INT UnShadowedRays = CalculatePointAreaShadowing(
							TextureMapping, 
							CurrentVertex, 
							TexelToVertex.ElementIndex,
							TexelToVertex.TexelRadius,
							Light, 
							MappingContext,
							SampleGenerator,
							UnnormalizedTransmission,
							LightSurfaceSamples,
							bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == 0);

						if (UnShadowedRays > 0)
						{
							if (UnShadowedRays < LightSurfaceSamples.Num())
							{
								// Trace more shadow rays if we are in the penumbra
								const TArray<FLightSurfaceSample>& PenumbraLightSurfaceSamples = Light->GetCachedSurfaceSamples(0, TRUE);
								FLinearColor UnnormalizedPenumbraTransmission;
								const INT UnShadowedPenumbraRays = CalculatePointAreaShadowing(
									TextureMapping, 
									CurrentVertex, 
									TexelToVertex.ElementIndex,
									TexelToVertex.TexelRadius,
									Light, 
									MappingContext,
									SampleGenerator, 
									UnnormalizedPenumbraTransmission,
									PenumbraLightSurfaceSamples,
									bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == 0);

								// Linear combination of uniform and penumbra shadow samples
								//@todo - weight the samples by their solid angle PDF, not uniformly
								ShadowFactor = (UnShadowedRays + UnShadowedPenumbraRays) / (FLOAT)(LightSurfaceSamples.Num() + PenumbraLightSurfaceSamples.Num());
								// Weight each transmission by the fraction of total unshadowed rays that contributed to it
								Transmission = (UnnormalizedTransmission + UnnormalizedPenumbraTransmission) / (UnShadowedRays + UnShadowedPenumbraRays);
							}
							else
							{
								// The texel is completely out of shadow, fully lit, with an explicit shadow factor of 1.0f
								ShadowFactor = 1.0f;
								Transmission = UnnormalizedTransmission / UnShadowedRays;
							}
						}
						else
						{
							Transmission = FLinearColor::Black;
							// The texel is completely in shadow, with an implicit shadow factor of 0.0f
						}

						// Cache off the computed values that we'll use later
						checkSlow(TexelToVertex.TotalSampleWeight > 0.0f);
						TransmissionCache((Y * TextureMapping->CachedSizeX) + X) = Transmission;
						LightIntensityCache((Y * TextureMapping->CachedSizeX) + X) = LightIntensity;
						UnfilteredShadowFactorData(X, Y).Visibility = ShadowFactor;
						UnfilteredShadowFactorData(X, Y).bIsMapped = TRUE;

						// We have valid shadow factor values, enable the filter pass
						bShadowFactorFilterPassEnabled = TRUE;
					}
				}
			}
		}
	}

	// Optional shadow factor filter pass
	if (bShadowFactorFilterPassEnabled && Scene.ShadowSettings.bFilterShadowFactor)
	{
		// Filter in texture space across nearest neighbors
		const FLOAT ThresholdForFilteringPenumbra = Scene.ShadowSettings.ShadowFactorGradientTolerance;
		const INT KernelSizeX = 3; // Expected to be odd
		const INT KernelSizeY = 3; // Expected to be odd
		const FLOAT FilterKernel3x3[KernelSizeX * KernelSizeY] = {
			0.150f, 0.332f, 0.150f,
			0.332f, 1.000f, 0.332f,
			0.150f, 0.332f, 0.150f,
		};
		for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
		{
			for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
			{
				UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugThisMapping
					&& Y == Scene.DebugInput.LocalY
					&& X == Scene.DebugInput.LocalX)
				{
					bDebugThisTexel = TRUE;
				}
#endif
				// If this texel is valid, look for sharp gradients in nearby texels
				if (UnfilteredShadowFactorData(X, Y).bIsMapped)
				{
					FLOAT UnfilteredValue = UnfilteredShadowFactorData(X, Y).Visibility;
					FLOAT FilteredValueNumerator = 0.0f;
					FLOAT FilteredValueDenominator = 0.0f;

					// Compare (up to) the full grid of adjacent texels
					INT X1, Y1;
					INT FilterStepX = ((KernelSizeX - 1) / 2);
					INT FilterStepY = ((KernelSizeY - 1) / 2);

					// Determine if filtering is needed at all
					UBOOL FilteringNeeded = FALSE;
					for (INT KernelIndexY = -FilterStepY; KernelIndexY <= FilterStepY; KernelIndexY++)
					{
						// If this row is out of bounds, skip it
						Y1 = Y + KernelIndexY;
						if ((Y1 < 0) ||
							(Y1 > (TextureMapping->CachedSizeY - 1)))
						{
							continue;
						}

						for (INT KernelIndexX = -FilterStepX; KernelIndexX <= FilterStepX; KernelIndexX++)
						{
							// If this row is out of bounds, skip it
							X1 = X + KernelIndexX;
							if ((X1 < 0) ||
								(X1 > (TextureMapping->CachedSizeX - 1)))
							{
								continue;
							}

							// Only include the texel if it's not completely in shadow
							if (UnfilteredShadowFactorData(X1, Y1).bIsMapped)
							{
								FLOAT ComparisonValue = UnfilteredShadowFactorData(X1, Y1).Visibility;
								FLOAT DifferenceValue = Abs(UnfilteredValue - ComparisonValue);
								if (DifferenceValue > ThresholdForFilteringPenumbra)
								{
									FilteringNeeded = TRUE;
								}

								// Accumulate the to-be-filtered values, in case we need to filter later.
								// This is almost free since we're already doing the work for the lookup.
								INT FilterKernelIndex = ((KernelIndexY + FilterStepY) * KernelSizeX) + (KernelIndexX + FilterStepX);
								FLOAT FilterKernelValue = FilterKernel3x3[FilterKernelIndex];

								FilteredValueNumerator += (ComparisonValue * FilterKernelValue);
								FilteredValueDenominator += FilterKernelValue;
							}
						}
					}

					FLOAT FinalShadowFactorValue;
					if (FilteringNeeded)
					{
						FinalShadowFactorValue = (FilteredValueNumerator / FilteredValueDenominator);
					}
					else
					{
						FinalShadowFactorValue = UnfilteredValue;
					}

					// If the shadow factor value is 0.0f, it will have no effect and thus can be ignored
					FilteredShadowFactorData(X, Y).Visibility = FinalShadowFactorValue;
					FilteredShadowFactorData(X, Y).bIsMapped = FinalShadowFactorValue > DELTA ? TRUE : FALSE;
				}
			}
		}
	}

	INT NumUnoccludedTexels = 0;
	INT NumMappedTexels = 0;
	if (bShadowFactorFilterPassEnabled)
	{
		LIGHTINGSTAT(FScopedRDTSCTimer AreaLightingTimer(MappingContext.Stats.AreaLightingThreadTime));
		for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
		{
			for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
			{
				UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugThisMapping
					&& Y == Scene.DebugInput.LocalY
					&& X == Scene.DebugInput.LocalX)
				{
					bDebugThisTexel = TRUE;
				}
#endif
				FLOAT ShadowFactor;
				UBOOL bIsMapped;
				if (Scene.ShadowSettings.bFilterShadowFactor)
				{
					bIsMapped = FilteredShadowFactorData(X, Y).bIsMapped;
					ShadowFactor = FilteredShadowFactorData(X, Y).Visibility;
				}
				else
				{
					bIsMapped = UnfilteredShadowFactorData(X, Y).bIsMapped;
					ShadowFactor = UnfilteredShadowFactorData(X, Y).Visibility;
				}

				NumMappedTexels += bIsMapped ? 1 : 0;
				if (bIsMapped && ShadowFactor > 0.0f)
				{
					NumUnoccludedTexels++;
					// Get any cached values
					const FLOAT AdjustedShadowFactor = appPow(ShadowFactor, Light->ShadowExponent);
					const FLinearColor Transmission = TransmissionCache((Y * TextureMapping->CachedSizeX) + X);
					const FLinearColor LightIntensity = LightIntensityCache((Y * TextureMapping->CachedSizeX) + X);

					// Calculate any derived values
					const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
					const FStaticLightingVertex CurrentVertex = TexelToVertex.GetVertex();
					const FGatheredLightSample DirectLighting = CalculatePointLighting(TextureMapping, CurrentVertex, TexelToVertex.ElementIndex, Light, LightIntensity, Transmission);

					if (GeneralSettings.ViewSingleBounceNumber < 1)
					{
						if (bUseStaticLighting)
						{
							FGatheredLightSample& CurrentLightSample = LightMapData(X,Y);
							CurrentLightSample.AddWeighted(DirectLighting, AdjustedShadowFactor, bForceShadowIntoSimpleLightMap);
							CurrentLightSample.Coefficients[3][0] = Max(Scene.GeneralSettings.UnmappedTexelColor.R, CurrentLightSample.Coefficients[3][0]);
							CurrentLightSample.Coefficients[3][1] = Max(Scene.GeneralSettings.UnmappedTexelColor.G, CurrentLightSample.Coefficients[3][1]);
							CurrentLightSample.Coefficients[3][2] = Max(Scene.GeneralSettings.UnmappedTexelColor.B, CurrentLightSample.Coefficients[3][2]);
						}
						else
						{
							FShadowSample& CurrentShadowSample = (*ShadowMapData)(X,Y);
							// Using greyscale transmission for shadow maps, since we don't want to increase storage
							CurrentShadowSample.Visibility = AdjustedShadowFactor * Transmission.LinearRGBToXYZ().G;
							if ( CurrentShadowSample.Visibility > 0.0001f )
							{
								bIsCompletelyOccluded = FALSE;
							}
						}
					}
				}
				else
				{
					FGatheredLightSample& CurrentLightSample = LightMapData(X, Y);
					CurrentLightSample.Coefficients[3][0] = Max(Scene.GeneralSettings.UnmappedTexelColor.R, CurrentLightSample.Coefficients[3][0]);
					CurrentLightSample.Coefficients[3][1] = Max(Scene.GeneralSettings.UnmappedTexelColor.G, CurrentLightSample.Coefficients[3][1]);
					CurrentLightSample.Coefficients[3][2] = Max(Scene.GeneralSettings.UnmappedTexelColor.B, CurrentLightSample.Coefficients[3][2]);
				}
			}
		}
	}

	if (bUseStaticLighting)
	{
		// Don't modify light array when only updating simple light maps.  This light has already been added
		// to the shadow map light set, and we never want a light to appear in both lists.  Instead, the 
		// runtime code will check both the light and shadow map lists for lights when running in a mode
		// that does not support shadow maps.
		if( !bForceShadowIntoSimpleLightMap )
		{
			if (Light->GetMeshAreaLight() == NULL)
			{
				LightMapData.AddLight(Light);
			}
		}
	}
	else if (ShadowMapData)
	{
		if ((bIsCompletelyOccluded || NumUnoccludedTexels < NumMappedTexels * ShadowSettings.MinUnoccludedFraction) && !TextureMapping->Mesh->bInstancedStaticMesh)
		{
			delete ShadowMapData;
		}
		else
		{
			ShadowMaps.Set(Light,ShadowMapData);
		}
	}
}

/** 
 * Sample data for the low and high resolution source data that the distance field for shadowing is generated off of.
 * The defaults for all members are implicitly 0 since any uses of this class zero the memory after allocating it.
 */
class FVisibilitySample
{
protected:
	/** World space position in XYZ, Distance to the nearest occluder in W, only valid if !bVisible. */
	FVector4 PositionAndOccluderDistance;
	/** World space normal */
	FLOAT NormalX, NormalY, NormalZ;
	/** Whether this sample is visible to the light. */
	BITFIELD bVisible : 1;
	/** True if this sample maps to a valid point on a surface. */
	BITFIELD bIsMapped : 1;
	/** Whether this sample needs high resolution sampling. */
	BITFIELD bNeedsHighResSampling : 1;

public:
	inline FVector4 GetPosition() const { return FVector4(PositionAndOccluderDistance, 0.0f); }
	inline FLOAT GetOccluderDistance() const { return PositionAndOccluderDistance.W; }
	inline FVector4 GetNormal() const { return FVector4(NormalX, NormalY, NormalZ); }
	inline UBOOL IsVisible() const { return bVisible; }
	inline UBOOL IsMapped() const { return bIsMapped; }
	inline UBOOL NeedsHighResSampling() const { return bNeedsHighResSampling; }

	inline void SetPosition(const FVector4& InPosition)  
	{  
		PositionAndOccluderDistance.X = InPosition.X;
		PositionAndOccluderDistance.Y = InPosition.Y;
		PositionAndOccluderDistance.Z = InPosition.Z;
	}
	inline void SetOccluderDistance(FLOAT InOccluderDistance)  
	{  
		PositionAndOccluderDistance.W = InOccluderDistance;
	}
	inline void SetNormal(const FVector4& InNormal)  
	{  
		NormalX = InNormal.X;
		NormalY = InNormal.Y;
		NormalZ = InNormal.Z;
	}
	inline void SetVisible(UBOOL bInVisible) { bVisible = bInVisible; }
	inline void SetMapped(UBOOL bInMapped) { bIsMapped = bInMapped; }
};

/** 
 * Sample data for the low resolution visibility data that is populated initially for distance field generation.
 * Each low resolution sample contains a set of high resolution samples if the low resolution sample is next to a shadow transition. 
 */
class FLowResolutionVisibilitySample : public FVisibilitySample
{
public:
	WORD ElementIndex;

	/** High resolution samples corresponding to this low resolution sample, only allocated if bNeedsHighResSampling == TRUE. */
	TArray<FVisibilitySample> HighResolutionSamples;

	inline void SetNeedsHighResSampling(UBOOL bInNeedsHighResSampling, INT UpsampleFactor) 
	{ 
		if (bInNeedsHighResSampling)
		{
			HighResolutionSamples.Empty(UpsampleFactor * UpsampleFactor);
			HighResolutionSamples.AddZeroed(UpsampleFactor * UpsampleFactor);
		}
		bNeedsHighResSampling = bInNeedsHighResSampling; 
	}
};

/** 2D array of FLowResolutionVisibilitySample's */
class FTexelVisibilityData2D : public FShadowMapData2DData
{
public:
	FTexelVisibilityData2D(UINT InSizeX,UINT InSizeY) :
		FShadowMapData2DData(InSizeX, InSizeY)
	{
		Data.Empty(InSizeX * InSizeY);
		Data.AddZeroed(InSizeX * InSizeY);
	}

	// Accessors.
	const FLowResolutionVisibilitySample& operator()(UINT X,UINT Y) const { return Data(SizeX * Y + X); }
	FLowResolutionVisibilitySample& operator()(UINT X,UINT Y) { return Data(SizeX * Y + X); }
	UINT GetSizeX() const { return SizeX; }
	UINT GetSizeY() const { return SizeY; }
	void Empty() { Data.Empty(); }
	SIZE_T GetAllocatedSize() const { return Data.GetAllocatedSize(); }

private:
	TArray<FLowResolutionVisibilitySample> Data;
};

class FDistanceFieldRasterPolicy
{
public:

	typedef FStaticLightingInterpolant InterpolantType;

    /** Initialization constructor. */
	FDistanceFieldRasterPolicy(FTexelVisibilityData2D& InLowResolutionVisibilityData, INT InUpsampleFactor, INT InSizeX, INT InSizeY) :
		LowResolutionVisibilityData(InLowResolutionVisibilityData),
		UpsampleFactor(InUpsampleFactor),
		SizeX(InSizeX),
		SizeY(InSizeY)
	{}

protected:

	// FTriangleRasterizer policy interface.

	INT GetMinX() const { return 0; }
	INT GetMaxX() const { return SizeX - 1; }
	INT GetMinY() const { return 0; }
	INT GetMaxY() const { return SizeY - 1; }

	void ProcessPixel(INT X,INT Y,const InterpolantType& Interpolant,UBOOL BackFacing);

private:

	FTexelVisibilityData2D& LowResolutionVisibilityData;
	const INT UpsampleFactor;
	const INT SizeX;
	const INT SizeY;
};

void FDistanceFieldRasterPolicy::ProcessPixel(INT X,INT Y,const InterpolantType& Interpolant,UBOOL BackFacing)
{
	FLowResolutionVisibilitySample& LowResSample = LowResolutionVisibilityData(X / UpsampleFactor, Y / UpsampleFactor);
	LowResSample.ElementIndex = Interpolant.ElementIndex;
	if (LowResSample.NeedsHighResSampling())
	{
		FVisibilitySample& Sample = LowResSample.HighResolutionSamples(Y % UpsampleFactor * UpsampleFactor + X % UpsampleFactor);
		Sample.SetPosition(Interpolant.Vertex.WorldPosition);
		Sample.SetNormal(Interpolant.Vertex.WorldTangentZ);
		Sample.SetMapped(TRUE);
	}
}

/** 
 * Calculate signed distance field shadowing from a single light,  
 * Based on the paper "Improved Alpha-Tested Magnification for Vector Textures and Special Effects" by Valve.
 */
void FStaticLightingSystem::CalculateDirectSignedDistanceFieldLightingTextureMapping(
	FStaticLightingTextureMapping* TextureMapping, 
	FStaticLightingMappingContext& MappingContext,
	FGatheredLightMapData2D& LightMapData, 
	TMap<const FLight*, FSignedDistanceFieldShadowMapData2D*>& ShadowMaps,
	const FTexelToVertexMap& TexelToVertexMap, 
	const FTexelToCornersMap& TexelToCornersMap,
	UBOOL bDebugThisMapping,
	const FLight* Light) const
{
	LIGHTINGSTAT(FManualRDTSCTimer FirstPassSourceTimer(MappingContext.Stats.SignedDistanceFieldSourceFirstPassThreadTime));
	TArray<FStaticLightingInterpolant> MeshVertices;
	MeshVertices.Empty(TextureMapping->Mesh->NumTriangles * 3);
	MeshVertices.AddZeroed(TextureMapping->Mesh->NumTriangles * 3);
	FLOAT AverageTexelDensity = 0.0f;
	for (INT TriangleIndex = 0; TriangleIndex < TextureMapping->Mesh->NumTriangles; TriangleIndex++)
	{
		// Query the mesh for the triangle's vertices.
		INT Element;
		TextureMapping->Mesh->GetTriangle(TriangleIndex, MeshVertices(TriangleIndex * 3).Vertex, MeshVertices(TriangleIndex * 3 + 1).Vertex, MeshVertices(TriangleIndex * 3 + 2).Vertex, Element);
		MeshVertices(TriangleIndex * 3).ElementIndex = MeshVertices(TriangleIndex * 3 + 1).ElementIndex = MeshVertices(TriangleIndex * 3 + 2).ElementIndex = Element;

		const FVector4 TriangleNormal = (MeshVertices(TriangleIndex * 3 + 2).Vertex.WorldPosition - MeshVertices(TriangleIndex * 3).Vertex.WorldPosition) ^ (MeshVertices(TriangleIndex * 3 + 1).Vertex.WorldPosition - MeshVertices(TriangleIndex).Vertex.WorldPosition);
		const FLOAT TriangleArea = 0.5f * TriangleNormal.Size();

		if (TriangleArea > DELTA)
		{
			// Triangle vertices in lightmap UV space, scaled by the lightmap resolution
			const FVector2D Vertex0 = MeshVertices(TriangleIndex * 3 + 0).Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);
			const FVector2D Vertex1 = MeshVertices(TriangleIndex * 3 + 1).Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);
			const FVector2D Vertex2 = MeshVertices(TriangleIndex * 3 + 2).Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);

			// Area in lightmap space, or the number of lightmap texels covered by this triangle
			const FLOAT LightmapTriangleArea = Abs(
				Vertex0.X * (Vertex1.Y - Vertex2.Y)
				+ Vertex1.X * (Vertex2.Y - Vertex0.Y)
				+ Vertex2.X * (Vertex0.Y - Vertex1.Y));

			// Accumulate the texel density
			AverageTexelDensity += LightmapTriangleArea / TriangleArea;
		}
	}

	INT UpsampleFactor = 1;
	if (AverageTexelDensity > DELTA)
	{
		// Normalize the average
		AverageTexelDensity /= TextureMapping->Mesh->NumTriangles;
		// Calculate the length of one side of a right isosceles triangle with texel density equal to the mesh's average texel density
		const FLOAT RightTriangleSide = appSqrt(2.0f * AverageTexelDensity);
		// Choose an upsample factor based on the average texels/world space ratio
		// The result is that small, high resolution meshes will not upsample as much, since they don't need it, 
		// But large, low resolution meshes will upsample a lot.
		const INT TargetUpsampleFactor = appTrunc(ShadowSettings.ApproximateHighResTexelsPerMaxTransitionDistance / (RightTriangleSide * ShadowSettings.MaxTransitionDistanceWorldSpace));
		// Round up to the nearest odd factor, so each destination texel has a high resolution source texel at its center
		// Clamp the upscale factor to be less than 13, since the quality improvements of upsampling higher than that are negligible.
		UpsampleFactor = Clamp(TargetUpsampleFactor - TargetUpsampleFactor % 2 + 1, ShadowSettings.MinDistanceFieldUpsampleFactor, 13);
	}
	MappingContext.Stats.AccumulatedSignedDistanceFieldUpsampleFactors += UpsampleFactor;
	MappingContext.Stats.NumSignedDistanceFieldCalculations++;

	UBOOL bIsCompletelyOccluded = TRUE;
	INT NumUnoccludedTexels = 0;
	INT NumMappedTexels = 0;
	// Calculate visibility at the resolution of the final distance field in a first pass
	FTexelVisibilityData2D LowResolutionVisibilityData(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);
	for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
	{
		for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
		{
			UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisMapping
				&& Y == Scene.DebugInput.LocalY
				&& X == Scene.DebugInput.LocalX)
			{
				bDebugThisTexel = TRUE;
			}
#endif
			const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
			if (TexelToVertex.TotalSampleWeight > 0.0f)
			{
				NumMappedTexels++;
				FLowResolutionVisibilitySample& CurrentSample = LowResolutionVisibilityData(X, Y);
				CurrentSample.SetMapped(TRUE);
				CurrentSample.SetPosition(TexelToVertex.WorldPosition);
				CurrentSample.SetNormal(TexelToVertex.WorldTangentZ);

				// Note: not checking for backfacing normals because some of the high resolution samples corresponding to this texel may be frontfacing
				if (Light->AffectsBounds(FBoxSphereBounds(TexelToVertex.WorldPosition, FVector4(0,0,0),0)))
				{
					const FVector4 LightPosition = Light->LightCenterPosition(TexelToVertex.WorldPosition);
					const FVector4 LightVector = (LightPosition - TexelToVertex.WorldPosition).SafeNormal();

					FVector4 NormalForOffset = CurrentSample.GetNormal();
					// Flip the normal used for offsetting the start of the ray for two sided materials if a flipped normal would be closer to the light.
					// This prevents incorrect shadowing where using the frontface normal would cause the ray to start inside a nearby object.
					const UBOOL bIsTwoSided = TextureMapping->Mesh->IsTwoSided(CurrentSample.ElementIndex);
					if (bIsTwoSided && (-NormalForOffset | LightVector) > (NormalForOffset | LightVector))
					{
						NormalForOffset = -NormalForOffset;
					}

					const FLightRay LightRay(
						// Offset the start of the ray by some fraction along the direction of the ray and some fraction along the vertex normal.
						TexelToVertex.WorldPosition 
						+ LightVector * SceneConstants.VisibilityRayOffsetDistance 
						+ NormalForOffset * SceneConstants.VisibilityNormalOffsetDistance,
						LightPosition,
						TextureMapping,
						Light
						);

					FLightRayIntersection Intersection;
					MappingContext.Stats.NumSignedDistanceFieldAdaptiveSourceRaysFirstPass++;
					// Could trace a boolean visibility ray, no other information is needed,
					// However FStaticLightingAggregateMesh::IntersectLightRay currently does not handle masked materials correctly with boolean visibility rays.
					AggregateMesh.IntersectLightRay(LightRay, TRUE, FALSE, TRUE, MappingContext.RayCache, Intersection);
					if (!Intersection.bIntersects)
					{
						NumUnoccludedTexels++;
						bIsCompletelyOccluded = FALSE;
						CurrentSample.SetVisible(TRUE);
					}

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
					if (bDebugThisTexel)
					{
						FDebugStaticLightingRay DebugRay(LightRay.Start, LightRay.End, Intersection.bIntersects);
						if (Intersection.bIntersects)
						{
							DebugRay.End = Intersection.IntersectionVertex.WorldPosition;
						}
						DebugOutput.ShadowRays.AddItem(DebugRay);
					}
#endif
				}
			}
		}
	}
	FirstPassSourceTimer.Stop();

	if ( (!bIsCompletelyOccluded && NumUnoccludedTexels > NumMappedTexels * ShadowSettings.MinUnoccludedFraction) || TextureMapping->Mesh->bInstancedStaticMesh)
	{
		LIGHTINGSTAT(FManualRDTSCTimer SecondPassSourceTimer(MappingContext.Stats.SignedDistanceFieldSourceSecondPassThreadTime));
		check(UpsampleFactor % 2 == 1 && UpsampleFactor >= 1);
		const INT HighResolutionSignalSizeX = TextureMapping->CachedSizeX * UpsampleFactor;
		const INT HighResolutionSignalSizeY = TextureMapping->CachedSizeY * UpsampleFactor;
		// Allocate the final distance field shadow map on the heap, since it will be passed out of this function
		FSignedDistanceFieldShadowMapData2D* ShadowMapData = new FSignedDistanceFieldShadowMapData2D(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);

		// Neighbor texel coordinates - the order in which these are stored matters later
		const FIntPoint Neighbors[] = 
		{
			FIntPoint(0, 1),
			FIntPoint(0, -1),
			FIntPoint(1, 0),
			FIntPoint(-1, 0)
		};

		// Offsets to the high resolution samples corresponding to the corners of a low resolution sample
		const FIntPoint Corners[] = 
		{
			FIntPoint(0, 0),
			FIntPoint(0, UpsampleFactor - 1),
			FIntPoint(UpsampleFactor - 1, 0),
			FIntPoint(UpsampleFactor - 1, UpsampleFactor - 1)
		};

		// Traverse the visibility data collected at the resolution of the final distance field, detecting where additional sampling is required.
		for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
		{
			for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
			{
				UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugThisMapping
					&& Y == Scene.DebugInput.LocalY
					&& X == Scene.DebugInput.LocalX)
				{
					bDebugThisTexel = TRUE;
				}
#endif
				FLowResolutionVisibilitySample& CurrentSample = LowResolutionVisibilityData(X, Y);
				if (CurrentSample.IsMapped())
				{
					FSignedDistanceFieldShadowSample& FinalShadowSample = (*ShadowMapData)(X, Y);
					FinalShadowSample.bIsMapped = TRUE;
					if (CurrentSample.IsVisible())
					{
						// Initialize the final distance field data, since it will only be written to after this if it gets scattered to during the search.
						FinalShadowSample.Distance = 1.0f;
					}

					// Search for a neighbor with different visibility
					UBOOL bNeighborsDifferent = FALSE;
					for (INT i = 0 ; i < ARRAY_COUNT(Neighbors); i++)
					{
						if (X + Neighbors[i].X > 0
							&& X + Neighbors[i].X < TextureMapping->CachedSizeX
							&& Y + Neighbors[i].Y > 0
							&& Y + Neighbors[i].Y < TextureMapping->CachedSizeY)
						{
							const FLowResolutionVisibilitySample& NeighborSample = LowResolutionVisibilityData(X + Neighbors[i].X, Y + Neighbors[i].Y);
							if (CurrentSample.IsVisible() != NeighborSample.IsVisible() && NeighborSample.IsMapped())
							{
								bNeighborsDifferent = TRUE;
								break;
							}
						}
					}

					// Mark the low resolution sample as needing high resolution sampling, since it is next to a shadow transition
					if (bNeighborsDifferent)
					{
						CurrentSample.SetNeedsHighResSampling(bNeighborsDifferent, UpsampleFactor);
					}
				}
			}
		}

		FDistanceFieldRasterPolicy RasterPolicy(LowResolutionVisibilityData, UpsampleFactor, HighResolutionSignalSizeX, HighResolutionSignalSizeY);
		FTriangleRasterizer<FDistanceFieldRasterPolicy> DistanceFieldRasterizer(RasterPolicy);
		// Rasterize the mesh at the upsampled source data resolution
		for (INT TriangleIndex = 0; TriangleIndex < MeshVertices.Num() / 3; TriangleIndex++)
		{
			const FStaticLightingInterpolant& V0 = MeshVertices(TriangleIndex * 3);
			const FStaticLightingInterpolant& V1 = MeshVertices(TriangleIndex * 3 + 1);
			const FStaticLightingInterpolant& V2 = MeshVertices(TriangleIndex * 3 + 2);

			DistanceFieldRasterizer.DrawTriangle(
				V0,
				V1,
				V2,
				V0.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(HighResolutionSignalSizeX, HighResolutionSignalSizeY) + FVector2D(-0.5f,-0.5f),
				V1.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(HighResolutionSignalSizeX, HighResolutionSignalSizeY) + FVector2D(-0.5f,-0.5f),
				V2.Vertex.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex] * FVector2D(HighResolutionSignalSizeX, HighResolutionSignalSizeY) + FVector2D(-0.5f,-0.5f),
				FALSE
				);
		}
		MeshVertices.Empty();

		// Check for edge cases where the low resolution sample is mapped, but none of the high resolution samples got mapped.
		for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
		{
			for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
			{
				UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugThisMapping
					&& Y == Scene.DebugInput.LocalY
					&& X == Scene.DebugInput.LocalX)
				{
					bDebugThisTexel = TRUE;
				}
#endif
				FLowResolutionVisibilitySample& CurrentSample = LowResolutionVisibilityData(X, Y);
				if (CurrentSample.IsMapped() && CurrentSample.NeedsHighResSampling())
				{
					UBOOL bAnyHighResSamplesMapped = FALSE;
					// Iterate over all the upsampled source data texels corresponding to this texel
					for (INT HighResY = 0; HighResY < UpsampleFactor; HighResY++)
					{
						for (INT HighResX = 0; HighResX < UpsampleFactor; HighResX++)
						{
							FVisibilitySample& CurrentHighResSample = CurrentSample.HighResolutionSamples(HighResY * UpsampleFactor + HighResX);
							if (CurrentHighResSample.IsMapped())
							{
								bAnyHighResSamplesMapped = TRUE;
							}
						}
					}

					// If none of the high res samples are mapped, but the low resolution sample is mapped, 
					// Propagate the low resolution corner information to the corresponding high resolution samples.
					// This handles texels along UV seams where only the corner of the texel is mapped.
					if (!bAnyHighResSamplesMapped)
					{
						const FTexelToCorners& TexelToCorners = TexelToCornersMap(X, Y);
						for (INT CornerIndex = 0; CornerIndex < ARRAY_COUNT(Corners); CornerIndex++)
						{						
							if (TexelToCorners.bValid[CornerIndex])
							{
								FVisibilitySample& CornerHighResSample = CurrentSample.HighResolutionSamples(Corners[CornerIndex].Y * UpsampleFactor + Corners[CornerIndex].X);
								CornerHighResSample.SetMapped(TRUE);
								CornerHighResSample.SetPosition(TexelToCorners.Corners[CornerIndex].WorldPosition);
								CornerHighResSample.SetNormal(TexelToCorners.WorldTangentZ);
							}
						}
					}
				}
			}
		}

		for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
		{
			for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
			{
				UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugThisMapping
					&& Y == Scene.DebugInput.LocalY
					&& X == Scene.DebugInput.LocalX)
				{
					bDebugThisTexel = TRUE;
				}
#endif
				FLowResolutionVisibilitySample& CurrentSample = LowResolutionVisibilityData(X, Y);
				// Do high resolution sampling if necessary
				if (CurrentSample.IsMapped() && CurrentSample.NeedsHighResSampling())
				{
					const UBOOL bIsTwoSided = TextureMapping->Mesh->IsTwoSided(CurrentSample.ElementIndex);
					for (INT HighResY = 0; HighResY < UpsampleFactor; HighResY++)
					{
						for (INT HighResX = 0; HighResX < UpsampleFactor; HighResX++)
						{
							FVisibilitySample& HighResSample = CurrentSample.HighResolutionSamples(HighResY * UpsampleFactor + HighResX);
							const UBOOL bLightIsInFrontOfTriangle = !IsLightBehindSurface(HighResSample.GetPosition(),HighResSample.GetNormal(),Light);

							if ((bLightIsInFrontOfTriangle || bIsTwoSided) 
								&& Light->AffectsBounds(FBoxSphereBounds(HighResSample.GetPosition(), FVector4(0,0,0),0)))
							{
								const FVector4 LightPosition = Light->LightCenterPosition(HighResSample.GetPosition());
								const FVector4 LightVector = (LightPosition - HighResSample.GetPosition()).SafeNormal();

								FVector4 NormalForOffset = HighResSample.GetNormal();
								// Flip the normal used for offsetting the start of the ray for two sided materials if a flipped normal would be closer to the light.
								// This prevents incorrect shadowing where using the frontface normal would cause the ray to start inside a nearby object.
								if (bIsTwoSided && (-NormalForOffset | LightVector) > (NormalForOffset | LightVector))
								{
									NormalForOffset = -NormalForOffset;
								}
								const FLightRay LightRay(
									// Offset the start of the ray by some fraction along the direction of the ray and some fraction along the vertex normal.
									HighResSample.GetPosition() 
									+ LightVector * SceneConstants.VisibilityRayOffsetDistance 
									+ NormalForOffset * SceneConstants.VisibilityNormalOffsetDistance,
									LightPosition,
									TextureMapping, 
									Light
									);

								FLightRayIntersection Intersection;
								MappingContext.Stats.NumSignedDistanceFieldAdaptiveSourceRaysSecondPass++;
								// Have to calculate the closest intersection so we know the distance to the nearest occluder
								//@todo - for the occluder distance to be correct, the ray should actually go from the light to the receiver
								AggregateMesh.IntersectLightRay(LightRay, TRUE, FALSE, TRUE, MappingContext.RayCache, Intersection);
								if (Intersection.bIntersects)
								{
									HighResSample.SetOccluderDistance((LightRay.Start - Intersection.IntersectionVertex.WorldPosition).Size());
								}
								else
								{
									HighResSample.SetVisible(TRUE);
								}
							}
						}
					}
				}
			}
		}
		SecondPassSourceTimer.Stop();

		INT NumScattersToSelectedTexel = 0;
		LIGHTINGSTAT(FScopedRDTSCTimer SearchTimer(MappingContext.Stats.SignedDistanceFieldSearchThreadTime));
		// Traverse the high resolution source data by going over low res samples that that need high resolution sampling, and at each texel that is next to a transition,
		// Scatter the distance to that texel onto all low resolution distance field texels within a certain world space distance from the transition texel.
		// The end result is that each low resolution texel in the distance field has the world space distance to the nearest transition in the high resolution visibility data.
		// Using a scatter from the high res transition texels is significantly faster than a brute force gather from the low resolution distance field texels, 
		// Because only a small set of the high resolution texels are next to the shadow transition.
		for (INT LowResY = 0; LowResY < TextureMapping->CachedSizeY; LowResY++)
		{
			for (INT LowResX = 0; LowResX < TextureMapping->CachedSizeX; LowResX++)
			{
				UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugThisMapping
					&& LowResY == Scene.DebugInput.LocalY
					&& LowResX == Scene.DebugInput.LocalX)
				{
					bDebugThisTexel = TRUE;
				}
#endif
				FLowResolutionVisibilitySample& CurrentLowResSample = LowResolutionVisibilityData(LowResX, LowResY);
				if (CurrentLowResSample.IsMapped() && CurrentLowResSample.NeedsHighResSampling())
				{
					for (INT HighResY = 0; HighResY < UpsampleFactor; HighResY++)
					{
						for (INT HighResX = 0; HighResX < UpsampleFactor; HighResX++)
						{
							FVisibilitySample& HighResSample = CurrentLowResSample.HighResolutionSamples(HighResY * UpsampleFactor + HighResX);
							// Only texels that needed high resolution sampling can be next to the shadow transition
							// Only operate on shadowed texels, since they know the distance to the nearest occluder, which is necessary for calculating penumbra size
							// As a result, the reconstructed shadow transition will be slightly offset
							if (HighResSample.IsMapped() && !HighResSample.IsVisible())
							{
								// Detect texels next to the shadow transition
								UBOOL bNeighborsDifferent = FALSE;
								for (INT i = 0 ; i < ARRAY_COUNT(Neighbors); i++)
								{
									// Calculate the high resolution indices, which may go into neighboring low resolution samples
									const INT HighResNeighborX = LowResX * UpsampleFactor + HighResX + Neighbors[i].X;
									const INT HighResNeighborY = LowResY * UpsampleFactor + HighResY + Neighbors[i].Y;
									const INT LowResNeighborX = HighResNeighborX / UpsampleFactor;
									const INT LowResNeighborY = HighResNeighborY / UpsampleFactor;
									if (LowResNeighborX > 0
										&& LowResNeighborX < TextureMapping->CachedSizeX
										&& LowResNeighborY > 0
										&& LowResNeighborY < TextureMapping->CachedSizeY)
									{
										const FLowResolutionVisibilitySample& LowResNeighborSample = LowResolutionVisibilityData(LowResNeighborX, LowResNeighborY);
										// If the low res neighbor sample has high resolution samples, check the neighboring high resolution sample's visibility
										if (LowResNeighborSample.NeedsHighResSampling())
										{
											const FVisibilitySample& HighResNeighborSample = LowResNeighborSample.HighResolutionSamples((HighResNeighborY % UpsampleFactor) * UpsampleFactor + HighResNeighborX % UpsampleFactor);
											if (HighResNeighborSample.IsMapped() && HighResNeighborSample.IsVisible())
											{
												bNeighborsDifferent = TRUE;
												break;
											}
										}
										else
										{
											// The low res neighbor sample didn't have high resolution samples, use its visibility
											if (LowResNeighborSample.IsMapped() && LowResNeighborSample.IsVisible())
											{
												bNeighborsDifferent = TRUE;
												break;
											}
										}
									}
								}

								if (bNeighborsDifferent)
								{
									FLOAT WorldSpacePerHighResTexelX = FLT_MAX;
									FLOAT WorldSpacePerHighResTexelY = FLT_MAX;
									// Determine how far to scatter transition distance by measuring the world space distance between this texel and its neighbors
									for (INT i = 0 ; i < ARRAY_COUNT(Neighbors); i++)
									{
										if (HighResX + Neighbors[i].X > 0
											&& HighResX + Neighbors[i].X < UpsampleFactor
											&& HighResY + Neighbors[i].Y > 0
											&& HighResY + Neighbors[i].Y < UpsampleFactor)
										{
											const FVisibilitySample& NeighborSample = CurrentLowResSample.HighResolutionSamples((HighResY + Neighbors[i].Y) * UpsampleFactor + HighResX + Neighbors[i].X);
											if (NeighborSample.IsMapped())
											{
												// Last two neighbor offsets are in X
												if (i >= 2)
												{
													WorldSpacePerHighResTexelX = Min(WorldSpacePerHighResTexelX, (NeighborSample.GetPosition() - HighResSample.GetPosition()).Size());
												}
												else
												{
													WorldSpacePerHighResTexelY = Min(WorldSpacePerHighResTexelY, (NeighborSample.GetPosition() - HighResSample.GetPosition()).Size());
												}
											}
										}
									}

									if (WorldSpacePerHighResTexelX == FLT_MAX && WorldSpacePerHighResTexelY == FLT_MAX)
									{
										WorldSpacePerHighResTexelX = 1.0f;
										WorldSpacePerHighResTexelY = 1.0f;
									}
									else if (WorldSpacePerHighResTexelX == FLT_MAX)
									{
										WorldSpacePerHighResTexelX = WorldSpacePerHighResTexelY;
									}
									else if (WorldSpacePerHighResTexelY == FLT_MAX)
									{
										WorldSpacePerHighResTexelY = WorldSpacePerHighResTexelX;
									}

									// Scatter to all distance field texels within MaxTransitionDistanceWorldSpace, rounded up.
									// This is an approximation to the actual set of distance field texels that are within MaxTransitionDistanceWorldSpace that tends to work out well.
									// Apply a clamp to avoid a performance cliff with some texels, whose adjacent texel in lightmap space is actually far away in world space
									const INT NumLowResScatterTexelsY = Min(appTrunc(ShadowSettings.MaxTransitionDistanceWorldSpace / (WorldSpacePerHighResTexelY * UpsampleFactor)) + 1, 100);
									const INT NumLowResScatterTexelsX = Min(appTrunc(ShadowSettings.MaxTransitionDistanceWorldSpace / (WorldSpacePerHighResTexelX * UpsampleFactor)) + 1, 100);
									MappingContext.Stats.NumSignedDistanceFieldScatters++;
									for (INT ScatterOffsetY = -NumLowResScatterTexelsY; ScatterOffsetY <= NumLowResScatterTexelsY; ScatterOffsetY++)
									{
										const INT LowResScatterY = LowResY + ScatterOffsetY;
										if (LowResScatterY < 0 || LowResScatterY >= TextureMapping->CachedSizeY)
										{
											continue;
										}
										for (INT ScatterOffsetX = -NumLowResScatterTexelsX; ScatterOffsetX <= NumLowResScatterTexelsX; ScatterOffsetX++)
										{
											const INT LowResScatterX = LowResX + ScatterOffsetX;
											if (LowResScatterX < 0 || LowResScatterX >= TextureMapping->CachedSizeX)
											{
												continue;
											}

											UBOOL bDebugThisScatterTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING 
											// Debug when the selected texel is being scattered to
											// This may get hit any number of times, only the closest transition distance will be kept in the end
											if (bDebugThisMapping
												&& LowResScatterY == Scene.DebugInput.LocalY
												&& LowResScatterX == Scene.DebugInput.LocalX)
											{
												bDebugThisScatterTexel = TRUE;
											}
#endif
											const FLowResolutionVisibilitySample& LowResScatterSample = LowResolutionVisibilityData(LowResScatterX, LowResScatterY);
											// Only scatter transition distance to mapped texels
											if (LowResScatterSample.IsMapped())
											{
												UBOOL CurrentRegion = FALSE;
												FVector4 ScatterPosition;
												UBOOL bFoundScatterPosition = FALSE;
												
												if (LowResScatterSample.NeedsHighResSampling())
												{
													// If the low res scatter sample has high resolution samples, use the center high resolution sample's visibility
													const FVisibilitySample& HighResScatterSample = LowResScatterSample.HighResolutionSamples((UpsampleFactor / 2) * UpsampleFactor + UpsampleFactor / 2);
													if (HighResScatterSample.IsMapped())
													{
														CurrentRegion = HighResScatterSample.IsVisible();
														ScatterPosition = HighResScatterSample.GetPosition();
														bFoundScatterPosition = TRUE;
													}
													else
													{
														// If the centered high resolution texel is not mapped, 
														// Search all of the high resolution texels corresponding to the low resolution distance field texel for the closest mapped texel.
														FLOAT ClosestMappedSubSampleDistSquared = FLT_MAX;
														for (INT SubY = 0; SubY < UpsampleFactor; SubY++)
														{
															for (INT SubX = 0; SubX < UpsampleFactor; SubX++)
															{
																const FVisibilitySample& SubHighResSample = LowResScatterSample.HighResolutionSamples(SubY * UpsampleFactor + SubX);
																const FLOAT SubSampleDistanceSquared = Square(SubX - UpsampleFactor / 2) + Square(SubY - UpsampleFactor / 2);
																if (SubHighResSample.IsMapped() && SubSampleDistanceSquared < ClosestMappedSubSampleDistSquared)
																{
																	ClosestMappedSubSampleDistSquared = SubSampleDistanceSquared;
																	CurrentRegion = SubHighResSample.IsVisible();
																	ScatterPosition = SubHighResSample.GetPosition();
																	bFoundScatterPosition = TRUE;
																}
															}
														}
													}
												}

												// No high resolution scatter samples were found, use the position and visibility of the low resolution sample
												if (!bFoundScatterPosition) 
												{
													CurrentRegion = LowResScatterSample.IsVisible();
													ScatterPosition = LowResScatterSample.GetPosition();
												}

												// World space distance from the distance field texel to the nearest shadow transition
												const FLOAT TransitionDistance = (ScatterPosition - HighResSample.GetPosition()).Size();
												const FLOAT NormalizedDistance = Clamp(TransitionDistance / ShadowSettings.MaxTransitionDistanceWorldSpace, 0.0f, 1.0f);
												FSignedDistanceFieldShadowSample& FinalShadowSample = (*ShadowMapData)(LowResScatterX, LowResScatterY);
												// If LowResScatterSample.IsMapped() is true, the distance field texel must be mapped.
												checkSlow(FinalShadowSample.bIsMapped);
												// Only write to distance field texels whose existing transition distance is further than the transition distance being scattered.
												if (NormalizedDistance * .5f < Abs(FinalShadowSample.Distance - .5f))
												{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
													// Debug when the selected texel is being scattered to
													// This may get hit any number of times, only the last hit will get stored in the distance field
													if (bDebugThisScatterTexel)
													{
														NumScattersToSelectedTexel++;
													}
#endif
													// Encode the transition distance so that [.5,0] corresponds to [0,1] for shadowed texels, and [.5,1] corresponds to [0,1] for unshadowed texels.
													// .5 of the encoded distance lies exactly on the shadow transition.
													FinalShadowSample.Distance = CurrentRegion ? (NormalizedDistance) * .5f + .5f : .5f - NormalizedDistance * .5f;
													// Approximate the penumbra size using PenumbraSize = (ReceiverDistanceFromLight - OccluderDistanceFromLight) * LightSize / OccluderDistanceFromLight,
													// Which is from the paper "Percentage-Closer Soft Shadows" by Randima Fernando
													const FLOAT ReceiverDistanceFromLight = (Light->LightCenterPosition(ScatterPosition) - ScatterPosition).Size();
													// World space distance from center of penumbra to fully shadowed or fully lit transition
													const FLOAT PenumbraSize = HighResSample.GetOccluderDistance() * Light->LightSourceRadius / (ReceiverDistanceFromLight - HighResSample.GetOccluderDistance());
													// Normalize the penumbra size so it is a fraction of MaxTransitionDistanceWorldSpace
													FinalShadowSample.PenumbraSize = Clamp(PenumbraSize / ShadowSettings.MaxTransitionDistanceWorldSpace, 0.01f, 1.0f);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		ShadowMaps.Set(Light, ShadowMapData);
	}
}

/**
 * Estimate direct lighting using the direct photon map.
 * This is only useful for debugging what the final gather rays see.
 */
void FStaticLightingSystem::CalculateDirectLightingTextureMappingPhotonMap(
	FStaticLightingTextureMapping* TextureMapping, 
	FStaticLightingMappingContext& MappingContext,
	FGatheredLightMapData2D& LightMapData, 
	TMap<const FLight*, FShadowMapData2D*>& ShadowMaps,
	const FTexelToVertexMap& TexelToVertexMap, 
	UBOOL bDebugThisMapping) const
{
	for (INT LightIndex = 0; LightIndex < TextureMapping->Mesh->RelevantLights.Num(); LightIndex++)
	{
		FLight* Light = TextureMapping->Mesh->RelevantLights(LightIndex);
		if (Light->GetMeshAreaLight() == NULL)
		{
			LightMapData.AddLight(Light);
		}
	}

	TArray<FIrradiancePhoton*> TempIrradiancePhotons;
	// Calculate direct lighting for each texel.
	for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
	{
		for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
		{
			UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisMapping
				&& Y == Scene.DebugInput.LocalY
				&& X == Scene.DebugInput.LocalX)
			{
				bDebugThisTexel = TRUE;
			}
#endif
			FGatheredLightSample& CurrentLightSample = LightMapData(X,Y);
			if (CurrentLightSample.bIsMapped)
			{
				const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
				FStaticLightingVertex CurrentVertex = TexelToVertex.GetVertex();

				if (PhotonMappingSettings.bUseIrradiancePhotons)
				{
					const FIrradiancePhoton* NearestPhoton = NULL;
					if (PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces)
					{
						// Trace a ray into the current texel to get a good representation of what the final gather will see.
						// Speed does not matter here since bUsePhotonsForDirectLighting is only used for debugging.
						const FLightRay TexelRay(
							CurrentVertex.WorldPosition + CurrentVertex.WorldTangentZ * TexelToVertex.TexelRadius,
							CurrentVertex.WorldPosition - CurrentVertex.WorldTangentZ * TexelToVertex.TexelRadius,
							TextureMapping,
							NULL
							);

						FLightRayIntersection Intersection;
						AggregateMesh.IntersectLightRay(TexelRay, TRUE, FALSE, FALSE, MappingContext.RayCache, Intersection);

						if (Intersection.bIntersects && TextureMapping == Intersection.Mapping)
						{
							CurrentVertex = Intersection.IntersectionVertex;
						}
						else
						{
							// Fall back to using the UV's of this texel
							CurrentVertex.TextureCoordinates[1] = FVector2D(X / (FLOAT)TextureMapping->CachedSizeX, Y / (FLOAT)TextureMapping->CachedSizeY);
						}
						// Find the nearest irradiance photon that was cached on this surface
						NearestPhoton = TextureMapping->GetCachedIrradiancePhoton(INDEX_NONE, CurrentVertex, *this, bDebugThisTexel && PhotonMappingSettings.bVisualizePhotonGathers && GeneralSettings.ViewSingleBounceNumber <= 0);
					}
					else
					{
						// Find the nearest irradiance photon by searching the irradiance photon map
						NearestPhoton = FindNearestIrradiancePhoton(CurrentVertex, MappingContext, TempIrradiancePhotons, FALSE, bDebugThisTexel);
					}
					const FLinearColor& PhotonIrradiance = NearestPhoton ? NearestPhoton->GetIrradiance() : FLinearColor::Black;
					if (GeneralSettings.ViewSingleBounceNumber < 1)
					{
						CurrentLightSample.AddWeighted(FGatheredLightSample::PointLight(PhotonIrradiance, FVector4(0,0,1)), 1.0f);
					}
				}
				else
				{
					// Estimate incident radiance from the photons in the direct photon map
					const FGatheredLightSample PhotonIncidentRadiance = CalculatePhotonIncidentRadiance(DirectPhotonMap, NumPhotonsEmittedDirect, PhotonMappingSettings.DirectPhotonSearchDistance, CurrentVertex, bDebugThisTexel);
					if (GeneralSettings.ViewSingleBounceNumber < 1)
					{
						CurrentLightSample.AddWeighted(PhotonIncidentRadiance, 1.0f);
					}
				}
			}
		}
	}
}

/** Handles indirect lighting calculations for a single texture mapping. */
void FStaticLightingSystem::CalculateIndirectLightingTextureMapping(
	FStaticLightingTextureMapping* TextureMapping,
	FStaticLightingMappingContext& MappingContext,
	FGatheredLightMapData2D& LightMapData, 
	const FTexelToVertexMap& TexelToVertexMap, 
	UBOOL bDebugThisMapping) const
{
	if (GeneralSettings.NumIndirectLightingBounces > 0 || AmbientOcclusionSettings.bUseAmbientOcclusion)
	{
		const DOUBLE IndirectLightingStartTime = appSeconds();
		FRandomStream SampleGenerator(0);

		// Calculate incident radiance from indirect lighting
		// With irradiance caching this is just the first pass, the results are added to the cache
		//@todo - use a hierarchical traversal to minimize the number of samples created
		// See "Problems and Solutions: Implementation Details" from the SIGGRAPH 2008 class titled "Practical Global Illumination with Irradiance Caching"
		for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
		{
			for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
			{
				UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugThisMapping
					&& Y == Scene.DebugInput.LocalY
					&& X == Scene.DebugInput.LocalX)
				{
					bDebugThisTexel = TRUE;
				}
#endif
				FGatheredLightSample& CurrentLightSample = LightMapData(X,Y);
				if (CurrentLightSample.bIsMapped)
				{
					const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
					checkSlow(TexelToVertex.TotalSampleWeight > 0.0f);
					FStaticLightingVertex TexelVertex = TexelToVertex.GetVertex();
					TexelVertex.TextureCoordinates[1] = FVector2D(X / (FLOAT)TextureMapping->CachedSizeX, Y / (FLOAT)TextureMapping->CachedSizeY);

					// Calculate incoming radiance for the frontface
					FGatheredLightSample IndirectLightingSample = CachePointIncomingRadiance(
						TextureMapping, 
						TexelVertex, 
						TexelToVertex.ElementIndex,
						INDEX_NONE,
						TexelToVertex.TexelRadius, 
						MappingContext, 
						SampleGenerator, 
						bDebugThisTexel);

					if (TextureMapping->Mesh->UsesTwoSidedLighting(TexelToVertex.ElementIndex))
					{
						TexelVertex.WorldTangentX = -TexelVertex.WorldTangentX;
						TexelVertex.WorldTangentY = -TexelVertex.WorldTangentY;
						TexelVertex.WorldTangentZ = -TexelVertex.WorldTangentZ;

						// Calculate incoming radiance for the backface
						const FGatheredLightSample BackFaceIndirectLightingSample = CachePointIncomingRadiance(
							TextureMapping, 
							TexelVertex, 
							TexelToVertex.ElementIndex,
							INDEX_NONE,
							TexelToVertex.TexelRadius, 
							MappingContext, 
							SampleGenerator, 
							bDebugThisTexel);
						// Average front and back face incident lighting
						IndirectLightingSample = (BackFaceIndirectLightingSample + IndirectLightingSample) * 0.5f;
					}

					// Only add caustic photon contribution if it was not already done through irradiance photons
					if (PhotonMappingSettings.bUsePhotonMapping
						&& GeneralSettings.NumIndirectLightingBounces > 0
						&& (!PhotonMappingSettings.bUseIrradiancePhotons || PhotonMappingSettings.bUseFinalGathering))
					{
						LIGHTINGSTAT(FScopedRDTSCTimer CausticPhotonGatherTimer(MappingContext.Stats.CausticPhotonGatherTime));
						// Calculate incident radiance from caustic photons
						const FGatheredLightSample CausticLighting = CalculatePhotonIncidentRadiance(
							CausticPhotonMap, 
							NumPhotonsEmittedCaustic, 
							PhotonMappingSettings.CausticPhotonSearchDistance, 
							TexelVertex, 
							bDebugThisTexel);

						if (GeneralSettings.ViewSingleBounceNumber != 0)
						{
							CurrentLightSample.AddWeighted(CausticLighting, 1.0f);
						}
					}

					if (!IrradianceCachingSettings.bAllowIrradianceCaching)
					{
						// Redistribute indirect lighting to be more centered around the dominant direction if desired
						const FGatheredLightSample RedistributedIndirectLighting = IndirectLightingSample.RedistributeToDominantDirection(SceneConstants.IndirectNormalInfluenceBoost);
						CurrentLightSample.AddWeighted(RedistributedIndirectLighting, 1.0f);
					}
				}
			}
		}

		const DOUBLE IrradianceCacheInterpolationStartTime = appSeconds();
		MappingContext.Stats.IndirectLightingTime += IrradianceCacheInterpolationStartTime - IndirectLightingStartTime;
		if (IrradianceCachingSettings.bAllowIrradianceCaching)
		{
			// Reseed the sample generator
			SampleGenerator = FRandomStream(0);

			// Interpolate irradiance cache samples in a separate shading pass
			// This avoids interpolating to positions where more samples will be added later, which would create a discontinuity
			// Also allows us to use more lenient restrictions in this pass, which effectively smooths the irradiance cache results
			for (INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
			{
				for (INT X = 0; X < TextureMapping->CachedSizeX; X++)
				{
					UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
					if (bDebugThisMapping
						&& Y == Scene.DebugInput.LocalY
						&& X == Scene.DebugInput.LocalX)
					{
						bDebugThisTexel = TRUE;
					}
#endif
					FGatheredLightSample& CurrentLightSample = LightMapData(X,Y);
					if (CurrentLightSample.bIsMapped)
					{
						const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
						checkSlow(TexelToVertex.TotalSampleWeight > 0.0f);
						FStaticLightingVertex TexelVertex = TexelToVertex.GetVertex();
						FFinalGatherSample IndirectLighting;
						// Interpolate the indirect lighting from the irradiance cache
						// Interpolation must succeed since this is the second pass
						verify(MappingContext.FirstBounceCache.InterpolateLighting(TexelVertex, FALSE, bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == 1, IndirectLighting));
						
						if (TextureMapping->Mesh->UsesTwoSidedLighting(TexelToVertex.ElementIndex))
						{
							TexelVertex.WorldTangentX = -TexelVertex.WorldTangentX;
							TexelVertex.WorldTangentY = -TexelVertex.WorldTangentY;
							TexelVertex.WorldTangentZ = -TexelVertex.WorldTangentZ;

							FFinalGatherSample BackFaceIndirectLighting;
							// Interpolate indirect lighting for the back face
							verify(MappingContext.FirstBounceCache.InterpolateLighting(TexelVertex, FALSE, bDebugThisTexel && GeneralSettings.ViewSingleBounceNumber == 1, BackFaceIndirectLighting));
							// Average front and back face incident lighting
							IndirectLighting = (BackFaceIndirectLighting + IndirectLighting) * 0.5f;
						}
						
						FLOAT IndirectOcclusion = 1.0f;
						if (AmbientOcclusionSettings.bUseAmbientOcclusion)
						{
							const FLOAT DirectOcclusion = 1.0f - AmbientOcclusionSettings.DirectIlluminationOcclusionFraction * IndirectLighting.Occlusion;
							// Apply occlusion to direct lighting, assuming CurrentLightSample only contains direct lighting
							//@todo - currently this affects caustics as well
							CurrentLightSample = CurrentLightSample * DirectOcclusion;
							IndirectOcclusion = 1.0f - AmbientOcclusionSettings.IndirectIlluminationOcclusionFraction * IndirectLighting.Occlusion;
						}

						// Redistribute indirect lighting to be more centered around the dominant direction if desired
						FGatheredLightSample RedistributedIndirectLighting = IndirectLighting.RedistributeToDominantDirection(SceneConstants.IndirectNormalInfluenceBoost);

						const FVector4 TangentNormal = MaterialSettings.bUseNormalMapsForSimpleLightMaps ?
							TextureMapping->Mesh->EvaluateNormal(TexelVertex.TextureCoordinates[0], TexelToVertex.ElementIndex) :
							FVector4( 0.0f, 0.0f, 1.0f, 0.0f );

						// Transform the tangent normal into the lightmap basis
						// Each component of BasisWeights is the dot product between the tangent normal and the corresponding lightmap basis vector
						FVector4 BasisWeights = FBasisVectorMatrix(LightMapBasis[0], LightMapBasis[1], LightMapBasis[2], FVector4(0, 0, 0)).TransformFVector(TangentNormal);
						// Square the weights to increase contrast, and so the weights add up to 1 at the per-vertex normal ((0,0,1) in tangent space)
						BasisWeights *= BasisWeights;
						// Derive the simple lightmap value from a weighted average of the incident lighting along the 3 basis vectors,
						// Where basis vectors that are closer to the normal's direction get a larger weight.
						RedistributedIndirectLighting.FromLinearColor(LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX, 
							RedistributedIndirectLighting.ToLinearColor(0) * BasisWeights.X
							+ RedistributedIndirectLighting.ToLinearColor(1) * BasisWeights.Y
							+ RedistributedIndirectLighting.ToLinearColor(2) * BasisWeights.Z);

						// Apply occlusion to indirect lighting and add this texel's indirect lighting to its running total
						CurrentLightSample.AddWeighted(RedistributedIndirectLighting, IndirectOcclusion);

						if (AmbientOcclusionSettings.bUseAmbientOcclusion && AmbientOcclusionSettings.bVisualizeAmbientOcclusion)
						{
							//@todo - this will only be the correct intensity for simple lightmaps
							const FGatheredLightSample OcclusionVisualization = FGatheredLightSample::PointLight(
								FLinearColor(1.0f - IndirectLighting.Occlusion, 1.0f - IndirectLighting.Occlusion, 1.0f - IndirectLighting.Occlusion) * 0.5f,
								FVector4(0,0,1));
							// Overwrite the lighting accumulated so far
							CurrentLightSample = OcclusionVisualization;
							CurrentLightSample.bIsMapped = TRUE;
						}
					}
				}
			}
		}
		const DOUBLE IrradianceCacheInterpolationEndTime = appSeconds();
		MappingContext.Stats.SecondPassIrradianceCacheInterpolationTime += IrradianceCacheInterpolationEndTime - IrradianceCacheInterpolationStartTime;
	}

	// Add the area lights to the light-map's light list.
	UBOOL bHasAreaLights = FALSE;
	UBOOL bHasShadowedAreaLights = FALSE;
	for(INT LightIndex = 0;LightIndex < TextureMapping->Mesh->RelevantLights.Num();LightIndex++)
	{
		FSkyLight* SkyLight = TextureMapping->Mesh->RelevantLights(LightIndex)->GetSkyLight();
		if (SkyLight)
		{
			LightMapData.Lights.AddUniqueItem(SkyLight);
			bHasAreaLights = TRUE;
			if((SkyLight->LightFlags & GI_LIGHT_CASTSHADOWS) && (SkyLight->LightFlags & GI_LIGHT_CASTSTATICSHADOWS))
			{
				bHasShadowedAreaLights = TRUE;
			}
		}
	}

	// Compute the area lighting for each of the mapping's texels.
	// Do it twice to allow the first pass to prime the lighting cache.
	if(bHasAreaLights)
	{
		FRandomStream SampleGenerator(0);
		// Skip the priming pass if radiance caching is disabled
		for(INT Pass = IrradianceCachingSettings.bAllowIrradianceCaching ? 0 : 1;Pass < 2;Pass++)
		{
			const UBOOL bApplyAreaLighting = (Pass == 1);
			if(bApplyAreaLighting || bHasShadowedAreaLights)
			{
				for(INT Y = 0;Y < TextureMapping->CachedSizeY;Y++)
				{
					for(INT X = 0;X < TextureMapping->CachedSizeX;X++)
					{
						UBOOL bDebugCurrentTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
						if (bDebugThisMapping)
						{
							if (X == Scene.DebugInput.LocalX 
								&& Y == Scene.DebugInput.LocalY)
							{
								bDebugCurrentTexel = TRUE;
							}
						}
#endif
						if (LightMapData(X,Y).bIsMapped)
						{
							const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
							// Compute the sample's sky lighting.
							const FGatheredLightSample AreaLightSample = CalculateSkyLighting(TextureMapping,TexelToVertex.GetVertex(),MappingContext,SampleGenerator);

							// Don't apply the area lighting in the first pass, since the area lighting cache hasn't been fully populated et.
							if(bApplyAreaLighting)
							{
								LightMapData(X,Y).AddWeighted(AreaLightSample,1.0f);
							}
						}
					}
				}
			}
		}
	}
}

/** Overrides LightMapData with material attributes if MaterialSettings.ViewMaterialAttribute != VMA_None */
void FStaticLightingSystem::ViewMaterialAttributesTextureMapping(
	FStaticLightingTextureMapping* TextureMapping,
	FStaticLightingMappingContext& MappingContext,
	FGatheredLightMapData2D& LightMapData, 
	const FTexelToVertexMap& TexelToVertexMap, 
	UBOOL bDebugThisMapping) const
{
	if (MaterialSettings.ViewMaterialAttribute != VMA_None)
	{
		for(INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
		{
			for(INT X = 0; X < TextureMapping->CachedSizeX; X++)
			{
				UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugThisMapping
					&& Y == Scene.DebugInput.LocalY
					&& X == Scene.DebugInput.LocalX)
				{
					bDebugThisTexel = TRUE;
				}
#endif
				FGatheredLightSample& CurrentLightSample = LightMapData(X,Y);
				if (CurrentLightSample.bIsMapped)
				{
					const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap(X,Y);
					checkSlow(TexelToVertex.TotalSampleWeight > 0.0f);
					FStaticLightingVertex CurrentVertex = TexelToVertex.GetVertex();

					// Trace a ray into the current texel to get a good representation of what material lookups from ray intersections will see.
					// Speed does not matter here since this visualization is only used for debugging.
					const FLightRay TexelRay(
						CurrentVertex.WorldPosition + CurrentVertex.WorldTangentZ * TexelToVertex.TexelRadius,
						CurrentVertex.WorldPosition - CurrentVertex.WorldTangentZ * TexelToVertex.TexelRadius,
						TextureMapping,
						NULL
						);

					FLightRayIntersection Intersection;
					AggregateMesh.IntersectLightRay(TexelRay, TRUE, TRUE, FALSE, MappingContext.RayCache, Intersection);
					CurrentLightSample = GetVisualizedMaterialAttribute(TextureMapping, Intersection);
				}
			}
		}
	}
}

/** A map from texel to the number of triangles mapped to that texel. */
class FTexelToNumTrianglesMap
{
public:

	/** Stores information about a texel needed for determining the validity of the lightmap UVs. */
	struct FTexelToNumTriangles
	{
		UBOOL bWrappingUVs;
		INT NumTriangles;
	};

	/** Initialization constructor. */
	FTexelToNumTrianglesMap(INT InSizeX, INT InSizeY) :
		Data(InSizeX * InSizeY),
		SizeX(InSizeX),
		SizeY(InSizeY)
	{
		// Clear the map to zero.
		for(INT Y = 0; Y < SizeY; Y++)
		{
			for(INT X = 0; X < SizeX; X++)
			{
				appMemzero(&(*this)(X,Y), sizeof(FTexelToNumTriangles));
			}
		}
	}

	// Accessors.
	FTexelToNumTriangles& operator()(INT X, INT Y)
	{
		const UINT TexelIndex = Y * SizeX + X;
		return Data(TexelIndex);
	}
	const FTexelToNumTriangles& operator()(INT X, INT Y) const
	{
		const INT TexelIndex = Y * SizeX + X;
		return Data(TexelIndex);
	}

	INT GetSizeX() const { return SizeX; }
	INT GetSizeY() const { return SizeY; }

private:

	/** The mapping data. */
	TChunkedArray<FTexelToNumTriangles> Data;

	/** The width of the mapping data. */
	INT SizeX;

	/** The height of the mapping data. */
	INT SizeY;
};

/** Rasterization policy for verifying unique lightmap UVs. */
class FUniqueMappingRasterPolicy
{
public:

	typedef UBOOL InterpolantType;

	/** Initialization constructor. */
	FUniqueMappingRasterPolicy(
		const FScene& InScene,
		FTexelToNumTrianglesMap& InTexelToNumTrianglesMap,
		UBOOL bInDebugThisMapping
		) :
		Scene(InScene),
		TexelToNumTrianglesMap(InTexelToNumTrianglesMap),
		bDebugThisMapping(bInDebugThisMapping)
	{}

protected:

	// FTriangleRasterizer policy interface.
	INT GetMinX() const { return 0; }
	INT GetMaxX() const { return TexelToNumTrianglesMap.GetSizeX() - 1; }
	INT GetMinY() const { return 0; }
	INT GetMaxY() const { return TexelToNumTrianglesMap.GetSizeY() - 1; }

	void ProcessPixel(INT X, INT Y, const InterpolantType& Interpolant, UBOOL BackFacing);

private:

	const FScene& Scene;

	/** The texel to vertex map which is being rasterized to. */
	FTexelToNumTrianglesMap& TexelToNumTrianglesMap;

	const UBOOL bDebugThisMapping;
};

void FUniqueMappingRasterPolicy::ProcessPixel(INT X,INT Y,const InterpolantType& bWrappingUVs,UBOOL BackFacing)
{
	FTexelToNumTrianglesMap::FTexelToNumTriangles& TexelToNumTriangles = TexelToNumTrianglesMap(X,Y);
	UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisMapping
		&& X == Scene.DebugInput.LocalX
		&& Y == Scene.DebugInput.LocalY)
	{
		bDebugThisTexel = TRUE;
	}
#endif
	TexelToNumTriangles.NumTriangles++;
	TexelToNumTriangles.bWrappingUVs = bWrappingUVs;
}

/** Colors texels with invalid lightmap UVs to make it obvious that they are wrong. */
void FStaticLightingSystem::ColorInvalidLightmapUVs(
	const FStaticLightingTextureMapping* TextureMapping, 
	FGatheredLightMapData2D& LightMapData, 
	UBOOL bDebugThisMapping) const
{
	FTexelToNumTrianglesMap TexelToNumTrianglesMap(TextureMapping->CachedSizeX, TextureMapping->CachedSizeY);

	// Rasterize the triangle using the mapping's texture coordinate channel.
	FTriangleRasterizer<FUniqueMappingRasterPolicy> TexelMappingRasterizer(FUniqueMappingRasterPolicy(
		Scene,
		TexelToNumTrianglesMap,
		bDebugThisMapping
		));

	// Rasterize the triangles
	for(INT TriangleIndex = 0;TriangleIndex < TextureMapping->Mesh->NumTriangles;TriangleIndex++)
	{
		// Query the mesh for the triangle's vertices.
		FStaticLightingVertex V0;
		FStaticLightingVertex V1;
		FStaticLightingVertex V2;
		INT DummyElement;
		TextureMapping->Mesh->GetTriangle(TriangleIndex,V0,V1,V2,DummyElement);

		const FVector2D UV0 = V0.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex];
		const FVector2D UV1 = V1.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex];
		const FVector2D UV2 = V2.TextureCoordinates[TextureMapping->LightmapTextureCoordinateIndex];

		UBOOL bHasWrappingLightmapUVs = FALSE;
		//@todo - remove the thresholds and fixup existing content
		if (UV0.X < -DELTA || UV0.X >= 1.0f + DELTA
			|| UV0.Y < -DELTA || UV0.Y >= 1.0f + DELTA
			|| UV1.X < -DELTA || UV1.X >= 1.0f + DELTA
			|| UV1.Y < -DELTA || UV1.Y >= 1.0f + DELTA
			|| UV2.X < -DELTA || UV2.X >= 1.0f + DELTA
			|| UV2.Y < -DELTA || UV2.Y >= 1.0f + DELTA)
		{
			bHasWrappingLightmapUVs = TRUE;
		}

		// Only rasterize the center of the texel
		TexelMappingRasterizer.DrawTriangle(
			bHasWrappingLightmapUVs,
			bHasWrappingLightmapUVs,
			bHasWrappingLightmapUVs,
			UV0 * FVector2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY) + FVector2D(-0.5f,-0.5f),
			UV1 * FVector2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY) + FVector2D(-0.5f,-0.5f),
			UV2 * FVector2D(TextureMapping->CachedSizeX,TextureMapping->CachedSizeY) + FVector2D(-0.5f,-0.5f),
			FALSE 
			);
	}

	UBOOL bHasWrappingUVs = FALSE;
	UBOOL bHasOverlappedUVs = FALSE;
	for(INT Y = 0; Y < TextureMapping->CachedSizeY; Y++)
	{
		// Color texels belonging to vertices with wrapping lightmap UV's bright green
		// Color texels that have more than one triangle mapped to them bright orange
		for(INT X = 0; X < TextureMapping->CachedSizeX; X++)
		{
			UBOOL bDebugThisTexel = FALSE;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisMapping
				&& Y == Scene.DebugInput.LocalY
				&& X == Scene.DebugInput.LocalX)
			{
				bDebugThisTexel = TRUE;
			}
#endif
			FGatheredLightSample& CurrentLightSample = LightMapData(X,Y);
			if (CurrentLightSample.bIsMapped)
			{
				const FTexelToNumTrianglesMap::FTexelToNumTriangles& TexelToNumTriangles = TexelToNumTrianglesMap(X,Y);
				if (TexelToNumTriangles.bWrappingUVs)
				{
					bHasWrappingUVs = TRUE;
					if (Scene.GeneralSettings.bUseErrorColoring && MaterialSettings.ViewMaterialAttribute == VMA_None)
					{
						// Color texels belonging to vertices with wrapping lightmap UV's bright green
						if (TextureMapping->Mesh->ShouldColorInvalidTexels())
						{
							CurrentLightSample = FGatheredLightSample::PointLight(FLinearColor(0.5f, 2.0f, 0.0f), FVector4(0,0,1));
							CurrentLightSample.bIsMapped = TRUE;
						}
					}
				}
				else if (TexelToNumTriangles.NumTriangles > 1)
				{
					bHasOverlappedUVs = TRUE;
					if (Scene.GeneralSettings.bUseErrorColoring && MaterialSettings.ViewMaterialAttribute == VMA_None)
					{
						// Color texels that have more than one triangle mapped to them bright orange
						if (TextureMapping->Mesh->ShouldColorInvalidTexels())
						{
							CurrentLightSample = FGatheredLightSample::PointLight(FLinearColor(2.0f, 0.7f, 0.0f), FVector4(0,0,1));
							CurrentLightSample.bIsMapped = TRUE;
						}
					}
				}
			}
		}
	}

	if (bHasWrappingUVs || bHasOverlappedUVs)
	{
		INT TypeId = TextureMapping->Mesh->GetObjectType();
		FGuid ObjectGuid = TextureMapping->Mesh->GetObjectGuid();
		if (bHasWrappingUVs)
		{
			GSwarm->SendAlertMessage(NSwarm::ALERT_LEVEL_ERROR, ObjectGuid, TypeId, TEXT("LightmassError_ObjectWrappedUVs"));
		}
		if (bHasOverlappedUVs)
		{
			GSwarm->SendAlertMessage(NSwarm::ALERT_LEVEL_ERROR, ObjectGuid, TypeId, TEXT("LightmassError_ObjectOverlappedUVs"));
		}
	}
}

/** Adds a texel of padding around texture mappings and copies the nearest texel into the padding. */
void FStaticLightingSystem::PadTextureMapping(
	const FStaticLightingTextureMapping* TextureMapping,
	const FGatheredLightMapData2D& LightMapData,
	FGatheredLightMapData2D& PaddedLightMapData,
	TMap<const FLight*, FShadowMapData2D*>& ShadowMaps,
	TMap<const FLight*, FSignedDistanceFieldShadowMapData2D*>& SignedDistanceFieldShadowMaps) const
{
	if (TextureMapping->bPadded)
	{
		check(TextureMapping->SizeX == TextureMapping->CachedSizeX + 2);
		check(TextureMapping->SizeY == TextureMapping->CachedSizeY + 2);
		// We need to expand it back out...
		UINT TrueSizeX = TextureMapping->SizeX;
		UINT TrueSizeY = TextureMapping->SizeY;
		FGatheredLightSample DebugLightSample = FGatheredLightSample::PointLight(FLinearColor(1.0f,0.0f,1.0f), FVector4(0.0f,0.0f,1.0f));
		for (UINT CopyY = 0; CopyY < TrueSizeY; CopyY++)
		{
			if (CopyY == 0)
			{
				// The first row, left corner
				PaddedLightMapData(0,0) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugLightSample : LightMapData(0,0);
				// The rest of the row, short of the right corner
				for (UINT TempX = 0; TempX < (UINT)(TextureMapping->CachedSizeX); TempX++)
				{
					PaddedLightMapData(TempX+1,0) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugLightSample : LightMapData(TempX,0);
				}
				// The right corner
				PaddedLightMapData(TrueSizeX-1,0) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugLightSample : LightMapData(TextureMapping->CachedSizeX-1,0);
			}
			else if (CopyY == TrueSizeY - 1)
			{
				// The last row, left corner
				PaddedLightMapData(0,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugLightSample : LightMapData(0,TextureMapping->CachedSizeY-1);
				// The rest of the row, short of the right corner
				for (UINT TempX = 0; TempX < (UINT)(TextureMapping->CachedSizeX); TempX++)
				{
					PaddedLightMapData(TempX+1,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugLightSample : LightMapData(TempX,TextureMapping->CachedSizeY-1);
				}
				// The right corner
				PaddedLightMapData(TrueSizeX-1,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugLightSample : LightMapData(TextureMapping->CachedSizeX-1,TextureMapping->CachedSizeY-1);
			}
			else
			{
				// The last row, left corner
				PaddedLightMapData(0,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugLightSample : LightMapData(0,CopyY-1);
				// The rest of the row, short of the right corner
				for (UINT TempX = 0; TempX < (UINT)(TextureMapping->CachedSizeX); TempX++)
				{
					PaddedLightMapData(TempX+1,CopyY) =LightMapData(TempX,CopyY-1);
				}
				// The right corner
				PaddedLightMapData(TrueSizeX-1,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugLightSample : LightMapData(TextureMapping->CachedSizeX-1,CopyY-1);
			}
		}
		PaddedLightMapData.Lights = LightMapData.Lights;

		FShadowSample DebugShadowSample;
		DebugShadowSample.bIsMapped = TRUE;
		DebugShadowSample.Visibility = 0.7f;
		for (TMap<const FLight*, FShadowMapData2D*>::TIterator It(ShadowMaps); It; ++It)
		{
			const FLight* Key = It.Key();
			FShadowMapData2D* ShadowMapData = It.Value();
			FShadowMapData2D* TempShadowMapData = new FShadowMapData2D(TrueSizeX, TrueSizeY);

			// Expand it
			for (UINT CopyY = 0; CopyY < TrueSizeY; CopyY++)
			{
				if (CopyY == 0)
				{
					// The first row, left corner
					(*TempShadowMapData)(0,0) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugShadowSample : (*ShadowMapData)(0,0);
					// The rest of the row, short of the right corner
					for (UINT TempX = 0; TempX < (UINT)(TextureMapping->CachedSizeX); TempX++)
					{
						(*TempShadowMapData)(TempX+1,0) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugShadowSample : (*ShadowMapData)(TempX,0) * 2.0f - (*ShadowMapData)(TempX,1);
					}
					// The right corner
					(*TempShadowMapData)(TrueSizeX-1,0) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugShadowSample : (*ShadowMapData)(TextureMapping->CachedSizeX-1,0);
				}
				else if (CopyY == TrueSizeY - 1)
				{
					// The last row, left corner
					(*TempShadowMapData)(0,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugShadowSample : (*ShadowMapData)(0,TextureMapping->CachedSizeY-1);
					// The rest of the row, short of the right corner
					for (UINT TempX = 0; TempX < (UINT)(TextureMapping->CachedSizeX); TempX++)
					{
						(*TempShadowMapData)(TempX+1,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugShadowSample : (*ShadowMapData)(TempX,TextureMapping->CachedSizeY-1) * 2.0f - (*ShadowMapData)(TempX,TextureMapping->CachedSizeY-2);
					}
					// The right corner
					(*TempShadowMapData)(TrueSizeX-1,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugShadowSample : (*ShadowMapData)(TextureMapping->CachedSizeX-1,TextureMapping->CachedSizeY-1);
				}
				else
				{
					// The last row, left corner
					(*TempShadowMapData)(0,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugShadowSample : (*ShadowMapData)(0,CopyY-1) * 2.0f - (*ShadowMapData)(1,CopyY-1);
					// The rest of the row, short of the right corner
					for (UINT TempX = 0; TempX < (UINT)(TextureMapping->CachedSizeX); TempX++)
					{
						(*TempShadowMapData)(TempX+1,CopyY) = (*ShadowMapData)(TempX,CopyY-1);
					}
					// The right corner
					(*TempShadowMapData)(TrueSizeX-1,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugShadowSample : (*ShadowMapData)(TextureMapping->CachedSizeX-1,CopyY-1) * 2.0f - (*ShadowMapData)(TextureMapping->CachedSizeX-2,CopyY-1);
				}
			}

			// Copy it back in
			ShadowMaps.Set(Key, TempShadowMapData);
			delete ShadowMapData;
		}


		FSignedDistanceFieldShadowSample DebugDistanceShadowSample;
		DebugDistanceShadowSample.bIsMapped = TRUE;
		DebugDistanceShadowSample.Distance = .5f;
		for (TMap<const FLight*, FSignedDistanceFieldShadowMapData2D*>::TIterator It(SignedDistanceFieldShadowMaps); It; ++It)
		{
			const FLight* Key = It.Key();
			FSignedDistanceFieldShadowMapData2D* ShadowMapData = It.Value();
			FSignedDistanceFieldShadowMapData2D* TempShadowMapData = new FSignedDistanceFieldShadowMapData2D(TrueSizeX, TrueSizeY);

			// Expand it
			for (UINT CopyY = 0; CopyY < TrueSizeY; CopyY++)
			{
				if (CopyY == 0)
				{
					// The first row, left corner
					(*TempShadowMapData)(0,0) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugDistanceShadowSample : (*ShadowMapData)(0,0);
					// The rest of the row, short of the right corner
					for (UINT TempX = 0; TempX < (UINT)(TextureMapping->CachedSizeX); TempX++)
					{
						// Extrapolate the padding texels, maintaining the same slope that the source data had, which is important for distance field shadows
						(*TempShadowMapData)(TempX+1,0) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugDistanceShadowSample : (*ShadowMapData)(TempX,0) * 2.0f - (*ShadowMapData)(TempX,1);
					}
					// The right corner
					(*TempShadowMapData)(TrueSizeX-1,0) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugDistanceShadowSample : (*ShadowMapData)(TextureMapping->CachedSizeX-1,0);
				}
				else if (CopyY == TrueSizeY - 1)
				{
					// The last row, left corner
					(*TempShadowMapData)(0,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugDistanceShadowSample : (*ShadowMapData)(0,TextureMapping->CachedSizeY-1);
					// The rest of the row, short of the right corner
					for (UINT TempX = 0; TempX < (UINT)(TextureMapping->CachedSizeX); TempX++)
					{
						(*TempShadowMapData)(TempX+1,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugDistanceShadowSample : (*ShadowMapData)(TempX,TextureMapping->CachedSizeY-1) * 2.0f - (*ShadowMapData)(TempX,TextureMapping->CachedSizeY-2);
					}
					// The right corner
					(*TempShadowMapData)(TrueSizeX-1,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugDistanceShadowSample : (*ShadowMapData)(TextureMapping->CachedSizeX-1,TextureMapping->CachedSizeY-1);
				}
				else
				{
					// The last row, left corner
					(*TempShadowMapData)(0,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugDistanceShadowSample : (*ShadowMapData)(0,CopyY-1) * 2.0f - (*ShadowMapData)(1,CopyY-1);
					// The rest of the row, short of the right corner
					for (UINT TempX = 0; TempX < (UINT)(TextureMapping->CachedSizeX); TempX++)
					{
						(*TempShadowMapData)(TempX+1,CopyY) = (*ShadowMapData)(TempX,CopyY-1);
					}
					// The right corner
					(*TempShadowMapData)(TrueSizeX-1,CopyY) = FStaticLightingMapping::s_bShowLightmapBorders ? DebugDistanceShadowSample : (*ShadowMapData)(TextureMapping->CachedSizeX-1,CopyY-1) * 2.0f - (*ShadowMapData)(TextureMapping->CachedSizeX-2,CopyY-1);
				}
			}

			// Copy it back in
			SignedDistanceFieldShadowMaps.Set(Key, TempShadowMapData);
			delete ShadowMapData;
		}
	}
	else
	{
		PaddedLightMapData = LightMapData;
	}
}

/** Rasterizes Mesh into TexelToCornersMap */
void FStaticLightingSystem::CalculateTexelCorners(const FStaticLightingMesh* Mesh, FTexelToCornersMap& TexelToCornersMap, INT UVIndex, UBOOL bDebugThisMapping) const
{
	static const FVector2D CornerOffsets[NumTexelCorners] = 
	{
		FVector2D(0, 0),
		FVector2D(-1, 0),
		FVector2D(0, -1),
		FVector2D(-1, -1)
	};

	// Rasterize each triangle of the mesh
	for (INT TriangleIndex = 0; TriangleIndex < Mesh->NumTriangles; TriangleIndex++)
	{
		// Query the mesh for the triangle's vertices.
		FStaticLightingVertex V0;
		FStaticLightingVertex V1;
		FStaticLightingVertex V2;
		INT TriangleElement;
		Mesh->GetTriangle(TriangleIndex, V0, V1, V2, TriangleElement);

		// Rasterize each triangle offset by the corner offsets
		for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
		{
			FTriangleRasterizer<FTexelCornerRasterPolicy> TexelCornerRasterizer(FTexelCornerRasterPolicy(
				Scene,
				TexelToCornersMap,
				CornerIndex,
				bDebugThisMapping
				));

			TexelCornerRasterizer.DrawTriangle(
				V0,
				V1,
				V2,
				V0.TextureCoordinates[UVIndex] * FVector2D(TexelToCornersMap.GetSizeX(), TexelToCornersMap.GetSizeY()) + CornerOffsets[CornerIndex],
				V1.TextureCoordinates[UVIndex] * FVector2D(TexelToCornersMap.GetSizeX(), TexelToCornersMap.GetSizeY()) + CornerOffsets[CornerIndex],
				V2.TextureCoordinates[UVIndex] * FVector2D(TexelToCornersMap.GetSizeX(), TexelToCornersMap.GetSizeY()) + CornerOffsets[CornerIndex],
				FALSE
				);
		}
	}
}

/** Rasterizes Mesh into TexelToCornersMap, with extra parameters like which material index to rasterize and UV scale and bias. */
void FStaticLightingSystem::CalculateTexelCorners(
	const TArray<INT>& TriangleIndices, 
	const TArray<FStaticLightingVertex>& Vertices, 
	FTexelToCornersMap& TexelToCornersMap, 
	const TArray<INT>& ElementIndices,
	INT MaterialIndex,
	INT UVIndex, 
	UBOOL bDebugThisMapping, 
	FVector2D UVBias, 
	FVector2D UVScale) const
{
	static const FVector2D CornerOffsets[NumTexelCorners] = 
	{
		FVector2D(0, 0),
		FVector2D(-1, 0),
		FVector2D(0, -1),
		FVector2D(-1, -1)
	};

	// Rasterize each triangle of the mesh
	for (INT TriangleIndex = 0; TriangleIndex < TriangleIndices.Num(); TriangleIndex++)
	{
		if (ElementIndices(TriangleIndices(TriangleIndex)) == MaterialIndex)
		{
			const FStaticLightingVertex& V0 = Vertices(TriangleIndices(TriangleIndex) * 3 + 0);
			const FStaticLightingVertex& V1 = Vertices(TriangleIndices(TriangleIndex) * 3 + 1);
			const FStaticLightingVertex& V2 = Vertices(TriangleIndices(TriangleIndex) * 3 + 2);

			// Rasterize each triangle offset by the corner offsets
			for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
			{
				FTriangleRasterizer<FTexelCornerRasterPolicy> TexelCornerRasterizer(FTexelCornerRasterPolicy(
					Scene,
					TexelToCornersMap,
					CornerIndex,
					bDebugThisMapping
					));

				TexelCornerRasterizer.DrawTriangle(
					V0,
					V1,
					V2,
					UVScale * (UVBias + V0.TextureCoordinates[UVIndex]) * FVector2D(TexelToCornersMap.GetSizeX(), TexelToCornersMap.GetSizeY()) + CornerOffsets[CornerIndex],
					UVScale * (UVBias + V1.TextureCoordinates[UVIndex]) * FVector2D(TexelToCornersMap.GetSizeX(), TexelToCornersMap.GetSizeY()) + CornerOffsets[CornerIndex],
					UVScale * (UVBias + V2.TextureCoordinates[UVIndex]) * FVector2D(TexelToCornersMap.GetSizeX(), TexelToCornersMap.GetSizeY()) + CornerOffsets[CornerIndex],
					FALSE
					);
			}
		}
	}
}

/** Accesses a cached photon at the given vertex, if one exists. */
const FIrradiancePhoton* FStaticLightingTextureMapping::GetCachedIrradiancePhoton(INT VertexIndex, const FStaticLightingVertex& Vertex, const FStaticLightingSystem& System, UBOOL bDebugThisLookup) const
{
	checkSlow(IrradiancePhotonCacheSizeX > 0 && IrradiancePhotonCacheSizeY > 0);
	// Clamping is necessary since the UV's may be outside the [0, 1) range
	const INT PhotonX = Clamp(appTrunc(Vertex.TextureCoordinates[1].X * IrradiancePhotonCacheSizeX), 0, IrradiancePhotonCacheSizeX - 1);
	const INT PhotonY = Clamp(appTrunc(Vertex.TextureCoordinates[1].Y * IrradiancePhotonCacheSizeY), 0, IrradiancePhotonCacheSizeY - 1);
	const INT PhotonIndex = PhotonY * IrradiancePhotonCacheSizeX + PhotonX;

	const FIrradiancePhoton* ClosestPhoton = CachedIrradiancePhotons(PhotonIndex);
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisLookup && ClosestPhoton != NULL)
	{
		System.DebugOutput.GatheredPhotons.AddItem(FDebugPhoton(0, ClosestPhoton->GetPosition(), ClosestPhoton->GetSurfaceNormal(), ClosestPhoton->GetSurfaceNormal()));
	}
#endif

	return ClosestPhoton;
}

}
