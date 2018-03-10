/*=============================================================================
	Scene.h: Scene classes.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "..\Public\SceneExport.h"

namespace Lightmass
{

class FDirectionalLight;
class FPointLight;
class FSpotLight;
class FSkyLight;
class FStaticMesh;
class FMaterial;
class FStaticMeshStaticLightingMesh;
class FFluidSurfaceStaticLightingMesh;
class FLandscapeStaticLightingMesh;
class FSpeedTreeStaticLightingMesh;
class FStaticMeshStaticLightingTextureMapping;
class FStaticMeshStaticLightingVertexMapping;
class FBSPSurfaceStaticLighting;
class FTerrainComponentStaticLighting;
class FFluidSurfaceStaticLightingTextureMapping;
class FLandscapeStaticLightingTextureMapping;
class FSpeedTreeStaticLightingMapping;
class FScene;

/** A sample of a light's surface. */
class FLightSurfaceSample
{
public:
	/** World space position */
	FVector4 Position;
    /** Normal of the light's surface at the sample point */
	FVector4 Normal;
	/** Position on the disk for lights modelled by a disk */
	FVector2D DiskPosition;
	/** The probability that a sample with this position was generated */
	FLOAT PDF;

	FLightSurfaceSample() {}

	FLightSurfaceSample(const FVector4& InPosition, const FVector4& InNormal, const FVector2D& InDiskPosition, FLOAT InPDF) :
		Position(InPosition),
		Normal(InNormal),
		DiskPosition(InDiskPosition),
		PDF(InPDF)
	{}
};

/** A path that was found to result in at least one indirect photon being deposited. */
class FIndirectPathRay
{
public:
	FVector4 Start;
	FVector4 UnitDirection;
	FVector4 LightSourceNormal;
	FVector2D LightSurfacePosition;
	FLOAT Length;

	FIndirectPathRay(const FVector4& InStart, const FVector4& InUnitDirection, const FVector4& InLightSourceNormal, const FVector2D& InLightSurfacePosition, const FLOAT& InLength) :
		Start(InStart),
		UnitDirection(InUnitDirection),
		LightSourceNormal(InLightSourceNormal),
		LightSurfacePosition(InLightSurfacePosition),
		Length(InLength)
	{}
};

class FIrradiancePhotonData
{
protected:
	/** XYZ stores world space position, W stores 1 if the photon has contribution from direct photons, and 0 otherwise. */
	FVector4 PositionAndDirectContribution;

	/** 
	* XYZ stores the world space normal of the receiving surface.
	* The irradiance photon caching pass stores 1 in W if the photon is actually used. 
	* This is overwritten with the photon's irradiance in RGBE in the irradiance photon calculating pass.
	*/
	FVector4 SurfaceNormalAndIrradiance;
};

//----------------------------------------------------------------------------
//	Light base class
//----------------------------------------------------------------------------
class FLight : public FLightData
{
public:
	virtual void			Read( FILE* FileHandle);
	virtual void			Import( class FLightmassImporter& Importer );

	/**
	 * @return 'this' if the light is a skylight, NULL otherwise 
	 */
	virtual const class FSkyLight* GetSkyLight() const
	{
		return NULL;
	}
	virtual class FSkyLight* GetSkyLight()
	{
		return NULL;
	}

	virtual class FDirectionalLight* GetDirectionalLight() 
	{
		return NULL;
	}

	virtual const class FDirectionalLight* GetDirectionalLight() const
	{
		return NULL;
	}

	virtual class FPointLight* GetPointLight() 
	{
		return NULL;
	}

	virtual const class FPointLight* GetPointLight() const
	{
		return NULL;
	}

	virtual class FSpotLight* GetSpotLight() 
	{
		return NULL;
	}

	virtual const class FSpotLight* GetSpotLight() const
	{
		return NULL;
	}

	virtual const class FMeshAreaLight* GetMeshAreaLight() const
	{
		return NULL;
	}

	/** Returns the number of direct photons to gather required by this light. */
	virtual INT GetNumDirectPhotons(FLOAT DirectPhotonDensity) const = 0;

	/**
	 * Tests whether the light affects the given bounding volume.
	 * @param Bounds - The bounding volume to test.
	 * @return True if the light affects the bounding volume
	 */
	virtual UBOOL AffectsBounds(const FBoxSphereBounds& Bounds) const;

	/**
	 * Computes the intensity of the direct lighting from this light on a specific point.
	 */
	virtual FLinearColor GetDirectIntensity(const FVector4& Point, UBOOL bCalculateForIndirectLighting) const;

	/** Returns an intensity scale based on the receiving point. */
	virtual FLOAT CustomAttenuation(const FVector4& Point, FRandomStream& RandomStream) const { return 1.0f; }

	/** Generates a direction sample from the light's domain */
	virtual void SampleDirection(FRandomStream& RandomStream, class FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const = 0;

	/** Gives the light an opportunity to precalculate information about the indirect path rays that will be used to generate new directions. */
	virtual void CachePathRays(const TArray<FIndirectPathRay>& IndirectPathRays) {}

	/** Generates a direction sample from the light based on the given rays */
	virtual void SampleDirection(
		const TArray<FIndirectPathRay>& IndirectPathRays, 
		FRandomStream& RandomStream, 
		FLightRay& SampleRay, 
		FLOAT& RayPDF,
		FLinearColor& Power) const = 0;

	/** Returns the light's radiant power. */
	virtual FLOAT Power() const = 0;

	/** Generates and caches samples on the light's surface. */
	virtual void CacheSurfaceSamples(INT BounceIndex, INT NumSamples, INT NumPenumbraSamples, FRandomStream& RandomStream);

	/** Retrieves the array of cached light surface samples. */
	const TArray<FLightSurfaceSample>& GetCachedSurfaceSamples(INT BounceIndex, UBOOL bPenumbra) const;

	/** Validates a surface sample given the position that sample is affecting.  The sample is unaffected by default. */
	virtual void ValidateSurfaceSample(const FVector4& Point, FLightSurfaceSample& Sample) const {}

	/** Gets a single position which represents the center of the area light source from the ReceivingPosition's point of view. */
	virtual FVector4 LightCenterPosition(const FVector4& ReceivingPosition) const { return Position; }

	/** Returns TRUE if all parts of the light are behind the surface being tested. */
	virtual UBOOL BehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal) const = 0;

	/** Gets a single direction to use for direct lighting that is representative of the whole area light. */
	virtual FVector4 GetDirectLightingDirection(const FVector4& Point, const FVector4& PointNormal) const = 0;

	/**
	 * Returns whether static lighting, aka lightmaps, is being used for primitive/ light
	 * interaction.
	 *
	 * @param bForceDirectLightMap	Whether primitive is set to force lightmaps
	 * @return TRUE if lightmaps/ static lighting is being used, FALSE otherwise
	 */
	UBOOL UseStaticLighting( UBOOL bForceDirectLightMap ) const
	{
		// use the flags that UE3 sent over to determine if we should use static lighting or not
		UBOOL Result = (LightFlags & GI_LIGHT_HASSTATICLIGHTING) && ((LightFlags & GI_LIGHT_USEDIRECTLIGHTMAP) || bForceDirectLightMap || !(LightFlags & GI_LIGHT_HASSTATICSHADOWING));

		return Result;
	}

protected:
	/** Cached samples of the light's surface, indexed first by bounce number, then by whether the shadow ray is a penumbra ray, then by sample index. */
	TArray<TArray<TArray<FLightSurfaceSample> > > CachedLightSurfaceSamples;

	/** Cached calculation of the light's indirect color, which is the base Color adjusted by the IndirectLightingScale and IndirectLightingSaturation */
	FLinearColor IndirectColor;

	/** Generates a sample on the light's surface. */
	virtual void SampleLightSurface(FRandomStream& RandomStream, FLightSurfaceSample& Sample) const = 0;
};


//----------------------------------------------------------------------------
//	Directional light class
//----------------------------------------------------------------------------
class FDirectionalLight : public FLight, public FDirectionalLightData
{
public:

	FLOAT IndirectDiskRadius;

	virtual void            Read( FILE* FileHandle, FScene& Scene);
	virtual void			Import( class FLightmassImporter& Importer );

	virtual class FDirectionalLight* GetDirectionalLight() 
	{
		return this;
	}

	virtual const class FDirectionalLight* GetDirectionalLight() const
	{
		return this;
	}

	void Initialize(
		const FBoxSphereBounds& InSceneBounds, 
		UBOOL bInEmitPhotonsOutsideImportanceVolume,
		const FBoxSphereBounds& InImportanceBounds, 
		FLOAT InIndirectDiskRadius, 
		INT InGridSize, 
		FLOAT InDirectPhotonDensity,
		FLOAT InOutsideImportanceVolumeDensity);

	/** Returns the number of direct photons to gather required by this light. */
	virtual INT GetNumDirectPhotons(FLOAT DirectPhotonDensity) const;

	/** Generates a direction sample from the light's domain */
	virtual void SampleDirection(FRandomStream& RandomStream, FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const;

	/** Gives the light an opportunity to precalculate information about the indirect path rays that will be used to generate new directions. */
	virtual void CachePathRays(const TArray<FIndirectPathRay>& IndirectPathRays);

	/** Generates a direction sample from the light based on the given rays */
	virtual void SampleDirection(
		const TArray<FIndirectPathRay>& IndirectPathRays, 
		FRandomStream& RandomStream, 
		FLightRay& SampleRay, 
		FLOAT& RayPDF,
		FLinearColor& Power) const;

	/** Returns the light's radiant power. */
	virtual FLOAT Power() const;

	/** Validates a surface sample given the position that sample is affecting. */
	virtual void ValidateSurfaceSample(const FVector4& Point, FLightSurfaceSample& Sample) const;

	/** Gets a single position which represents the center of the area light source from the ReceivingPosition's point of view. */
	virtual FVector4 LightCenterPosition(const FVector4& ReceivingPosition) const;

	/** Returns TRUE if all parts of the light are behind the surface being tested. */
	virtual UBOOL BehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal) const;

	/** Gets a single direction to use for direct lighting that is representative of the whole area light. */
	virtual FVector4 GetDirectLightingDirection(const FVector4& Point, const FVector4& PointNormal) const;

protected:

	/** Extent of PathRayGrid in the [-1, 1] space of the directional light's disk. */
	FLOAT GridExtent;

	/** Center of PathRayGrid in the [-1, 1] space of the directional light's disk. */
	FVector2D GridCenter;

	/** Size of PathRayGrid in each dimension. */
	INT GridSize;

	/** Grid of indices into IndirectPathRays that affect each cell. */
	TArray<TArray<INT>> PathRayGrid;

	/** Bounds of the scene that the directional light is affecting. */
	FBoxSphereBounds SceneBounds;

	UBOOL bEmitPhotonsOutsideImportanceVolume;

	/** Bounds of the importance volume in the scene.  If the radius is 0, there was no importance volume. */
	FBoxSphereBounds ImportanceBounds;

	/** Center of the importance volume in the [-1,1] space of the directional light's disk */
	FVector2D ImportanceDiskOrigin;

	/** Radius of the importance volume in the [-1,1] space of the directional light's disk */
	FLOAT LightSpaceImportanceDiskRadius;

	/** Density of photons to gather outside of the importance volume. */
	FLOAT OutsideImportanceVolumeDensity;

	/** Probability of generating a sample inside the importance volume. */
	FLOAT ImportanceBoundsSampleProbability;

	/** X axis of the directional light, which is unit length and orthogonal to the direction and YAxis. */
	FVector4 XAxis;

	/** Y axis of the directional light, which is unit length and orthogonal to the direction and XAxis. */
	FVector4 YAxis;

	/** Generates a sample on the light's surface. */
	virtual void SampleLightSurface(FRandomStream& RandomStream, FLightSurfaceSample& Sample) const;
};


//----------------------------------------------------------------------------
//	Point light class
//----------------------------------------------------------------------------
class FPointLight : public FLight, public FPointLightData
{
public:
	virtual void			Read( FILE* FileHandle, FScene& Scene);
	virtual void			Import( class FLightmassImporter& Importer );

	virtual class FPointLight* GetPointLight() 
	{
		return this;
	}

	virtual const class FPointLight* GetPointLight() const
	{
		return this;
	}

	void Initialize(FLOAT InIndirectPhotonEmitConeAngle);

	/** Returns the number of direct photons to gather required by this light. */
	virtual INT GetNumDirectPhotons(FLOAT DirectPhotonDensity) const;

	/**
	 * Tests whether the light affects the given bounding volume.
	 * @param Bounds - The bounding volume to test.
	 * @return True if the light affects the bounding volume
	 */
	virtual UBOOL AffectsBounds(const FBoxSphereBounds& Bounds) const;

	/**
	 * Computes the intensity of the direct lighting from this light on a specific point.
	 */
	virtual FLinearColor GetDirectIntensity(const FVector4& Point, UBOOL bCalculateForIndirectLighting) const;

	/** Returns an intensity scale based on the receiving point. */
	virtual FLOAT CustomAttenuation(const FVector4& Point, FRandomStream& RandomStream) const;

	/** Generates a direction sample from the light's domain */
	virtual void SampleDirection(FRandomStream& RandomStream, FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const;

	/** Generates a direction sample from the light based on the given rays */
	virtual void SampleDirection(
		const TArray<FIndirectPathRay>& IndirectPathRays, 
		FRandomStream& RandomStream, 
		FLightRay& SampleRay, 
		FLOAT& RayPDF,
		FLinearColor& Power) const;

	/** Validates a surface sample given the position that sample is affecting. */
	virtual void ValidateSurfaceSample(const FVector4& Point, FLightSurfaceSample& Sample) const;

	/** Returns the light's radiant power. */
	virtual FLOAT Power() const;

	/** Returns TRUE if all parts of the light are behind the surface being tested. */
	virtual UBOOL BehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal) const;

	/** Gets a single direction to use for direct lighting that is representative of the whole area light. */
	virtual FVector4 GetDirectLightingDirection(const FVector4& Point, const FVector4& PointNormal) const;

protected:

	FLOAT CosIndirectPhotonEmitConeAngle;

	/** Generates a sample on the light's surface. */
	virtual void SampleLightSurface(FRandomStream& RandomStream, FLightSurfaceSample& Sample) const;
};


//----------------------------------------------------------------------------
//	Spot light class
//----------------------------------------------------------------------------
class FSpotLight : public FPointLight, public FSpotLightData
{
public:

	virtual class FSpotLight* GetSpotLight() 
	{
		return this;
	}

	virtual const class FSpotLight* GetSpotLight() const
	{
		return this;
	}

	virtual void			Read( FILE* FileHandle, FScene& Scene);
	virtual void			Import( class FLightmassImporter& Importer );

	/**
	 * Tests whether the light affects the given bounding volume.
	 * @param Bounds - The bounding volume to test.
	 * @return True if the light affects the bounding volume
	 */
	virtual UBOOL AffectsBounds(const FBoxSphereBounds& Bounds) const;

	/**
	 * Computes the intensity of the direct lighting from this light on a specific point.
	 */
	virtual FLinearColor GetDirectIntensity(const FVector4& Point, UBOOL bCalculateForIndirectLighting) const;

	/** Returns the number of direct photons to gather required by this light. */
	virtual INT GetNumDirectPhotons(FLOAT DirectPhotonDensity) const;

	/** Generates a direction sample from the light's domain */
	virtual void SampleDirection(FRandomStream& RandomStream, FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const;
};


//----------------------------------------------------------------------------
//	Sky light class
//----------------------------------------------------------------------------
class FSkyLight : public FLight, public FSkyLightData
{
public:
	virtual void			Import( class FLightmassImporter& Importer );

	/**
	 * @return 'this' if the light is a skylight, NULL otherwise 
	 */
	virtual const class FSkyLight* GetSkyLight() const
	{
		return this;
	}
	virtual class FSkyLight* GetSkyLight()
	{
		return this;
	}

	/** Returns the number of direct photons to gather required by this light. */
	virtual INT GetNumDirectPhotons(FLOAT DirectPhotonDensity) const
	{ checkf(0, TEXT("GetNumDirectPhotons is not supported for skylights")); return 0; }

	/** Generates a direction sample from the light's domain */
	virtual void SampleDirection(FRandomStream& RandomStream, class FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const
	{ checkf(0, TEXT("SampleDirection is not supported for skylights")); }

	/** Generates a direction sample from the light based on the given rays */
	virtual void SampleDirection(
		const TArray<FIndirectPathRay>& IndirectPathRays, 
		FRandomStream& RandomStream, 
		FLightRay& SampleRay, 
		FLOAT& RayPDF,
		FLinearColor& Power) const
	{ checkf(0, TEXT("SampleDirection is not supported for skylights")); }

	/** Returns the light's radiant power. */
	virtual FLOAT Power() const
	{ checkf(0, TEXT("Power is not supported for skylights")); return 0; }

	/** Returns TRUE if all parts of the light are behind the surface being tested. */
	virtual UBOOL BehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal) const { return FALSE; }

	/** Gets a single direction to use for direct lighting that is representative of the whole area light. */
	virtual FVector4 GetDirectLightingDirection(const FVector4& Point, const FVector4& PointNormal) const 
	{ checkf(0, TEXT("GetDirectLightingDirection is not supported for skylights")); return FVector4(); }

protected:

	/** Generates a sample on the light's surface. */
	virtual void SampleLightSurface(FRandomStream& RandomStream, FLightSurfaceSample& Sample) const
	{ checkf(0, TEXT("SampleLightSurface is not supported for skylights")); }
};

class FMeshLightPrimitiveCorner
{
public:
	/** World space corner position, not necessarily coplanar with the other corners. */
	FVector4 WorldPosition;
	/** Coordinate in texture space corresponding to the position this corner is storing. */
	FIntPoint FurthestCoordinates;
};

/** The atomic shape used to represent an area light's shape. */
class FMeshLightPrimitive
{
public:
	FMeshLightPrimitiveCorner Corners[NumTexelCorners];
	/** Average normal of the sub primitives making up this simplified primitive. */
	FVector4 SurfaceNormal;
	/** Radiant flux of this primitive */
	FLinearColor Power;
	/** Surface area of this primitive */
	FLOAT SurfaceArea;
	/** Number of original primitives that were combined into this simplified primitive. */
	INT NumSubPrimitives;

	void AddSubPrimitive(const struct FTexelToCorners& TexelToCorners, const FIntPoint& Coordinates, const FLinearColor& InTexelPower, FLOAT NormalOffset);
	void Finalize();
};

//----------------------------------------------------------------------------
//	Mesh Area Light class
//----------------------------------------------------------------------------
class FMeshAreaLight : public FLight
{
public:

	FMeshAreaLight(EInit)
	{
		// Initialize base members since the default constructor does nothing
		// All the other light types are always serialized in so this is only needed for FMeshAreaLight
		// Note: this will stomp on derived class data members, but it's assumed that 0 is a valid default.
		appMemzero((FLightData*)this, sizeof(FLightData));
	}

	virtual const class FMeshAreaLight* GetMeshAreaLight() const
	{
		return this;
	}

	void Initialize(FLOAT InIndirectPhotonEmitConeAngle, const FBoxSphereBounds& InImportanceBounds);

	/** Returns the number of direct photons to gather required by this light. */
	virtual INT GetNumDirectPhotons(FLOAT DirectPhotonDensity) const;

	/** Initializes the mesh area light with primitives */
	void SetPrimitives(
		const TArray<FMeshLightPrimitive>& InPrimitives, 
		FLOAT EmissiveLightFalloffExponent, 
		FLOAT EmissiveLightExplicitInfluenceRadius,
		INT InMeshAreaLightGridSize,
		INT InLevelId);

	INT GetNumPrimitives() const 
	{ 
		INT NumTotalPrimitives = 0;
		for (INT PrimitiveIndex = 0; PrimitiveIndex < Primitives.Num(); PrimitiveIndex++)
		{
			NumTotalPrimitives += Primitives(PrimitiveIndex).NumSubPrimitives;
		}
		return NumTotalPrimitives; 
	}

	INT GetNumSimplifiedPrimitives() const { return Primitives.Num(); }

	/**
	* Tests whether the light affects the given bounding volume.
	* @param Bounds - The bounding volume to test.
	* @return True if the light affects the bounding volume
	*/
	virtual UBOOL AffectsBounds(const FBoxSphereBounds& Bounds) const;

	/**
	 * Computes the intensity of the direct lighting from this light on a specific point.
	 */
	virtual FLinearColor GetDirectIntensity(const FVector4& Point, UBOOL bCalculateForIndirectLighting) const;

	/** Returns an intensity scale based on the receiving point. */
	virtual FLOAT CustomAttenuation(const FVector4& Point, FRandomStream& RandomStream) const;

	/** Generates a direction sample from the light's domain */
	virtual void SampleDirection(FRandomStream& RandomStream, FLightRay& SampleRay, FVector4& LightSourceNormal, FVector2D& LightSurfacePosition, FLOAT& RayPDF, FLinearColor& Power) const;

	/** Generates a direction sample from the light based on the given rays */
	virtual void SampleDirection(
		const TArray<FIndirectPathRay>& IndirectPathRays, 
		FRandomStream& RandomStream, 
		FLightRay& SampleRay, 
		FLOAT& RayPDF,
		FLinearColor& Power) const;

	/** Validates a surface sample given the position that sample is affecting. */
	void ValidateSurfaceSample(const FVector4& Point, FLightSurfaceSample& Sample) const;

	/** Returns the light's radiant power. */
	virtual FLOAT Power() const;

	/** Returns TRUE if all parts of the light are behind the surface being tested. */
	virtual UBOOL BehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal) const;

	/** Gets a single direction to use for direct lighting that is representative of the whole area light. */
	virtual FVector4 GetDirectLightingDirection(const FVector4& Point, const FVector4& PointNormal) const;

protected:
	/** Radiant flux of all the primitives */
	FLinearColor TotalPower;

	/** Accumulated surface area of all primitives */
	FLOAT TotalSurfaceArea;

	/** Generated radius of the light's influence */
	FLOAT InfluenceRadius;

	/** Bounds of the light's primitives */
	FBoxSphereBounds SourceBounds;

	/** Falloff of the attenuation function */
	FLOAT FalloffExponent;

	/** Cosine of the angle about an indirect path in which to emit indirect photons */
	FLOAT CosIndirectPhotonEmitConeAngle;

	/** All the primitives that define this light's shape */
	TArray<FMeshLightPrimitive> Primitives;

	/** Size of the data stored in CachedPrimitiveNormals in each dimension. */
	INT MeshAreaLightGridSize;

	INT LevelId;

	/** Grid of arrays of primitive normals, used to accelerate PDF calculation once a sample is generated. */
	TArray<TArray<FVector4> > CachedPrimitiveNormals;

	/** Entries in CachedPrimitiveNormals that have one or more cached normals. */
	TArray<FIntPoint> OccupiedCachedPrimitiveNormalCells;

	/** Bounds of the importance volume in the scene.  If the radius is 0, there was no importance volume. */
	FBoxSphereBounds ImportanceBounds;

	/** Probability of selecting each primitive when sampling. */
	TArray<FLOAT> PrimitivePDFs;

	/** Stores the cumulative distribution function of PrimitivePDFs */
	TArray<FLOAT> PrimitiveCDFs;

	/** Stores the integral of PrimitivePDFs */
	FLOAT UnnormalizedIntegral;

	/** Generates a sample on the light's surface. */
	virtual void SampleLightSurface(FRandomStream& RandomStream, FLightSurfaceSample& Sample) const;

	friend class FLightmassSolverExporter;
};

/** Volume that determines where to place visibility cells. */
class FPrecomputedVisibilityVolume
{
public:
	FBox Bounds;
	TArray<FPlane> Planes;
};

/** Volume that overrides visibility for a set of Ids. */
class FPrecomputedVisibilityOverrideVolume
{
public:
	FBox Bounds;
	TArray<INT> OverrideVisiblityIds;
	TArray<INT> OverrideInvisiblityIds;
};

//----------------------------------------------------------------------------
//	Scene class
//----------------------------------------------------------------------------
class FScene : public FSceneFileHeader
{
public:
	FScene();
	virtual BOOL			Load( const FString& SceneFile);
	virtual BOOL			Import( class FLightmassImporter& Importer );
	FBoxSphereBounds		GetImportanceBounds() const;

	FBox ImportanceBoundingBox;
	TArray<FBox> ImportanceVolumes;
	TArray<FBox> CharacterIndirectDetailVolumes;
	TArray<FPrecomputedVisibilityVolume> PrecomputedVisibilityVolumes;
	TArray<FPrecomputedVisibilityOverrideVolume> PrecomputedVisibilityOverrideVolumes;
	TArray<FVector4> CameraTrackPositions;

	TArray<FDirectionalLight>	DirectionalLights;									// direction lights
	TArray<FPointLight>			PointLights;										// point lights
	TArray<FSpotLight>			SpotLights;											// spot lights
	TArray<FSkyLight>			SkyLights;											// sky lights


	TArray<FStaticMesh>									StaticMeshs;				// 静态模型(LORD)
	TArray<FMaterial>									Materials;					// 静态模型材质

	TArray<FStaticMeshStaticLightingMesh>				StaticMeshInstances;		/** Represents the triangles of one LOD of a static mesh primitive to the static lighting system. */
	TArray<FFluidSurfaceStaticLightingMesh>				FluidMeshInstances;
	TArray<FLandscapeStaticLightingMesh>				LandscapeMeshInstances;
	TArray<FSpeedTreeStaticLightingMesh>				SpeedTreeMeshInstances;
	TArray<FBSPSurfaceStaticLighting>					BspMappings;
	TArray<FTerrainComponentStaticLighting>				TerrainMappings;
	TArray<FStaticMeshStaticLightingTextureMapping>		TextureLightingMappings;	// static mesh primitive with texture mapped static lighting.
	TArray<FStaticMeshStaticLightingVertexMapping>		VertexLightingMappings;		// static mesh primitive with vertex mapped static lighting
	TArray<FFluidSurfaceStaticLightingTextureMapping>	FluidMappings;
	TArray<FLandscapeStaticLightingTextureMapping>		LandscapeMappings;
	TArray<FSpeedTreeStaticLightingMapping>				SpeedTreeMappings;

	TArray<FGuid> VisibilityBucketGuids;

	/** The mapping whose texel is selected in UE3 and is being debugged. */
	const class FStaticLightingMapping* DebugMapping;

	const FLight* FindLightByGuid(const FGuid& Guid) const;

	/** Returns TRUE if the specified position is inside any of the importance volumes. */
	UBOOL IsPointInImportanceVolume(const FVector4& Position) const;

	/** Returns TRUE if the specified position is inside any of the visibility volumes. */
	UBOOL IsPointInVisibilityVolume(const FVector4& Position) const;

	/** Returns accumulated bounds from all the visibility volumes. */
	FBox GetVisibilityVolumeBounds() const;

	// 根据GUID获取材质
	FMaterial* GetMaterial( FGuid Guid);

	// 根据GUID获取光源
	FLight* GetLights( FGuid Guid);

	// 根据GUID获取模型
	FStaticMesh* GetStaticMesh( FGuid Guid);

	// 根据GUID获取MeshInstance
	FStaticMeshStaticLightingMesh* GetStaticMeshInstance( FGuid Guid);

public:
	/** Reads a TArray of objects */
	template <class ObjType>
	UBOOL ReadObjectArray( TArray<ObjType>& Array, INT Count, FILE* FileHandle)
	{
		Array.Empty( Count);
		for ( INT Index=0; Index < Count; ++Index )
		{
			ObjType* Item = new(Array)ObjType;
			Item->Read( FileHandle, *this);
		}
		return TRUE;
	}

	/** Imports a TArray of simple elements in one bulk read. */
	template <class ArrayType>
	static UBOOL ReadArray( ArrayType& Array, INT Count, FILE* FileHandle)
	{
		Array.Empty( Count );
		Array.Add( Count );

		fread( Array.GetData(), Count*sizeof(ArrayType::ElementType), 1, FileHandle);

		return TRUE;
	}

private:
	/** Searches through all mapping arrays for the mapping matching FindGuid. */
	const FStaticLightingMapping* FindMappingByGuid(FGuid FindGuid) const;
	
	/** Applies GeneralSettings.StaticLightingLevelScale to all scale dependent settings. */
	void ApplyStaticLightingScale();
};


} // namespace Lightmass
