/*=============================================================================
	LightingSystem.h: Private static lighting system definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "CPUSolver.h"
#include "ImportExport.h"
#include "Exporter.h"
#include "Cache.h"

namespace Lightmass
{

/** Whether to allow static lighting stats that may affect the system's performance. */
#define ALLOW_STATIC_LIGHTING_STATS 1

/** Whether to make Lightmass do pretty much no processing at all (NOP). */
#define LIGHTMASS_NOPROCESSING 0

#if ALLOW_STATIC_LIGHTING_STATS
	#define LIGHTINGSTAT(x) x
#else
	#define LIGHTINGSTAT(x)
#endif

/** The number of coefficients that are gathered for each FGatheredLightSample. */ 
static const INT LM_NUM_GATHERED_LIGHTMAP_COEF = 4;
/** The number of directional coefficients gathered for each FGatheredLightSample. */ 
static const INT LM_NUM_GATHERED_DIRECTIONAL_LIGHTMAP_COEF = 3;
/** The index at which simple coefficients are stored in any array containing all NUM_GATHERED_LIGHTMAP_COEF coefficients. */ 
static const INT LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX = 3;

/** 
 * The three orthonormal bases used with directional static lighting, and then the surface normal in tangent space.
 * This must match LightMapBasis in UE3 BasePassPixelShader.usf.
 */
static const FVector4 LightMapBasis[] =
{
	FVector4(	0.0f,						appSqrt(6.0f) / 3.0f,			1.0f / appSqrt(3.0f),	0.0f),
	FVector4(	-1.0f / appSqrt(2.0f),		-1.0f / appSqrt(6.0f),			1.0f / appSqrt(3.0f),	0.0f),
	FVector4(	+1.0f / appSqrt(2.0f),		-1.0f / appSqrt(6.0f),			1.0f / appSqrt(3.0f),	0.0f),
	FVector4(	0.0f,						0.0f,							1.0f,					0.0f)
};

checkAtCompileTime(ARRAY_COUNT(LightMapBasis) == LM_NUM_GATHERED_LIGHTMAP_COEF, LightmapCoefficientCountMustMatchBasisCount);

/** 
 * The light incident for a point on a surface, in the representation used when gathering lighting. 
 * This representation is additive, and allows for accumulating lighting contributions in-place. 
 */
class FGatheredLightSample
{
public:

	/** 
	 * The lighting coefficients, colored. 
	 * The first NUM_GATHERED_DIRECTIONAL_LIGHTMAP_COEF coefficients store the colored incident lighting along each lightmap basis axis.
     * The last coefficient, SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX, stores the simple lighting, which is incident lighting along the vertex normal.
	 */
	FLOAT Coefficients[LM_NUM_GATHERED_LIGHTMAP_COEF][3];

	/** True if this sample maps to a valid point on a triangle.  This is only meaningful for texture lightmaps. */
	UBOOL bIsMapped;

	/** Initialization constructor. */
	FGatheredLightSample()
	{
		bIsMapped = FALSE;
		appMemzero(Coefficients,sizeof(Coefficients));
	}

	FGatheredLightSample(EInit)
	{
		bIsMapped = FALSE;
		appMemzero(Coefficients,sizeof(Coefficients));
	}

	/**
	 * Constructs a light sample representing a point light.
	 * @param Color - The color/intensity of the light at the sample point.
	 * @param Direction - The direction toward the light at the sample point.
	 */
	static FGatheredLightSample PointLight(const FLinearColor& Color,const FVector4& Direction,const FVector4& TangentNormal = LightMapBasis[LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX]);

	/**
	 * Constructs a light sample representing a sky light.
	 * @param UpperColor - The color/intensity of the sky light's upper hemisphere.
	 * @param LowerColor - The color/intensity of the sky light's lower hemisphere.
	 * @param WorldZ - The world's +Z axis in tangent space.
	 */
	static FGatheredLightSample SkyLight(const FLinearColor& UpperColor,const FLinearColor& LowerColor,const FVector4& WorldZ,const FVector4& TangentNormal = LightMapBasis[LM_SIMPLE_GATHERED_LIGHTMAP_COEF_INDEX]);

	/**
	 * Adds a weighted light sample to this light sample.
	 * @param OtherSample - The sample to add.
	 * @param Weight - The weight to multiply the other sample by before addition.
	 * @param bSimpleLightMapsOnly - True to only add contribution to the simple light map channel
	 */
	void AddWeighted(const FGatheredLightSample& OtherSample,FLOAT Weight, const UBOOL bSimpleLightMapsOnly = FALSE);

	UBOOL AreFloatsValid() const;

	FGatheredLightSample operator*(FLOAT Scalar) const
	{
		FGatheredLightSample Result;
		Result.bIsMapped = bIsMapped;
		for(INT CoefficientIndex = 0;CoefficientIndex < LM_NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
		{
			for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
			{
				Result.Coefficients[CoefficientIndex][ColorIndex] = Coefficients[CoefficientIndex][ColorIndex] * Scalar;
			}
		}
		return Result;
	}

	FGatheredLightSample operator+(const FGatheredLightSample& SampleB) const
	{
		FGatheredLightSample Result;
		Result.bIsMapped = bIsMapped;
		for(INT CoefficientIndex = 0;CoefficientIndex < LM_NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
		{
			for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
			{
				Result.Coefficients[CoefficientIndex][ColorIndex] = Coefficients[CoefficientIndex][ColorIndex] + SampleB.Coefficients[CoefficientIndex][ColorIndex];
			}
		}
		return Result;
	}

	FGatheredLightSample operator/(const FGatheredLightSample& SampleB) const
	{
		FGatheredLightSample Result;
		Result.bIsMapped = bIsMapped;
		for(INT CoefficientIndex = 0;CoefficientIndex < LM_NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
		{
			for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
			{
				Result.Coefficients[CoefficientIndex][ColorIndex] = Coefficients[CoefficientIndex][ColorIndex] / SampleB.Coefficients[CoefficientIndex][ColorIndex];
			}
		}
		return Result;
	}

	/** Converts an FGatheredLightSample into a FLightSample. */
	FLightSample ConvertToLightSample(UBOOL bDebugThisSample) const;

	inline FLinearColor ToLinearColor(INT CoefficientIndex) const
	{
		return FLinearColor(Coefficients[CoefficientIndex][0], Coefficients[CoefficientIndex][1], Coefficients[CoefficientIndex][2], 0.0f);
	}

	inline void FromLinearColor(INT CoefficientIndex, const FLinearColor& Color)
	{
		Coefficients[CoefficientIndex][0] = Color.R;
		Coefficients[CoefficientIndex][1] = Color.G;
		Coefficients[CoefficientIndex][2] = Color.B;
	}

	/** 
	 * Redistributes incident light from all directions into the dominant direction based on RedistributeFactor, 
	 * Which effectively increases per-pixel normal influence while introducing compression artifacts.
	 */
	FGatheredLightSample RedistributeToDominantDirection(FLOAT RedistributeFactor) const;
};

/** LightMap data 1D */
class FGatheredLightMapData1D
{
public:
	/** The lights which this light-map stores. */
	TArray<const FLight*> Lights;

	FGatheredLightMapData1D(INT Size)
	{
		Data.Empty(Size);
		Data.AddZeroed(Size);
	}

	// Accessors.
	const FGatheredLightSample& operator()(UINT Index) const { return Data(Index); }
	FGatheredLightSample& operator()(UINT Index) { return Data(Index); }
	INT GetSize() const { return Data.Num(); }

	void AddLight(const FLight* NewLight)
	{
		Lights.AddUniqueItem(NewLight);
	}

	FLightMapData1D* ConvertToLightmap1D(UBOOL bDebugThisMapping, INT DebugVertexIndex) const;

private:
	TArray<FGatheredLightSample> Data;
};

/**
 * The raw data which is used to construct a 2D light-map.
 */
class FGatheredLightMapData2D
{
public:

	/** The width of the light-map. */
	UINT SizeX;
	/** The height of the light-map. */
	UINT SizeY;

	/** The lights which this light-map stores. */
	TArray<const FLight*> Lights;

	FGatheredLightMapData2D(UINT InSizeX, UINT InSizeY) :
		SizeX(InSizeX),
		SizeY(InSizeY)
	{
		Data.Empty(SizeX * SizeY);
		Data.AddZeroed(SizeX * SizeY);
	}

	// Accessors.
	const FGatheredLightSample& operator()(UINT X,UINT Y) const { return Data(SizeX * Y + X); }
	FGatheredLightSample& operator()(UINT X,UINT Y) { return Data(SizeX * Y + X); }
	UINT GetSizeX() const { return SizeX; }
	UINT GetSizeY() const { return SizeY; }

	void Empty()
	{
		Data.Empty();
		SizeX = SizeY = 0;
		Lights.Empty();
	}

	void AddLight(const FLight* NewLight)
	{
		Lights.AddUniqueItem(NewLight);
	}

	FLightMapData2D* ConvertToLightmap2D(UBOOL bDebugThisMapping, INT PaddedDebugX, INT PaddedDebugY) const;

private:
	TArray<FGatheredLightSample> Data;
};

/** The lighting information gathered for one final gather sample */
class FFinalGatherSample : public FGatheredLightSample
{
public:

	/** Occlusion factor of the sample, 0 is completely unoccluded, 1 is completely occluded. */
	FLOAT Occlusion;

	/** Initialization constructor. */
	FFinalGatherSample() : 
		FGatheredLightSample(),
		Occlusion(0.0f)
	{}

	FFinalGatherSample(EInit) : 
		FGatheredLightSample(E_Init),
		Occlusion(0.0f)
	{}

	/**
	 * Adds a weighted light sample to this light sample.
	 * @param OtherSample - The sample to add.
	 * @param Weight - The weight to multiply the other sample by before addition.
	 */
	void AddWeighted(const FFinalGatherSample& OtherSample, FLOAT Weight);

	/**
	 * Adds a weighted light sample to this light sample.
	 * @param OtherSample - The sample to add.
	 * @param Weight - The weight to multiply the other sample by before addition.
	 */
	inline void AddWeighted(const FGatheredLightSample& OtherSample, FLOAT Weight)
	{
		FGatheredLightSample::AddWeighted(OtherSample, Weight);
	}

	inline void SetOcclusion(FLOAT InOcclusion)
	{
		Occlusion = InOcclusion;
	}

	inline void AddIncomingRadiance(const FLinearColor& IncomingRadiance, FLOAT Weight, const FVector4& TangentSpaceDirection, const FVector4& WorldSpaceDirection)
	{
		AddWeighted(FGatheredLightSample::PointLight(IncomingRadiance, TangentSpaceDirection), Weight);
	}

	inline void AddIncomingEnvRadiance(const FLinearColor& IncomingRadiance, FLOAT Weight, const FVector4& TangentSpaceDirection, const FVector4& WorldSpaceDirection)
	{
		AddIncomingRadiance(IncomingRadiance, Weight, TangentSpaceDirection, WorldSpaceDirection);
	}

	UBOOL AreFloatsValid() const;

	FFinalGatherSample operator*(FLOAT Scalar) const
	{
		FFinalGatherSample Result;
		(FGatheredLightSample&)Result = (const FGatheredLightSample&)(*this) * Scalar;
		Result.Occlusion = Occlusion * Scalar;
		return Result;
	}

	FFinalGatherSample operator+(const FFinalGatherSample& SampleB) const
	{
		FFinalGatherSample Result;
		(FGatheredLightSample&)Result = (const FGatheredLightSample&)(*this) + (const FGatheredLightSample&)SampleB;
		Result.Occlusion = Occlusion + SampleB.Occlusion;
		return Result;
	}
};

struct FTexelCorner
{
	FVector4 WorldPosition;
};

/** Information about a texel's corners */
struct FTexelToCorners
{ 
	/** The position of each corner */
	FTexelCorner Corners[NumTexelCorners];
	/** The tangent basis of the last valid corner to be rasterized */
	FVector4 WorldTangentX;
	FVector4 WorldTangentY;
	FVector4 WorldTangentZ;
	/** Whether each corner lies on the mesh */
	UBOOL bValid[NumTexelCorners];
};

/** Map from texel to the corners of that texel. */
class FTexelToCornersMap
{
public:

	/** Initialization constructor. */
	FTexelToCornersMap(INT InSizeX,INT InSizeY):
		Data(InSizeX * InSizeY),
		SizeX(InSizeX),
		SizeY(InSizeY)
	{
		// Clear the map to zero.
		for(INT Y = 0;Y < SizeY;Y++)
		{
			for(INT X = 0;X < SizeX;X++)
			{
				appMemzero(&(*this)(X,Y),sizeof(FTexelToCorners));
			}
		}
	}

	// Accessors.
	FTexelToCorners& operator()(INT X,INT Y)
	{
		const UINT TexelIndex = Y * SizeX + X;
		return Data(TexelIndex);
	}
	const FTexelToCorners& operator()(INT X,INT Y) const
	{
		const INT TexelIndex = Y * SizeX + X;
		return Data(TexelIndex);
	}

	INT GetSizeX() const { return SizeX; }
	INT GetSizeY() const { return SizeY; }
	void Empty() { Data.Empty(); }

private:

	/** The mapping data. */
	TChunkedArray<FTexelToCorners> Data;

	/** The width of the mapping data. */
	INT SizeX;

	/** The height of the mapping data. */
	INT SizeY;
};

/** A particle representing the distribution of a light's radiant power. */
class FPhoton
{
private:

	/** Position that the photon was deposited at in XYZ, and Id in W for debugging. */
	FVector4 PositionAndId;

	/** Direction the photon came from in XYZ, and distance that the photon traveled along its last path before being deposited in W. */
	FVector4 IncidentDirectionAndDistance;

	/** Normal of the surface the photon was deposited on in XYZ, and fraction of the originating light's power that this photon represents in W. */
	FVector4 SurfaceNormalAndPower;

public:

	FPhoton(INT InId, const FVector4& InPosition, FLOAT InDistance, const FVector4& InIncidentDirection, const FVector4& InSurfaceNormal, const FLinearColor& InPower)
	{
		PositionAndId = FVector4(InPosition, *(FLOAT*)&InId);
		IncidentDirectionAndDistance = FVector4(InIncidentDirection, InDistance);
		checkSlow(InPower.AreFloatsValid());
		const FColor PowerRGBE = InPower.ToRGBE();
		SurfaceNormalAndPower = FVector4(InSurfaceNormal, *(FLOAT*)&PowerRGBE);
	}

	FORCEINLINE INT GetId() const
	{
		return *(INT*)&PositionAndId.W;
	}

	FORCEINLINE FVector4 GetPosition() const
	{
		return FVector4(PositionAndId, 0.0f);
	}

	FORCEINLINE FVector4 GetIncidentDirection() const
	{
		return FVector4(IncidentDirectionAndDistance, 0.0f);
	}

	FORCEINLINE FLOAT GetDistance() const
	{
		return IncidentDirectionAndDistance.W;
	}

	FORCEINLINE FVector4 GetSurfaceNormal() const
	{
		return FVector4(SurfaceNormalAndPower, 0.0f);
	}

	FORCEINLINE FLinearColor GetPower() const
	{
		const FColor PowerRGBE = *(FColor*)&SurfaceNormalAndPower.W;
		const FLinearColor OutPower = PowerRGBE.FromRGBE();
		checkSlow(OutPower.AreFloatsValid());
		return OutPower;
	}
};

/** An octree element that contains a photon */
struct FPhotonElement
{
	/** Stores a photon by value so we can discard the original array and avoid a level of indirection */
	const FPhoton Photon;

	/** Initialization constructor. */
	FPhotonElement(const FPhoton& InPhoton) :
		Photon(InPhoton)
	{}
};

typedef TOctree<FPhotonElement,struct FPhotonMapOctreeSemantics> FPhotonOctree;

/** Octree semantic definitions. */
struct FPhotonMapOctreeSemantics
{
	//@todo - evaluate different performance/memory tradeoffs with these
	enum { MaxElementsPerLeaf = 16 };
	enum { MaxNodeDepth = 12 };
	enum { LoosenessDenominator = 16 };

	// Using the default heap allocator instead of an inline allocator to reduce memory usage
	typedef FDefaultAllocator ElementAllocator;

	static FBoxCenterAndExtent GetBoundingBox(const FPhotonElement& PhotonElement)
	{
		return FBoxCenterAndExtent(PhotonElement.Photon.GetPosition(), FVector4(0,0,0));
	}
};

/** A photon which stores a precalculated irradiance estimate. */
class FIrradiancePhoton : public FIrradiancePhotonData
{
public:

	FIrradiancePhoton(const FVector4& InPosition, const FVector4& InSurfaceNormal, UBOOL bInHasContributionFromDirectPhotons)
	{
		PositionAndDirectContribution = FVector4(InPosition, bInHasContributionFromDirectPhotons);
		SurfaceNormalAndIrradiance = FVector4(InSurfaceNormal, 0.0f);
	}

	FORCEINLINE UBOOL HasDirectContribution() const
	{
		return PositionAndDirectContribution.W > 0.0f;
	}

	FORCEINLINE void SetHasDirectContribution() 
	{
		PositionAndDirectContribution.W = TRUE;
	}

	FORCEINLINE void SetUsed()
	{
		SurfaceNormalAndIrradiance.W = 1.0f;
	}

	FORCEINLINE UBOOL IsUsed() const
	{
		return SurfaceNormalAndIrradiance.W > 0.0f;
	}

	FORCEINLINE void SetIrradiance(FLinearColor InIrradiance)
	{
		checkSlow(InIrradiance.AreFloatsValid());
		const FColor IrradianceRGBE = InIrradiance.ToRGBE();
		SurfaceNormalAndIrradiance.W = *(FLOAT*)&IrradianceRGBE;
	}

	FORCEINLINE FLinearColor GetIrradiance() const
	{
		const FColor IrradianceRGBE = *(FColor*)&SurfaceNormalAndIrradiance.W;
		const FLinearColor OutIrradiance = IrradianceRGBE.FromRGBE();
		checkSlow(OutIrradiance.AreFloatsValid());
		return OutIrradiance;
	}

	FORCEINLINE FVector4 GetPosition() const
	{
		return FVector4(PositionAndDirectContribution, 0.0f);
	}

	FORCEINLINE FVector4 GetSurfaceNormal() const
	{
		return FVector4(SurfaceNormalAndIrradiance, 0.0f);
	}
};

/** An octree element that contains an irradiance photon */
struct FIrradiancePhotonElement
{
public:
	/** Initialization constructor. */
	FIrradiancePhotonElement(const INT InPhotonIndex, TArray<FIrradiancePhoton>& InPhotonArray) :
		PhotonIndex(InPhotonIndex),
		PhotonArray(InPhotonArray)
	{}

	FIrradiancePhoton& GetPhoton() { return PhotonArray(PhotonIndex); }
	const FIrradiancePhoton& GetPhoton() const  { return PhotonArray(PhotonIndex); }

private:
	INT PhotonIndex;
	TArray<FIrradiancePhoton>& PhotonArray;
};

typedef TOctree<FIrradiancePhotonElement,struct FIrradiancePhotonMapOctreeSemantics> FIrradiancePhotonOctree;

/** Octree semantic definitions. */
struct FIrradiancePhotonMapOctreeSemantics
{
	//@todo - evaluate different performance/memory tradeoffs with these
	enum { MaxElementsPerLeaf = 32 };
	enum { MaxNodeDepth = 12 };
	enum { LoosenessDenominator = 16 };

	typedef FDefaultAllocator ElementAllocator;

	static FBoxCenterAndExtent GetBoundingBox(const FIrradiancePhotonElement& PhotonElement)
	{
		return FBoxCenterAndExtent(PhotonElement.GetPhoton().GetPosition(), FVector4(0,0,0));
	}
};

/** A lighting sample in world space storing incident radiance from a whole sphere of directions. */
class FVolumeLightingSample : public FVolumeLightingSampleData
{
public:
	FVolumeLightingSample(const FVector4& InPositionAndRadius)
	{
		PositionAndRadius = InPositionAndRadius;
		IndirectDirection = FVector4(0,0,0);
		EnvironmentDirection = FVector4(0,0,0);
		IndirectRadiance = FColor(0,0,0);
		EnvironmentRadiance = FColor(0,0,0);
		AmbientRadiance = FColor(0,0,0);
		bShadowedFromDominantLights = 0;
	}
	inline FVector4 GetPosition() const
	{
		return FVector4(PositionAndRadius, 0.0f);
	}
	inline FLOAT GetRadius() const
	{
		return PositionAndRadius.W;
	}
	/** Constructs an SH environment from this lighting sample. */
	void ToSHVector(FSHVectorRGB& SHVector) const;
};

struct FVolumeSampleInterpolationElement
{
	const INT SampleIndex;
	const TArray<FVolumeLightingSample>& VolumeSamples;

	/** Initialization constructor. */
	FVolumeSampleInterpolationElement(const INT InSampleIndex, const TArray<FVolumeLightingSample>& InVolumeSamples) :
		SampleIndex(InSampleIndex),
		VolumeSamples(InVolumeSamples)
	{}
};

typedef TOctree<FVolumeSampleInterpolationElement,struct FVolumeLightingInterpolationOctreeSemantics> FVolumeLightingInterpolationOctree;

/** Octree semantic definitions. */
struct FVolumeLightingInterpolationOctreeSemantics
{
	//@todo - evaluate different performance/memory tradeoffs with these
	enum { MaxElementsPerLeaf = 4 };
	enum { MaxNodeDepth = 12 };
	enum { LoosenessDenominator = 16 };

	typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

	static FBoxCenterAndExtent GetBoundingBox(const FVolumeSampleInterpolationElement& Element)
	{
		const FVolumeLightingSample& Sample = Element.VolumeSamples(Element.SampleIndex);
		return FBoxCenterAndExtent(FVector4(Sample.PositionAndRadius, 0.0f), FVector4(Sample.PositionAndRadius.W, Sample.PositionAndRadius.W, Sample.PositionAndRadius.W));
	}
};

class FPrecomputedVisibilityCell
{
public:
	FBox Bounds;
	TArray<BYTE> VisibilityData;
};

/** Stores depth for a single cell of a shadow map for a dominant light. */
class FDominantLightShadowSample : public FDominantLightShadowSampleData
{
public:
	FDominantLightShadowSample(WORD InDistance)
	{
		Distance = InDistance;
	}
};

/** Stores information about how ShadowMap was generated. */
class FDominantLightShadowInfo : public FDominantLightShadowInfoData
{
public:
	TArray<FDominantLightShadowSample> ShadowMap;
};

/** Number of light bounces that we are keeping track of stats for */
static const INT NumTrackedBounces = 3;

/** Stats for a single mapping.  All times are thread seconds if the stat was calculated during a multi threaded mapping process. */
class FStaticLightingMappingStats
{
public:

	/** Part of TotalLightingThreadTime that was spent on texture mappings. */
	FLOAT TotalTextureMappingLightingThreadTime;

	/** Part of TotalLightingThreadTime that was spent on vertex mappings. */
	FLOAT TotalVertexMappingLightingThreadTime;

	/** Part of TotalLightingThreadTime that was spent on volume samples. */
	FLOAT TotalVolumeSampleLightingThreadTime;

	/** Time taken to generate the FTexelToVertexMap */
	FLOAT TexelRasterizationTime;

	/** Time taken to create vertex samples. */
	FLOAT VertexSampleCreationTime;

	/** Number of texels mapped to geometry in the scene. */
	INT NumMappedTexels;

	/** Number of vertex light samples calculated. */
	INT NumVertexSamples;

	/** Time taken to calculate direct lighting */
	FLOAT DirectLightingTime;

	/** Thread seconds spent calculating area shadows. */
	FLOAT AreaShadowsThreadTime;

	/** Thread seconds spent calculating light attenuation and influence. */
	FLOAT AreaLightingThreadTime;

	/** Accumulated signed distance field upsample factors for all mappings that used signed distance field shadows. */
	FLOAT AccumulatedSignedDistanceFieldUpsampleFactors;

	/** Number of mappings that used signed distance field shadows. */
	INT NumSignedDistanceFieldCalculations;

	/** Number of rays traced during the sparse source data generation pass. */
	QWORD NumSignedDistanceFieldAdaptiveSourceRaysFirstPass;

	/** Number of rays traced during the refining source data generation pass. */
	QWORD NumSignedDistanceFieldAdaptiveSourceRaysSecondPass;

	/** Thread seconds spend processing the sparse source data generation pass. */
	FLOAT SignedDistanceFieldSourceFirstPassThreadTime;

	/** Thread seconds spend processing the refining source data generation pass. */
	FLOAT SignedDistanceFieldSourceSecondPassThreadTime;

	/** Number of transition distance scatters during the distance field search pass. */
	QWORD NumSignedDistanceFieldScatters;

	/** Thread seconds spent searching the source data for the closest transition distance. */
	FLOAT SignedDistanceFieldSearchThreadTime;

	/** Number of cell - mesh queries processed. */
	QWORD NumPrecomputedVisibilityQueries;

	/** Number of cell - mesh queries considered visible because the mesh was very close to the cell. */
	QWORD NumQueriesVisibleByDistanceRatio;

	/** Number of cell - mesh queries determined visible from explicitly sampling the mesh's bounds. */
	QWORD NumQueriesVisibleExplicitSampling;

	/** Number of cell - mesh queries determined visible from importance sampling. */
	QWORD NumQueriesVisibleImportanceSampling;

	/** Number of rays traced for precomputed visibility. */
	QWORD NumPrecomputedVisibilityRayTraces;

	/** Number of visibility cells processed on this agent. */
	INT NumPrecomputedVisibilityCellsProcessed;

	/** Thread seconds processing visibility cells. */
	FLOAT PrecomputedVisibilityThreadTime;

	/** Thread seconds generating visibility sample positions. */
	FLOAT PrecomputedVisibilitySampleSetupThreadTime;

	/** Thread seconds tracing visibility rays. */
	FLOAT PrecomputedVisibilityRayTraceThreadTime;

	/** Thread seconds importance sampling visiblity queries. */
	FLOAT PrecomputedVisibilityImportanceSampleThreadTime;

	/** Thread seconds calculating the dominant shadow map. */
	FLOAT DominantShadowThreadTime;

	/** Thread seconds calculating the volume distance field. */
	FLOAT VolumeDistanceFieldThreadTime;

	/** Time taken to calculate indirect lighting */
	FLOAT IndirectLightingTime;

	/** Time taken to gather photons which are used for importance sampling the final gather */
	FLOAT ImportancePhotonGatherTime;

	/** Number of importance photons found */
	QWORD TotalFoundImportancePhotons;

	/** Time taken to generate a sample direction based on the importance photons, and then calculate the PDF for the generated sample direction. */
	FLOAT CalculateImportanceSampleTime;

	/** Number of elements contributing to the PDF times the number of samples */
	QWORD NumImportancePDFCalculations;

	/** Time taken to calculate the exitant radiance at the end of each final gather ray */
	FLOAT CalculateExitantRadianceTime;

	/** Number of final gather rays */
	QWORD NumFirstBounceRaysTraced;

	/** Time taken to trace final gather rays */
	FLOAT FirstBounceRayTraceTime;

	/** Number of shadow rays traced for direct lighting. */
	QWORD NumDirectLightingShadowRays;

	/** Number of times the nearest irradiance photon was searched for */
	QWORD NumIrradiancePhotonMapSearches;

	/** Number of unique irradiance photons cached on surfaces.  */
	INT NumFoundIrradiancePhotons;

	/** Number of irradiance photons placed outside the importance volume. */
	INT NumIrradiancePhotonsOutsideVolume;

	/** Number of nearest irradiance photon samples that were cached on surfaces */
	QWORD NumCachedIrradianceSamples;

	/** Time taken to gather caustic photons and esitmate caustic contribution from them */
	FLOAT CausticPhotonGatherTime;

	/** Time taken for irradiance cache interpolation for the final shading pass */
	FLOAT SecondPassIrradianceCacheInterpolationTime;

	/** Number of rays traced to determine visibility of irradiance photons while caching them on surfaces */
	QWORD NumIrradiancePhotonSearchRays;

	/** Time spent caching irradiance photons on surfaces */
	FLOAT IrradiancePhotonCachingThreadTime;

	/** Time taken traversing the irradiance photon octree */
	FLOAT IrradiancePhotonOctreeTraversalTime;

	/** Time taken to trace rays determining the visibility of irradiance photons */
	FLOAT IrradiancePhotonSearchRayTime;

	FStaticLightingMappingStats() :
		TotalTextureMappingLightingThreadTime(0),
		TotalVertexMappingLightingThreadTime(0),
		TotalVolumeSampleLightingThreadTime(0),
		TexelRasterizationTime(0),
		VertexSampleCreationTime(0),
		NumMappedTexels(0),
		NumVertexSamples(0),
		DirectLightingTime(0),
		AreaShadowsThreadTime(0),
		AreaLightingThreadTime(0),
		AccumulatedSignedDistanceFieldUpsampleFactors(0),
		NumSignedDistanceFieldCalculations(0),
		NumSignedDistanceFieldAdaptiveSourceRaysFirstPass(0),
		NumSignedDistanceFieldAdaptiveSourceRaysSecondPass(0),
		SignedDistanceFieldSourceFirstPassThreadTime(0),
		SignedDistanceFieldSourceSecondPassThreadTime(0),
		NumSignedDistanceFieldScatters(0),
		SignedDistanceFieldSearchThreadTime(0),
		NumPrecomputedVisibilityQueries(0),
		NumQueriesVisibleByDistanceRatio(0),
		NumQueriesVisibleExplicitSampling(0),
		NumQueriesVisibleImportanceSampling(0),
		NumPrecomputedVisibilityRayTraces(0),
		NumPrecomputedVisibilityCellsProcessed(0),
		PrecomputedVisibilityThreadTime(0),
		PrecomputedVisibilitySampleSetupThreadTime(0),
		PrecomputedVisibilityRayTraceThreadTime(0),
		PrecomputedVisibilityImportanceSampleThreadTime(0),
		DominantShadowThreadTime(0),
		VolumeDistanceFieldThreadTime(0),
		IndirectLightingTime(0),
		ImportancePhotonGatherTime(0),
		TotalFoundImportancePhotons(0),
		CalculateImportanceSampleTime(0),
		NumImportancePDFCalculations(0),
		CalculateExitantRadianceTime(0),
		NumFirstBounceRaysTraced(0),
		FirstBounceRayTraceTime(0),
		NumDirectLightingShadowRays(0),
		NumIrradiancePhotonMapSearches(0),
		NumFoundIrradiancePhotons(0),
		NumIrradiancePhotonsOutsideVolume(0),
		NumCachedIrradianceSamples(0),
		CausticPhotonGatherTime(0),
		SecondPassIrradianceCacheInterpolationTime(0),
		NumIrradiancePhotonSearchRays(0),
		IrradiancePhotonCachingThreadTime(0),
		IrradiancePhotonOctreeTraversalTime(0),
		IrradiancePhotonSearchRayTime(0)
	{
	}

	FStaticLightingMappingStats& operator+=(const FStaticLightingMappingStats& B)
	{
		TotalTextureMappingLightingThreadTime += B.TotalTextureMappingLightingThreadTime;
		TotalVertexMappingLightingThreadTime += B.TotalVertexMappingLightingThreadTime;
		TotalVolumeSampleLightingThreadTime += B.TotalVolumeSampleLightingThreadTime;
		TexelRasterizationTime += B.TexelRasterizationTime;
		VertexSampleCreationTime += B.VertexSampleCreationTime;
		NumMappedTexels += B.NumMappedTexels;
		NumVertexSamples += B.NumVertexSamples;
		DirectLightingTime += B.DirectLightingTime;
		AreaShadowsThreadTime += B.AreaShadowsThreadTime;
		AreaLightingThreadTime += B.AreaLightingThreadTime;
		AccumulatedSignedDistanceFieldUpsampleFactors += B.AccumulatedSignedDistanceFieldUpsampleFactors;
		NumSignedDistanceFieldCalculations += B.NumSignedDistanceFieldCalculations;
		NumSignedDistanceFieldAdaptiveSourceRaysFirstPass += B.NumSignedDistanceFieldAdaptiveSourceRaysFirstPass;
		NumSignedDistanceFieldAdaptiveSourceRaysSecondPass += B.NumSignedDistanceFieldAdaptiveSourceRaysSecondPass;
		SignedDistanceFieldSourceFirstPassThreadTime += B.SignedDistanceFieldSourceFirstPassThreadTime;
		SignedDistanceFieldSourceSecondPassThreadTime += B.SignedDistanceFieldSourceSecondPassThreadTime;
		NumSignedDistanceFieldScatters += B.NumSignedDistanceFieldScatters;
		SignedDistanceFieldSearchThreadTime += B.SignedDistanceFieldSearchThreadTime;
		NumPrecomputedVisibilityQueries += B.NumPrecomputedVisibilityQueries;
		NumQueriesVisibleByDistanceRatio += B.NumQueriesVisibleByDistanceRatio;
		NumQueriesVisibleExplicitSampling += B.NumQueriesVisibleExplicitSampling;
		NumQueriesVisibleImportanceSampling += B.NumQueriesVisibleImportanceSampling;
		NumPrecomputedVisibilityRayTraces += B.NumPrecomputedVisibilityRayTraces;
		NumPrecomputedVisibilityCellsProcessed += B.NumPrecomputedVisibilityCellsProcessed;
		PrecomputedVisibilityThreadTime += B.PrecomputedVisibilityThreadTime;
		PrecomputedVisibilitySampleSetupThreadTime += B.PrecomputedVisibilitySampleSetupThreadTime;
		PrecomputedVisibilityRayTraceThreadTime += B.PrecomputedVisibilityRayTraceThreadTime;
		PrecomputedVisibilityImportanceSampleThreadTime += B.PrecomputedVisibilityImportanceSampleThreadTime;
		DominantShadowThreadTime += B.DominantShadowThreadTime;
		VolumeDistanceFieldThreadTime += B.VolumeDistanceFieldThreadTime;
		IndirectLightingTime += B.IndirectLightingTime;
		ImportancePhotonGatherTime += B.ImportancePhotonGatherTime;
		TotalFoundImportancePhotons += B.TotalFoundImportancePhotons;
		CalculateImportanceSampleTime += B.CalculateImportanceSampleTime;
		NumImportancePDFCalculations += B.NumImportancePDFCalculations;
		CalculateExitantRadianceTime += B.CalculateExitantRadianceTime;
		NumFirstBounceRaysTraced += B.NumFirstBounceRaysTraced;
		FirstBounceRayTraceTime += B.FirstBounceRayTraceTime;
		NumDirectLightingShadowRays += B.NumDirectLightingShadowRays;
		NumIrradiancePhotonMapSearches += B.NumIrradiancePhotonMapSearches;
		NumFoundIrradiancePhotons += B.NumFoundIrradiancePhotons;
		NumIrradiancePhotonsOutsideVolume += B.NumIrradiancePhotonsOutsideVolume;
		NumCachedIrradianceSamples += B.NumCachedIrradianceSamples;
		CausticPhotonGatherTime += B.CausticPhotonGatherTime;
		SecondPassIrradianceCacheInterpolationTime += B.SecondPassIrradianceCacheInterpolationTime;
		NumIrradiancePhotonSearchRays += B.NumIrradiancePhotonSearchRays;
		IrradiancePhotonCachingThreadTime += B.IrradiancePhotonCachingThreadTime;
		IrradiancePhotonOctreeTraversalTime += B.IrradiancePhotonOctreeTraversalTime;
		IrradiancePhotonSearchRayTime += B.IrradiancePhotonSearchRayTime;
		return *this;
	}
};

/** Stats collected by FLightingSystem::FindNearbyPhotons*() */
struct FFindNearbyPhotonStats
{
	/** Number of photon map searches using the iterative search process. */
	QWORD NumIterativePhotonMapSearches;

	/** Number of size-increasing photon map search iterations until enough photons were found. */
	QWORD NumSearchIterations;

	/** Thread seconds spent traversing the photon map octree and pushing child nodes onto the traversal stack. */
	FLOAT PushingOctreeChildrenThreadTime;

	/** Thread seconds spent processing photons found in the photon map. */
	FLOAT ProcessingOctreeElementsThreadTime;

	/** Thread seconds spent finding the furthest found photon in order to replace it with the incoming one. */
	FLOAT FindingFurthestPhotonThreadTime;

	/** Number of octree nodes tested during all photon map searches. */
	QWORD NumOctreeNodesTested;

	/** Number of octree nodes that passed testing and had their elements processed during all photon map searches. */
	QWORD NumOctreeNodesVisited;

	/** Number of elements tested during all photon map searches. */
	QWORD NumElementsTested;

	/** Number of elements that passed testing during all photon map searches. */
	QWORD NumElementsAccepted;

	FFindNearbyPhotonStats() :
		NumIterativePhotonMapSearches(0),
		NumSearchIterations(0),
		PushingOctreeChildrenThreadTime(0),
		ProcessingOctreeElementsThreadTime(0),
		FindingFurthestPhotonThreadTime(0),
		NumOctreeNodesTested(0),
		NumOctreeNodesVisited(0),
		NumElementsTested(0),
		NumElementsAccepted(0)
	{}

	FFindNearbyPhotonStats& operator+=(const FFindNearbyPhotonStats& B)
	{
		NumIterativePhotonMapSearches += B.NumIterativePhotonMapSearches;
		NumSearchIterations += B.NumSearchIterations;
		PushingOctreeChildrenThreadTime += B.PushingOctreeChildrenThreadTime;
		ProcessingOctreeElementsThreadTime += B.ProcessingOctreeElementsThreadTime;
		FindingFurthestPhotonThreadTime += B.FindingFurthestPhotonThreadTime;
		NumOctreeNodesTested += B.NumOctreeNodesTested;
		NumOctreeNodesVisited += B.NumOctreeNodesVisited;
		NumElementsTested += B.NumElementsTested;
		NumElementsAccepted += B.NumElementsAccepted;
		return *this;
	}
};

/** Stats collected by FStaticLightingSystem::CalculateIrradiancePhotonsThreadLoop() */
struct FCalculateIrradiancePhotonStats : public FFindNearbyPhotonStats
{
	/** Thread seconds spent calculating irradiance once the relevant photons have been found. */
	FLOAT CalculateIrradianceThreadTime;

	FCalculateIrradiancePhotonStats() :
		FFindNearbyPhotonStats(),
		CalculateIrradianceThreadTime(0)
	{}

	FCalculateIrradiancePhotonStats& operator+=(const FCalculateIrradiancePhotonStats& B)
	{
		(FFindNearbyPhotonStats&)(*this) += (const FFindNearbyPhotonStats&)B;
		CalculateIrradianceThreadTime += B.CalculateIrradianceThreadTime;
		return *this;
	}
};

/** Stats for the whole lighting system, which belong to the main thread.  Other threads must use synchronization to access them. */
class FStaticLightingStats : public FStaticLightingMappingStats
{
public:
	
	/** Main thread seconds setting up the scene */
	FLOAT SceneSetupTime;

	/** Main thread seconds setting up mesh area lights */
	FLOAT MeshAreaLightSetupTime;
	
	/** Thread seconds spent processing mappings for the final lighting pass */
	FLOAT TotalLightingThreadTime;

	/** Main thread seconds until the final lighting pass was complete */
	FLOAT MainThreadLightingTime;

	/** Number of mappings processed */
	INT NumMappings;

	/** Number of texels processed */
	INT NumTexelsProcessed;

	/** Number of vertices processed */
	INT NumVerticesProcessed;

	/** Number of lights in the scene. */
	INT NumLights;

	/** Number of meshes that created mesh area lights. */
	INT NumMeshAreaLightMeshes;

	/** Number of mesh area lights created from emissive areas in the scene. */
	INT NumMeshAreaLights;

	/** Number of mesh area light primitives before simplification. */
	QWORD NumMeshAreaLightPrimitives;

	/** Number of simplified mesh area light primitives that are used for lighting. */
	QWORD NumSimplifiedMeshAreaLightPrimitives;

	/** Number of volume lighting samples created off of surfaces. */
	INT NumDynamicObjectSurfaceSamples;

	/** Number of volume lighting samples created based on the importance volume. */
	INT NumDynamicObjectVolumeSamples;

	/** Total number of cells which visibility will be computed for. */
	INT NumPrecomputedVisibilityCellsTotal;

	/** Number of visibility cells generated on camera tracks. */
	INT NumPrecomputedVisibilityCellsCamaraTracks;

	/** Number of meshes with valid visibility Ids in the scene. */
	INT NumPrecomputedVisibilityMeshes;

	/** Size of the raw visibility data exported. */
	SIZE_T PrecomputedVisibilityDataBytes;

	/** Main thread time emitting direct photons */
	FLOAT EmitDirectPhotonsTime;

	/** Thread time emitting direct photons. */
	FLOAT EmitDirectPhotonsThreadTime;

	/** Thread time tracing rays for direct photons. */
	FLOAT DirectPhotonsTracingThreadTime;

	/** Thread time generating light samples for direct photons. */
	FLOAT DirectPhotonsLightSamplingThreadTime;

	/** Thread time spent applying non-physical attenuation to direct photons. */
	FLOAT DirectCustomAttenuationThreadTime;

	/** Thread seconds spent processing gathered direct photons and adding them to spatial data structures. */
	FLOAT ProcessDirectPhotonsThreadTime;

	/** Number of direct photons that were deposited on surfaces. */
	INT NumDirectPhotonsGathered;

	/** Main thread time letting lights cache information about the indirect photon paths. */
	FLOAT CachingIndirectPhotonPathsTime;

	/** Main thread time emitting indirect photons */
	FLOAT EmitIndirectPhotonsTime;

	/** Thread time emitting indirect photons. */
	FLOAT EmitIndirectPhotonsThreadTime;

	/** Thread seconds spent processing gathered indirect photons and adding them to spatial data structures. */
	FLOAT ProcessIndirectPhotonsThreadTime;

	/** Thread time sampling lights while emitting indirect photons. */
	FLOAT LightSamplingThreadTime;

	/** Thread time spent applying non-physical attenuation to indirect photons. */
	FLOAT IndirectCustomAttenuationThreadTime;

	/** Thread time tracing indirect photons from a light. */
	FLOAT IntersectLightRayThreadTime;

	/** Thread time tracing photons bouncing off of surfaces in the scene. */
	FLOAT PhotonBounceTracingThreadTime;

	/** Number of indirect photons that were deposited on surfaces. */
	INT NumIndirectPhotonsGathered;

	/** Main thread time marking photons as having direct lighting influence or not. */
	FLOAT IrradiancePhotonMarkingTime;

	/** Thread time marking photons. */
	FLOAT IrradiancePhotonMarkingThreadTime;

	/** Main thread time calculating irradiance photons */
	FLOAT IrradiancePhotonCalculatingTime;

	/** Thread time calculating irradiance photons */
	FLOAT IrradiancePhotonCalculatingThreadTime;

	/** Main thread time caching irradiance photons on surfaces */
	FLOAT CacheIrradiancePhotonsTime;

	/** Number of irradiance photons */
	INT NumIrradiancePhotons;

	/** Number of irradiance photons created off of direct photons. */
	INT NumDirectIrradiancePhotons;

	/** Number of times a photon map was searched, excluding irradiance photon searches */
	volatile INT NumPhotonGathers;

	/** Number of photons emitted in the first pass */
	QWORD NumFirstPassPhotonsEmitted;

	/** Number of photons requested to be emitted in the first pass */
	QWORD NumFirstPassPhotonsRequested;

	/** Number of photons emitted in the second pass */
	QWORD NumSecondPassPhotonsEmitted;

	/** Number of photons requested to be emitted in the second pass */
	QWORD NumSecondPassPhotonsRequested;

	/** Total number of first hit rays traced */
	QWORD NumFirstHitRaysTraced;

	/** Total number of boolean visibility rays traced */
	QWORD NumBooleanRaysTraced;

	/** Thread seconds spent tracing first hit rays */
	FLOAT FirstHitRayTraceThreadTime;

	/** Thread seconds spent tracing shadow rays */
	FLOAT BooleanRayTraceThreadTime;

	/** Thread seconds spent placing volume lighting samples and then calculating their lighting. */
	FLOAT VolumeSampleThreadTime;

	/** Irradiance cache stats */
	FIrradianceCacheStats Cache[NumTrackedBounces];

	/** Stats from irradiance photon calculations. */
	FCalculateIrradiancePhotonStats CalculateIrradiancePhotonStats;

	/** Critical section that worker threads must acquire before writing to members of this class */
	FCriticalSection StatsSync;

	FStaticLightingStats() :
		SceneSetupTime(0),
		MeshAreaLightSetupTime(0),
		TotalLightingThreadTime(0),
		MainThreadLightingTime(0),
		NumMappings(0),
		NumTexelsProcessed(0),
		NumVerticesProcessed(0),
		NumLights(0),
		NumMeshAreaLightMeshes(0),
		NumMeshAreaLights(0),
		NumMeshAreaLightPrimitives(0),
		NumSimplifiedMeshAreaLightPrimitives(0),
		NumDynamicObjectSurfaceSamples(0),
		NumDynamicObjectVolumeSamples(0),
		NumPrecomputedVisibilityCellsTotal(0),
		NumPrecomputedVisibilityCellsCamaraTracks(0),
		NumPrecomputedVisibilityMeshes(0),
		PrecomputedVisibilityDataBytes(0),
		EmitDirectPhotonsTime(0),
		EmitDirectPhotonsThreadTime(0),
		DirectPhotonsTracingThreadTime(0),
		DirectPhotonsLightSamplingThreadTime(0),
		DirectCustomAttenuationThreadTime(0),
		ProcessDirectPhotonsThreadTime(0),
		NumDirectPhotonsGathered(0),
		CachingIndirectPhotonPathsTime(0),
		EmitIndirectPhotonsTime(0),
		EmitIndirectPhotonsThreadTime(0),
		ProcessIndirectPhotonsThreadTime(0),
		LightSamplingThreadTime(0),
		IndirectCustomAttenuationThreadTime(0),
		IntersectLightRayThreadTime(0),
		PhotonBounceTracingThreadTime(0),
		NumIndirectPhotonsGathered(0),
		IrradiancePhotonMarkingTime(0),
		IrradiancePhotonMarkingThreadTime(0),
		IrradiancePhotonCalculatingTime(0),
		IrradiancePhotonCalculatingThreadTime(0),
		CacheIrradiancePhotonsTime(0),
		NumIrradiancePhotons(0),
		NumDirectIrradiancePhotons(0),
		NumPhotonGathers(0),
		NumFirstPassPhotonsEmitted(0),
		NumFirstPassPhotonsRequested(0),
		NumSecondPassPhotonsEmitted(0),
		NumSecondPassPhotonsRequested(0),
		NumFirstHitRaysTraced(0),
		NumBooleanRaysTraced(0),
		FirstHitRayTraceThreadTime(0),
		BooleanRayTraceThreadTime(0),
		VolumeSampleThreadTime(0)
	{
		for (INT i = 0; i < NumTrackedBounces; i++)
		{
			Cache[i] = FIrradianceCacheStats();
		}
	}
};

/** Local state for a mapping, accessed only by the owning thread. */
class FStaticLightingMappingContext
{
public:
	FStaticLightingMappingStats Stats;

	/** Lighting caches for the mapping */
	TLightingCache<FGatheredLightSample> AreaLightingCache;
	TLightingCache<FFinalGatherSample> FirstBounceCache;
	TLightingCache<FLinearColor> SecondBounceCache;
	TLightingCache<FLinearColor> ThirdBounceCache;

	FCoherentRayCache RayCache;

	INT NumUniformSamples;
	INT NumUnshadowedEnvironmentSamples;

private:

	class FStaticLightingSystem& System;

public:
	FStaticLightingMappingContext(const FStaticLightingMesh* InSubjectMesh, class FStaticLightingSystem& InSystem);

	~FStaticLightingMappingContext();
};

/** Information about the power distribution of lights in the scene. */
class FSceneLightPowerDistribution
{
public:
	/** Stores an unnormalized step 1D probability distribution function of emitting a photon from a given light */
	TArray<FLOAT> LightPDFs;

	/** Stores the cumulative distribution function of LightPDFs */
	TArray<FLOAT> LightCDFs;

	/** Stores the integral of LightPDFs */
	FLOAT UnnormalizedIntegral;
};

/** The static lighting data for a vertex mapping. */
struct FVertexMappingStaticLightingData
{
	FStaticLightingVertexMapping* Mapping;
	FLightMapData1D* LightMapData;
	TMap<FLight*, FShadowMapData1D*> ShadowMaps;
	BYTE PreviewEnvironmentShadowing;

	/** Stores the time this mapping took to process */
	DOUBLE ExecutionTime;
};

/** The static lighting data for a texture mapping. */
struct FTextureMappingStaticLightingData
{
	FStaticLightingTextureMapping* Mapping;
	FLightMapData2D* LightMapData;
	TMap<const FLight*,FShadowMapData2D*> ShadowMaps;
	TMap<const FLight*,FSignedDistanceFieldShadowMapData2D*> SignedDistanceFieldShadowMaps;
	BYTE PreviewEnvironmentShadowing;

	/** Stores the time this mapping took to process */
	DOUBLE ExecutionTime;
};

/** Visibility output data from a single visibility task. */
struct FPrecomputedVisibilityData
{
	FGuid Guid;
	TArray<FPrecomputedVisibilityCell> PrecomputedVisibilityCells;
	TArray<FDebugStaticLightingRay> DebugVisibilityRays;
};

/** A thread which processes static lighting mappings. */
class FStaticLightingThreadRunnable : public FRunnable
{
public:

	FRunnableThread* Thread;

	/** Seconds that the thread spent in Run() */
	FLOAT ExecutionTime;

	/** Seconds since GStartupTime that the thread exited Run() */
	DOUBLE EndTime;

	INT	ThreadIndex;

	FThreadStatistics ThreadStatistics;

	/** Initialization constructor. */
	FStaticLightingThreadRunnable(FStaticLightingSystem* InSystem, INT InThreadIndex) :
		System(InSystem),
		Thread(NULL),
		ThreadIndex(InThreadIndex),
		bTerminatedByError(FALSE)
	{}

	FStaticLightingThreadRunnable(FStaticLightingSystem* InSystem) :
		System(InSystem),
		Thread(NULL),
		ThreadIndex(0),
		bTerminatedByError(FALSE)
	{}

	/** Checks the thread's health, and passes on any errors that have occurred.  Called by the main thread. */
	UBOOL CheckHealth(UBOOL bReportError = TRUE) const;

protected:
	FStaticLightingSystem* System;

	/** If the thread has been terminated by an unhandled exception, this contains the error message. */
	FString ErrorMessage;

	/** TRUE if the thread has been terminated by an unhandled exception. */
	UBOOL bTerminatedByError;
};

/** Input required to emit direct photons. */
class FDirectPhotonEmittingInput
{
public:
	const FBoxSphereBounds& ImportanceBounds;
	const FSceneLightPowerDistribution& LightDistribution;
	
	FDirectPhotonEmittingInput(
		const FBoxSphereBounds& InImportanceBounds,
		const FSceneLightPowerDistribution& InLightDistribution)
		:
		ImportanceBounds(InImportanceBounds),
		LightDistribution(InLightDistribution)
	{}
};

/** A work range for emitting direct photons, which is the smallest unit that can be parallelized. */
class FDirectPhotonEmittingWorkRange
{
public:
	const INT RangeIndex;
	const INT NumDirectPhotonsToEmit;
	const INT TargetNumIndirectPhotonPaths;

	FDirectPhotonEmittingWorkRange(
		INT InRangeIndex,
		INT InNumDirectPhotonsToEmit,
		INT InTargetNumIndirectPhotonPaths)
		:
		RangeIndex(InRangeIndex),
		NumDirectPhotonsToEmit(InNumDirectPhotonsToEmit),
		TargetNumIndirectPhotonPaths(InTargetNumIndirectPhotonPaths)
	{}
};

/** Direct photon emitting output for a single FDirectPhotonEmittingWorkRange. */
class FDirectPhotonEmittingOutput
{
public:
	/** 
	 * A worker thread will increment this counter once the output is complete, 
	 * so that the main thread can process it while the worker thread moves on. 
	 */
	volatile INT OutputComplete;
	INT NumPhotonsEmittedDirect;
	TArray<FPhoton> DirectPhotons;
	TArray<FIrradiancePhoton>* IrradiancePhotons;
	TArray<TArray<FIndirectPathRay> > IndirectPathRays;
	INT NumPhotonsEmitted;
	FLOAT DirectPhotonsTracingThreadTime;
	FLOAT DirectPhotonsLightSamplingThreadTime;
	FLOAT DirectCustomAttenuationThreadTime;

	FDirectPhotonEmittingOutput(TArray<FIrradiancePhoton>* InIrradiancePhotons) :
		OutputComplete(0),
		NumPhotonsEmittedDirect(0),
		IrradiancePhotons(InIrradiancePhotons),
		NumPhotonsEmitted(0),
		DirectPhotonsTracingThreadTime(0),
		DirectPhotonsLightSamplingThreadTime(0),
		DirectCustomAttenuationThreadTime(0)
	{}
};

/** Thread used to parallelize indirect photon emitting. */
class FDirectPhotonEmittingThreadRunnable : public FStaticLightingThreadRunnable
{
public:

	/** Initialization constructor. */
	FDirectPhotonEmittingThreadRunnable::FDirectPhotonEmittingThreadRunnable(
		FStaticLightingSystem* InSystem, 
		INT InThreadIndex,
		const FDirectPhotonEmittingInput& InInput)
		:
		ThreadIndex(InThreadIndex),
		FStaticLightingThreadRunnable(InSystem),
		Input(InInput)
	{}

	// FRunnable interface.
	virtual UBOOL Init(void) { return TRUE; }
	virtual void Exit(void) {}
	virtual void Stop(void) {}
	virtual DWORD Run(void);

protected:
	INT ThreadIndex;
	const FDirectPhotonEmittingInput& Input;
};

/** Input required to emit indirect photons. */
class FIndirectPhotonEmittingInput
{
public:
	const FBoxSphereBounds& ImportanceBounds;
	const FSceneLightPowerDistribution& LightDistribution;
	const TArray<TArray<FIndirectPathRay> >& IndirectPathRays;
	
	FIndirectPhotonEmittingInput(
		const FBoxSphereBounds& InImportanceBounds,
		const FSceneLightPowerDistribution& InLightDistribution,
		const TArray<TArray<FIndirectPathRay> >& InIndirectPathRays)
		:
		ImportanceBounds(InImportanceBounds),
		LightDistribution(InLightDistribution),
		IndirectPathRays(InIndirectPathRays)
	{}
};

/** A work range for emitting indirect photons, which is the smallest unit that can be parallelized. */
class FIndirectPhotonEmittingWorkRange
{
public:
	const INT RangeIndex;
	const INT NumIndirectPhotonsToEmit;
	const INT TargetNumCausticPhotons;

	FIndirectPhotonEmittingWorkRange(
		INT InRangeIndex,
		INT InNumIndirectPhotonsToEmit,
		INT InTargetNumCausticPhotons)
		:
		RangeIndex(InRangeIndex),
		NumIndirectPhotonsToEmit(InNumIndirectPhotonsToEmit),
		TargetNumCausticPhotons(InTargetNumCausticPhotons)
	{}
};

/** Indirect photon emitting output for a single FIndirectPhotonEmittingWorkRange. */
class FIndirectPhotonEmittingOutput
{
public:
	/** 
	 * A worker thread will increment this counter once the output is complete, 
	 * so that the main thread can process it while the worker thread moves on. 
	 */
	volatile INT OutputComplete;
	INT NumPhotonsEmittedFirstBounce;
	TArray<FPhoton> FirstBouncePhotons;
	INT NumPhotonsEmittedSecondBounce;
	TArray<FPhoton> SecondBouncePhotons;
	INT NumPhotonsEmittedCaustic;
	TArray<FPhoton> CausticPhotons;
	TArray<FIrradiancePhoton>* IrradiancePhotons;
	INT NumPhotonsEmitted;
	FLOAT LightSamplingThreadTime;
	FLOAT IndirectCustomAttenuationThreadTime;
	FLOAT IntersectLightRayThreadTime;
	FLOAT PhotonBounceTracingThreadTime;

	FIndirectPhotonEmittingOutput(TArray<FIrradiancePhoton>* InIrradiancePhotons) :
		OutputComplete(0),
		NumPhotonsEmittedFirstBounce(0),
		NumPhotonsEmittedSecondBounce(0),
		NumPhotonsEmittedCaustic(0),
		IrradiancePhotons(InIrradiancePhotons),
		NumPhotonsEmitted(0),
		LightSamplingThreadTime(0),
		IndirectCustomAttenuationThreadTime(0),
		IntersectLightRayThreadTime(0),
		PhotonBounceTracingThreadTime(0)
	{}
};

/** Thread used to parallelize indirect photon emitting. */
class FIndirectPhotonEmittingThreadRunnable : public FStaticLightingThreadRunnable
{
public:

	/** Initialization constructor. */
	FIndirectPhotonEmittingThreadRunnable::FIndirectPhotonEmittingThreadRunnable(
		FStaticLightingSystem* InSystem, 
		INT InThreadIndex,
		const FIndirectPhotonEmittingInput& InInput)
		:
		FStaticLightingThreadRunnable(InSystem),
		ThreadIndex(InThreadIndex),
		Input(InInput)
	{}

	// FRunnable interface.
	virtual UBOOL Init(void) { return TRUE; }
	virtual void Exit(void) {}
	virtual void Stop(void) {}
	virtual DWORD Run(void);

protected:
	INT ThreadIndex;
	const FIndirectPhotonEmittingInput& Input;
};

/** Smallest unit of irradiance photon marking work that can be done in parallel. */
class FIrradianceMarkingWorkRange
{
public:
	const INT RangeIndex;
	/** Index into IrradiancePhotons that should be processed for this work range. */
	const INT IrradiancePhotonArrayIndex;

	FIrradianceMarkingWorkRange(
		INT InRangeIndex,
		INT InIrradiancePhotonArrayIndex)
		:
		RangeIndex(InRangeIndex),
		IrradiancePhotonArrayIndex(InIrradiancePhotonArrayIndex)
	{}
};

class FIrradiancePhotonMarkingThreadRunnable : public FStaticLightingThreadRunnable
{
public:

	/** Initialization constructor. */
	FIrradiancePhotonMarkingThreadRunnable(FStaticLightingSystem* InSystem, INT InThreadIndex, TArray<TArray<FIrradiancePhoton>>& InIrradiancePhotons) :
		FStaticLightingThreadRunnable(InSystem),
		ThreadIndex(InThreadIndex),
		IrradiancePhotons(InIrradiancePhotons)
	{}

	// FRunnable interface.
	virtual UBOOL Init(void) { return TRUE; }
	virtual void Exit(void) {}
	virtual void Stop(void) {}
	virtual DWORD Run(void);

private:
	const INT ThreadIndex;
	/** Irradiance photons to operate on */
	TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons;
};

/** Smallest unit of irradiance photon calculating work that can be done in parallel. */
class FIrradianceCalculatingWorkRange
{
public:
	const INT RangeIndex;
	/** Index into IrradiancePhotons that should be processed for this work range. */
	const INT IrradiancePhotonArrayIndex;

	FIrradianceCalculatingWorkRange(
		INT InRangeIndex,
		INT InIrradiancePhotonArrayIndex)
		:
		RangeIndex(InRangeIndex),
		IrradiancePhotonArrayIndex(InIrradiancePhotonArrayIndex)
	{}
};

class FIrradiancePhotonCalculatingThreadRunnable : public FStaticLightingThreadRunnable
{
public:

	/** Stats for this thread's operations. */
	FCalculateIrradiancePhotonStats Stats;

	/** Initialization constructor. */
	FIrradiancePhotonCalculatingThreadRunnable(FStaticLightingSystem* InSystem, INT InThreadIndex, TArray<TArray<FIrradiancePhoton>>& InIrradiancePhotons) :
		FStaticLightingThreadRunnable(InSystem),
		ThreadIndex(InThreadIndex),
		IrradiancePhotons(InIrradiancePhotons)
	{}

	// FRunnable interface.
	virtual UBOOL Init(void) { return TRUE; }
	virtual void Exit(void) {}
	virtual void Stop(void) {}
	virtual DWORD Run(void);

private:
	const INT ThreadIndex;
	/** Irradiance photons to operate on */
	TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons;
};

/** Indicates which type of task a FMappingProcessingThreadRunnable should execute */
enum EStaticLightingTaskType
{
	StaticLightingTask_ProcessMappings,
	StaticLightingTask_CacheIrradiancePhotons
};

/** A thread which processes static lighting mappings. */
class FMappingProcessingThreadRunnable : public FStaticLightingThreadRunnable
{
public:

	EStaticLightingTaskType TaskType;

	/** Initialization constructor. */
	FMappingProcessingThreadRunnable(FStaticLightingSystem* InSystem, INT ThreadIndex, EStaticLightingTaskType InTaskType) :
		FStaticLightingThreadRunnable(InSystem, ThreadIndex),
		TaskType(InTaskType)
	{}

	// FRunnable interface.
	virtual UBOOL Init(void) { return TRUE; }
	virtual void Exit(void) {}
	virtual void Stop(void) {}
	virtual DWORD Run(void);
};

/** Encapsulates a list of mappings which static lighting has been computed for, but not yet applied. */
template<typename StaticLightingDataType>
class TCompleteStaticLightingList
{
public:

	/** Initialization constructor. */
	TCompleteStaticLightingList():
		FirstElement(NULL)
	{}

	/** Adds an element to the list. */
	void AddElement(TList<StaticLightingDataType>* Element)
	{
		// Link the element at the beginning of the list.
		TList<StaticLightingDataType>* LocalFirstElement;
		do 
		{
			LocalFirstElement = FirstElement;
			Element->Next = LocalFirstElement;
		}
		while(appInterlockedCompareExchangePointer((void**)&FirstElement,Element,LocalFirstElement) != LocalFirstElement);
	}

	/**
	 * Applies the static lighting to the mappings in the list, and clears the list.
	 * Also reports back to UE3 after each mapping has been exported.
	 * @param LightingSystem - Reference to the static lighting system
	 */
	void ApplyAndClear(FStaticLightingSystem& LightingSystem);

protected:

	TList<StaticLightingDataType>* FirstElement;
};

template<typename DataType>
class TCompleteTaskList : public TCompleteStaticLightingList<DataType>
{
public:
	void ApplyAndClear(FStaticLightingSystem& LightingSystem);
};

/** The state of the static lighting system. */
class FStaticLightingSystem
{
public:

	/** Debug data to transfer back to UE3. */
	mutable FDebugLightingOutput DebugOutput;

	FVolumeLightingDebugOutput VolumeLightingDebugOutput;

	/** Threads must acquire this critical section before reading or writing to DebugOutput or VolumeLightingDebugOutput, if contention is possible. */
	mutable FCriticalSection DebugOutputSync;

	/**
	 * Initializes this static lighting system, and builds static lighting based on the provided options.
	 * @param InOptions		- The static lighting build options.
	 * @param InScene		- The scene containing all the lights and meshes
	 * @param InExporter	- The exporter used to send completed data back to UE3
	 * @param InNumThreads	- Number of concurrent threads to use for lighting building
	 */
	FStaticLightingSystem( const FLightingBuildOptions& InOptions, class FScene& InScene, class FLightmassSolverExporter& InExporter, INT InNumThreads );

	/**
	 * Returns the Lightmass exporter (back to UE3)
	 * @return	Lightmass exporter
	 */
	class FLightmassSolverExporter& GetExporter()
	{
		return Exporter;
	}

	FGuid GetDebugGuid() const { return Scene.DebugInput.MappingGuid; }

	/**
	 * Whether the Lighting System is in debug mode or not. When in debug mode, Swarm is not completely hooked up
	 * and the system will process all mappings in the scene file on its own.
	 * @return	TRUE if in debug mode
	 */
	UBOOL IsDebugMode() const
	{
		return GDebugMode;
	}

	void CompleteDeterministicMappings(class FLightmassProcessor* LightMassProcessor);

	/** Rasterizes Mesh into TexelToCornersMap */
	void CalculateTexelCorners(const FStaticLightingMesh* Mesh, class FTexelToCornersMap& TexelToCornersMap, INT UVIndex, UBOOL bDebugThisMapping) const;

	/** Rasterizes Mesh into TexelToCornersMap, with extra parameters like which material index to rasterize and UV scale and bias. */
	void CalculateTexelCorners(
		const TArray<INT>& TriangleIndices, 
		const TArray<FStaticLightingVertex>& Vertices, 
		FTexelToCornersMap& TexelToCornersMap, 
		const TArray<INT>& ElementIndices,
		INT MaterialIndex,
		INT UVIndex, 
		UBOOL bDebugThisMapping, 
		FVector2D UVBias, 
		FVector2D UVScale) const;

private:

	/** Exports tasks that are not mappings, if they are ready. */
	void ExportNonMappingTasks();

	/** Internal accessors */
	INT GetNumShadowRays(INT BounceNumber, UBOOL bPenumbra=FALSE) const;
	INT GetNumUniformHemisphereSamples(INT BounceNumber) const;
	INT GetNumPhotonImportanceHemisphereSamples() const;

	FBoxSphereBounds GetImportanceBounds(UBOOL bClampToScene = TRUE) const;

	/** Returns TRUE if the specified position is inside any of the importance volumes. */
	UBOOL IsPointInImportanceVolume(const FVector4& Position) const;

	/** Changes the scene's settings if necessary so that only valid combinations are used */
	void ValidateSettings(FScene& InScene);

	/** Logs solver stats */
	void DumpStats(FLOAT TotalStaticLightingTime) const;

	/** Logs a solver message */
	void LogSolverMessage(const FString& Message) const;

	/** Logs a progress update message when appropriate */
	void UpdateInternalStatus(INT OldNumTexelsCompleted, INT OldNumVerticesCompleted) const;

	/** Caches samples for any sampling distributions that are known ahead of time, which greatly reduces noise in those estimates. */
	void CacheSamples();

	/** Sets up photon mapping settings. */
	void InitializePhotonSettings();

	/** Emits photons, builds data structures to accelerate photon map lookups, and does any other photon preprocessing required. */
	void EmitPhotons();

	/** Gathers direct photons and generates indirect photon paths. */
	void EmitDirectPhotons(
		const FBoxSphereBounds& ImportanceBounds, 
		TArray<TArray<FIndirectPathRay> >& IndirectPathRays,
		TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons);

	/** Entrypoint for all threads emitting direct photons. */
	void EmitDirectPhotonsThreadLoop(const FDirectPhotonEmittingInput& Input, INT ThreadIndex);

	/** Emits direct photons for a given work range. */
	void EmitDirectPhotonsWorkRange(
		const FDirectPhotonEmittingInput& Input, 
		FDirectPhotonEmittingWorkRange WorkRange, 
		FDirectPhotonEmittingOutput& Output);

	/** Gathers indirect and caustic photons based on the indirect photon paths. */
	void EmitIndirectPhotons(
		const FBoxSphereBounds& ImportanceBounds,
		const TArray<TArray<FIndirectPathRay> >& IndirectPathRays, 
		TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons);

	/** Entrypoint for all threads emitting indirect photons. */
	void EmitIndirectPhotonsThreadLoop(const FIndirectPhotonEmittingInput& Input, INT ThreadIndex);

	/** Emits indirect photons for a given work range. */
	void EmitIndirectPhotonsWorkRange(
		const FIndirectPhotonEmittingInput& Input, 
		FIndirectPhotonEmittingWorkRange WorkRange, 
		FIndirectPhotonEmittingOutput& Output);

	/** Iterates through all irradiance photons, searches for nearby direct photons, and marks the irradiance photon has having direct photon influence if necessary. */
	void MarkIrradiancePhotons(const FBoxSphereBounds& ImportanceBounds, TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons);

	/** Entry point for all threads marking irradiance photons. */
	void MarkIrradiancePhotonsThreadLoop(
		INT ThreadIndex, 
		TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons);

	/** Marks irradiance photons specified by a single work range. */
	void MarkIrradiancePhotonsWorkRange(
		TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons, 
		FIrradianceMarkingWorkRange WorkRange);

	/** Calculates irradiance for photons randomly chosen to precalculate irradiance. */
	void CalculateIrradiancePhotons(const FBoxSphereBounds& ImportanceBounds, TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons);

	/** Main loop that all threads access to calculate irradiance photons. */
	void CalculateIrradiancePhotonsThreadLoop(
		INT ThreadIndex, 
		TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons, 
		FCalculateIrradiancePhotonStats& Stats);

	/** Calculates irradiance for the photons specified by a single work range. */
	void CalculateIrradiancePhotonsWorkRange(
		TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons, 
		FIrradianceCalculatingWorkRange WorkRange,
		FCalculateIrradiancePhotonStats& Stats);

	/** Cache irradiance photons on surfaces. */
	void CacheIrradiancePhotons();

	/** Main loop that all threads access to cache irradiance photons. */
	void CacheIrradiancePhotonsThreadLoop(INT ThreadIndex, UBOOL bIsMainThread);

	/** Caches irradiance photons on a single texture mapping. */
	void CacheIrradiancePhotonsTextureMapping(FStaticLightingTextureMapping* TextureMapping);

	/** Caches irradiance photons on a single vertex mapping. */
	void CacheIrradiancePhotonsVertexMapping(FStaticLightingVertexMapping* VertexMapping);

	/** Returns TRUE if a photon was found within MaxPhotonSearchDistance. */
	UBOOL FindAnyNearbyPhoton(
		const FPhotonOctree& PhotonMap, 
		const FVector4& SearchPosition, 
		FLOAT MaxPhotonSearchDistance,
		UBOOL bDebugThisLookup) const;

	/** 
	 * Searches the given photon map for the nearest NumPhotonsToFind photons to SearchPosition using an iterative process, 
	 * Unless the start and max search distances are the same, in which case all photons in that distance will be returned.
	 * The iterative search starts at StartPhotonSearchDistance and doubles the search distance until enough photons are found or the distance is greater than MaxPhotonSearchDistance.
	 * @return - the furthest found photon's distance squared from SearchPosition, unless the start and max search distances are the same,
	 *		in which case Square(MaxPhotonSearchDistance) will be returned.
	 */
	FLOAT FindNearbyPhotonsIterative(
		const FPhotonOctree& PhotonMap, 
		const FVector4& SearchPosition, 
		const FVector4& SearchNormal, 
		INT NumPhotonsToFind,
		FLOAT StartPhotonSearchDistance, 
		FLOAT MaxPhotonSearchDistance,
		UBOOL bDebugSearchResults,
		UBOOL bDebugSearchProcess,
		TArray<FPhoton>& FoundPhotons,
		FFindNearbyPhotonStats& SearchStats) const;

	/** 
	 * Searches the given photon map for the nearest NumPhotonsToFind photons to SearchPosition by sorting octree nodes nearest to furthest.
	 * @return - the furthest found photon's distance squared from SearchPosition.
	 */
	FLOAT FindNearbyPhotonsSorted(
		const FPhotonOctree& PhotonMap, 
		const FVector4& SearchPosition, 
		const FVector4& SearchNormal, 
		INT NumPhotonsToFind, 
		FLOAT MaxPhotonSearchDistance,
		UBOOL bDebugSearchResults,
		UBOOL bDebugSearchProcess,
		TArray<FPhoton>& FoundPhotons,
		FFindNearbyPhotonStats& SearchStats) const;

	/** Finds the nearest irradiance photon, if one exists. */
	FIrradiancePhoton* FindNearestIrradiancePhoton(
		const FStaticLightingVertex& Vertex, 
		FStaticLightingMappingContext& MappingContext, 
		TArray<FIrradiancePhoton*>& TempIrradiancePhotons,
		UBOOL bVisibleOnly, 
		UBOOL bDebugThisLookup) const;

	/** Calculates the irradiance for an irradiance photon */
	FLinearColor CalculatePhotonIrradiance(
		const FPhotonOctree& PhotonMap,
		INT NumPhotonsEmitted, 
		INT NumPhotonsToFind,
		FLOAT SearchDistance,
		const FIrradiancePhoton& IrradiancePhoton,
		UBOOL bDebugThisCalculation,
		TArray<FPhoton>& TempFoundPhotons,
		FCalculateIrradiancePhotonStats& Stats) const;

	/** Calculates incident radiance at a vertex from the given photon map. */
	FGatheredLightSample CalculatePhotonIncidentRadiance(
		const FPhotonOctree& PhotonMap,
		INT NumPhotonsEmitted, 
		FLOAT SearchDistance,
		const FStaticLightingVertex& Vertex,
		UBOOL bDebugThisDensityEstimation) const;

	/** Calculates exitant radiance at a vertex from the given photon map. */
	FLinearColor CalculatePhotonExitantRadiance(
		const FPhotonOctree& PhotonMap,
		INT NumPhotonsEmitted, 
		FLOAT SearchDistance,
		const FStaticLightingMesh* Mesh,
		const FStaticLightingVertex& Vertex,
		INT ElementIndex,
		const FVector4& OutgoingDirection,
		UBOOL bDebugThisDensityEstimation) const;

	/** Places volume lighting samples and calculates lighting for them. */
	void CalculateVolumeSamples();

	/** 
	 * Interpolates lighting from the volume lighting samples to a vertex. 
	 * This mirrors FPrecomputedLightVolume::InterpolateIncidentRadiance in UE3, used for visualizing interpolation from the lighting volume on surfaces.
	 */
	FGatheredLightSample InterpolatePrecomputedVolumeIncidentRadiance(
		const FStaticLightingVertex& Vertex, 
		FLOAT SampleRadius, 
		FCoherentRayCache& RayCache, 
		UBOOL bDebugThisTexel) const;

	/** Calculates incident radiance for a given world space position. */
	void CalculateVolumeSampleIncidentRadiance(
		const TArray<FVector4>& UniformHemisphereSamples,
		FVolumeLightingSample& LightingSample,
		FRandomStream& RandomStream,
		FStaticLightingMappingContext& MappingContext
		) const;

	/** Determines visibility cell placement, called once at startup. */
	void SetupPrecomputedVisibility();

	/** Calculates visibility for a given group of cells, called from all threads. */
	void CalculatePrecomputedVisibility(INT BucketIndex);

	/** Initializes DominantLightShadowInfo and prepares for multithreaded generation of DominantLightShadowInfo.ShadowMap. */
	void BeginCalculateDominantShadowInfo(FGuid LightGuid);

	/** Generates a single row of the dominant light shadow map. */
	void CalculateDominantShadowInfoWorkRange(INT ShadowMapY);

	/** Prepares for multithreaded generation of VolumeDistanceField. */
	void BeginCalculateVolumeDistanceField();

	/** Generates a single z layer of VolumeDistanceField. */
	void CalculateVolumeDistanceFieldWorkRange(INT ZIndex);

	/**
	 * Calculates shadowing for a given mapping surface point and light.
	 * @param Mapping - The mapping the point comes from.
	 * @param WorldSurfacePoint - The point to check shadowing at.
	 * @param Light - The light to check shadowing from.
	 * @param CacheGroup - The calling thread's collision cache.
	 * @return TRUE if the surface point is shadowed from the light.
	 */
	UBOOL CalculatePointShadowing(
		const FStaticLightingMapping* Mapping,
		const FVector4& WorldSurfacePoint,
		const FLight* Light,
		FStaticLightingMappingContext& MappingContext,
		UBOOL bDebugThisSample) const;

	/** Calculates area shadowing from a light for the given vertex. */
	INT CalculatePointAreaShadowing(
		const FStaticLightingMapping* Mapping,
		const FStaticLightingVertex& Vertex,
		INT ElementIndex,
		FLOAT SampleRadius,
		const FLight* Light,
		FStaticLightingMappingContext& MappingContext,
		FRandomStream& RandomStream,
		FLinearColor& UnnormalizedTransmission,
		const TArray<FLightSurfaceSample>& LightPositionSamples,
		UBOOL bDebugThisSample
		) const;

	/** Calculates the lighting contribution of a light to a mapping vertex. */
	FGatheredLightSample CalculatePointLighting(
		const FStaticLightingMapping* Mapping, 
		const FStaticLightingVertex& Vertex, 
		INT ElementIndex,
		const FLight* Light,
		const FLinearColor& InLightIntensity,
		const FLinearColor& InTransmission
		) const;

	/** Evaluates the PDF that was used to generate samples for the non-importance sampled final gather for the given direction. */
	FLOAT EvaluatePDF(const FStaticLightingVertex& Vertex, const FVector4& IncomingDirection) const;

	/** Returns environment lighting for the given direction. */
	FLinearColor EvaluateEnvironmentLighting(const FVector4& IncomingDirection) const;

	/** Returns a light sample that represents the material attribute specified by MaterialSettings.ViewMaterialAttribute at the intersection. */
	FGatheredLightSample GetVisualizedMaterialAttribute(const FStaticLightingMapping* Mapping, const FLightRayIntersection& Intersection) const;

	/** Calculates incident radiance due to direct illumination at a vertex. */
	FLinearColor CalculateIncidentDirectIllumination(
		const FStaticLightingMapping* Mapping,
		const FStaticLightingVertex& Vertex,
		INT BounceNumber,
		FStaticLightingMappingContext& MappingContext,
		FRandomStream& RandomStream,
		UBOOL bCalculateForIndirectLighting,
		UBOOL bDebugThisTexel) const;

	/** 
	 * Calculates exitant radiance due to direct illumination at a vertex. 
	 * This is only used with hemispherical sampling or path tracing, not when photon mapping is enabled.
	 */
	FLinearColor DirectIlluminationHemisphereSampling(
		const FStaticLightingMapping* SourceMapping,
		const FStaticLightingMesh* HitMesh,
		const FStaticLightingVertex& Vertex,
		INT ElementIndex,
		const FVector4& OutgoingDirection,
		INT BounceNumber,
		FStaticLightingMappingContext& MappingContext,
		FRandomStream& RandomStream,
		UBOOL bDebugThisTexel) const;

	/** 
	 * Calculates exitant radiance due to indirect illumination at a vertex. 
	 * This is only used with hemispherical sampling or path tracing, not when photon mapping is enabled.
	 */
	FLinearColor IndirectIlluminationHemisphericalSampling(
		const FStaticLightingMapping* SourceMapping,
		const FStaticLightingVertex& Vertex,
		INT ElementIndex,
		const FVector4& OutgoingDirection,
		INT BounceNumber,
		FStaticLightingMappingContext& MappingContext,
		FRandomStream& RandomStream,
		UBOOL bDebugThisTexel) const;

	/** Calculates exitant radiance at a vertex. */
	FLinearColor CalculateExitantRadiance(
		const FStaticLightingMapping* SourceMapping,
		const FStaticLightingMapping* HitMapping,
		const FStaticLightingMesh* HitMesh,
		const FStaticLightingVertex& Vertex,
		INT VertexIndex,
		INT ElementIndex,
		const FVector4& OutgoingDirection,
		INT BounceNumber,
		FStaticLightingMappingContext& MappingContext,
		FRandomStream& RandomStream,
		UBOOL bDebugThisTexel) const;

	/** Final gather using first bounce indirect photons to importance sample the incident radiance function. */
	FGatheredLightSample IncomingRadianceImportancePhotons(
		const FStaticLightingMapping* Mapping,
		const FStaticLightingVertex& Vertex,
		FLOAT SampleRadius,
		INT ElementIndex,
		INT BounceNumber,
		const TArray<FVector4>& ImportancePhotonDirections,
		FStaticLightingMappingContext& MappingContext,
		FRandomStream& RandomStream,
		UBOOL bDebugThisTexel) const;

	/** Final gather using uniform sampling to estimate the incident radiance function. */
	template<class SampleType>
	SampleType IncomingRadianceUniform(
		const FStaticLightingMapping* Mapping,
		const FStaticLightingVertex& Vertex,
		FLOAT SampleRadius,
		INT ElementIndex,
		INT BounceNumber,
		const TArray<FVector4>& UniformHemisphereSamples,
		const TArray<FVector4>& ImportancePhotonDirections,
		FStaticLightingMappingContext& MappingContext,
		FRandomStream& RandomStream,
		FLightingCacheGatherInfo& GatherInfo,
		UBOOL bDebugThisTexel) const;

	/** Calculates irradiance gradients for a sample position that will be cached. */
	void CalculateIrradianceGradients(
		INT BounceNumber,
		const FLightingCacheGatherInfo& GatherInfo,
		FVector4& RotationalGradient,
		FVector4& TranslationalGradient) const;

	/** 
	 * Interpolates incoming radiance from the lighting cache if possible,
	 * otherwise estimates incoming radiance for this sample point and adds it to the cache. 
	 */
	FFinalGatherSample CachePointIncomingRadiance(
		const FStaticLightingMapping* Mapping,
		const FStaticLightingVertex& Vertex,
		INT ElementIndex,
		INT VertexIndex,
		FLOAT SampleRadius,
		FStaticLightingMappingContext& MappingContext,
		FRandomStream& RandomStream,
		UBOOL bDebugThisTexel) const;

	/**
	 * Calculates the lighting contribution of all sky lights to a mapping vertex.
	 * @param Mapping - The mapping the vertex comes from.
	 * @param Vertex - The vertex to calculate the lighting contribution at.
	 * @param CacheGroup - The calling thread's collision cache.
	 * @return The incident area lighting on the vertex.
	 */
	FGatheredLightSample CalculateSkyLighting(
		const FStaticLightingMapping* Mapping,
		const FStaticLightingVertex& Vertex,
		FStaticLightingMappingContext& MappingContext,
		FRandomStream& RandomStream) const;

	/**
	 * Builds lighting for a vertex mapping.
	 * @param VertexMapping - The mapping to build lighting for.
	 */
	void ProcessVertexMapping(FStaticLightingVertexMapping* VertexMapping);

	/**
	 * Builds lighting for a texture mapping.
	 * @param TextureMapping - The mapping to build lighting for.
	 */
	void ProcessTextureMapping(FStaticLightingTextureMapping* TextureMapping);

	/** Calculates TexelToVertexMap and initializes each texel's light sample as mapped or not. */
	void SetupTextureMapping(
		FStaticLightingTextureMapping* TextureMapping, 
		FGatheredLightMapData2D& LightMapData, 
		class FTexelToVertexMap& TexelToVertexMap, 
		class FTexelToCornersMap& TexelToCornersMap,
		FStaticLightingMappingContext& MappingContext,
		UBOOL bDebugThisMapping) const;

	/** Calculates direct lighting as if all lights were non-area lights, then filters the results in texture space to create approximate soft shadows. */
	void CalculateDirectLightingTextureMappingFiltered(
		FStaticLightingTextureMapping* TextureMapping, 
		FStaticLightingMappingContext& MappingContext,
		FGatheredLightMapData2D& LightMapData, 
		TMap<const FLight*, FShadowMapData2D*>& ShadowMaps,
		const FTexelToVertexMap& TexelToVertexMap, 
		UBOOL bDebugThisMapping,
		const FLight* Light) const;

	/**
	 * Calculate lighting from area lights, no filtering in texture space.  
	 * Shadow penumbras will be correctly shaped and will be softer for larger light sources and distant shadow casters.
	 */
	void CalculateDirectAreaLightingTextureMapping(
		FStaticLightingTextureMapping* TextureMapping, 
		FStaticLightingMappingContext& MappingContext,
		FGatheredLightMapData2D& LightMapData, 
		TMap<const FLight*, FShadowMapData2D*>& ShadowMaps,
		const FTexelToVertexMap& TexelToVertexMap, 
		UBOOL bDebugThisMapping,
		const FLight* Light,
		const UBOOL bSimpleLightMapsOnly ) const;

	/** 
	 * Calculate signed distance field shadowing from a single light,  
	 * Based on the paper "Improved Alpha-Tested Magnification for Vector Textures and Special Effects" by Valve.
	 */
	void CalculateDirectSignedDistanceFieldLightingTextureMapping(
		FStaticLightingTextureMapping* TextureMapping, 
		FStaticLightingMappingContext& MappingContext,
		FGatheredLightMapData2D& LightMapData, 
		TMap<const FLight*, FSignedDistanceFieldShadowMapData2D*>& ShadowMaps,
		const FTexelToVertexMap& TexelToVertexMap, 
		const FTexelToCornersMap& TexelToCornersMap,
		UBOOL bDebugThisMapping,
		const FLight* Light) const;

	/**
	 * Estimate direct lighting using the direct photon map.
	 * This is only useful for debugging what the final gather rays see.
	 */
	void CalculateDirectLightingTextureMappingPhotonMap(
		FStaticLightingTextureMapping* TextureMapping, 
		FStaticLightingMappingContext& MappingContext,
		FGatheredLightMapData2D& LightMapData, 
		TMap<const FLight*, FShadowMapData2D*>& ShadowMaps,
		const FTexelToVertexMap& TexelToVertexMap, 
		UBOOL bDebugThisMapping) const;

	/** Handles indirect lighting calculations for a single texture mapping. */
	void CalculateIndirectLightingTextureMapping(
		FStaticLightingTextureMapping* TextureMapping,
		FStaticLightingMappingContext& MappingContext,
		FGatheredLightMapData2D& LightMapData, 
		const FTexelToVertexMap& TexelToVertexMap, 
		UBOOL bDebugThisMapping) const;

	/** Overrides LightMapData with material attributes if MaterialSettings.ViewMaterialAttribute != VMA_None */
	void ViewMaterialAttributesTextureMapping(
		FStaticLightingTextureMapping* TextureMapping,
		FStaticLightingMappingContext& MappingContext,
		FGatheredLightMapData2D& LightMapData, 
		const FTexelToVertexMap& TexelToVertexMap, 
		UBOOL bDebugThisMapping) const;

	/** Colors texels with invalid lightmap UVs to make it obvious that they are wrong. */
	void ColorInvalidLightmapUVs(
		const FStaticLightingTextureMapping* TextureMapping, 
		FGatheredLightMapData2D& LightMapData, 
		UBOOL bDebugThisMapping) const;

	/** Adds a texel of padding around texture mappings and copies the nearest texel into the padding. */
	void PadTextureMapping(
		const FStaticLightingTextureMapping* TextureMapping,
		const FGatheredLightMapData2D& LightMapData,
		FGatheredLightMapData2D& PaddedLightMapData,
		TMap<const FLight*, FShadowMapData2D*>& ShadowMaps,
		TMap<const FLight*, FSignedDistanceFieldShadowMapData2D*>& SignedDistanceFieldShadowMaps) const;

	/**
	 * Retrieves the next task from Swarm. Optionally blocking, thread-safe function call. Returns NULL when there are no more tasks.
	 * @param WaitTime The timeout period in milliseconds for the request
	 * @param bWaitTimedOut Output parameter; TRUE if the request timed out, FALSE if not
	 * @return	The next mapping task to process.
	 */
	FStaticLightingMapping*	ThreadGetNextMapping( 
		FThreadStatistics& 
		ThreadStatistics, 
		FGuid& TaskGuid, 
		DWORD WaitTime, 
		UBOOL& bWaitTimedOut, 
		UBOOL& bDynamicObjectTask, 
		INT& PrecomputedVisibilityTaskIndex,
		UBOOL& DominantShadowTask,
		UBOOL& bMeshAreaLightDataTask,
		UBOOL& bVolumeDataTask);

	/**
	 * The processing loop for a static lighting thread.
	 * @param bIsMainThread		- TRUE if this is running in the main thread.
	 * @param ThreadStatistics	- [out] Various thread statistics
	 */
	void ThreadLoop(UBOOL bIsMainThread, INT ThreadIndex, FThreadStatistics& ThreadStatistics);

	/**
	 * Creates multiple worker threads and starts the process locally.
	 */
	void MultithreadProcess();

	/** The lights in the world which the system is building, excluding sky lights. */
	TArray<FLight*> Lights;

	/** Mesh area lights in the world. */
	TIndirectArray<FMeshAreaLight> MeshAreaLights;

	/** The options the system is building lighting with. */
	const FLightingBuildOptions Options;

	/** Critical section to synchronize the access to Mappings (used only when GDebugMode is TRUE). */
	FCriticalSection CriticalSection;

	/** References to the scene's settings, for convenience */
	FStaticLightingSettings& GeneralSettings;
	FStaticLightingSceneConstants& SceneConstants;
	FSceneMaterialSettings& MaterialSettings;
	FMeshAreaLightSettings& MeshAreaLightSettings;
	FDynamicObjectSettings& DynamicObjectSettings;
	FPrecomputedVisibilitySettings& PrecomputedVisibilitySettings;
	FVolumeDistanceFieldSettings& VolumeDistanceFieldSettings;
	FAmbientOcclusionSettings& AmbientOcclusionSettings;
	FStaticShadowSettings& ShadowSettings;
	FImportanceTracingSettings& ImportanceTracingSettings;
	FPhotonMappingSettings& PhotonMappingSettings;
	FIrradianceCachingSettings& IrradianceCachingSettings;

	/** Stats of the system */
	mutable FStaticLightingStats Stats;

	/* Positive if the lighting threads are done writing to VolumeLightingSamples, and the samples can be exported. */
	volatile INT bVolumeLightingSamplesComplete;
	/** Bounds that VolumeLightingSamples were generated in. */
	FBoxSphereBounds VolumeBounds;
	/** Octree used for interpolating the volume lighting samples if DynamicObjectSettings.bVisualizeVolumeLightInterpolation is TRUE. */
	FVolumeLightingInterpolationOctree VolumeLightingInterpolationOctree;
	/** Map from LevelId to array of volume lighting samples generated. */
	TMap<INT,TArray<FVolumeLightingSample> > VolumeLightingSamples;

	/** All precomputed visibility cells in the scene.  Some of these may be processed on other agents. */
	TArray<FPrecomputedVisibilityCell> AllPrecomputedVisibilityCells;

	/** Positive if the lighting threads are done writing to DominantLightShadowInfo, and the samples can be exported. */
	volatile INT bDominantShadowTaskComplete;
	/** */
	volatile INT NumOutstandingDominantShadowColumns;
	/** Index of the last row of the dominant shadow map that was processed. */
	volatile INT OutstandingDominantShadowYIndex;
	/** Information about the dominant light's shadow that is needed by UE3. */
	FDominantLightShadowInfo DominantLightShadowInfo;

	/** Guid of the dominant directional light that DominantLightShadowInfo corresponds to. */
	FGuid DominantDirectionalLightId;

	/** Map from dominant spotlight to the shadowmap generated for that light. */
	TMap<const FSpotLight*, FDominantLightShadowInfo> DominantSpotLightShadowInfos;

	/** Threads must acquire this critical section before reading or writing to DominantSpotLightShadowInfos. */
	FCriticalSection DominantLightShadowSync;

	/** Non-zero if the mesh area light data task should be exported. */
	volatile INT bShouldExportMeshAreaLightData;

	/** Non-zero if the volume distance field task should be exported. */
	volatile INT bShouldExportVolumeDistanceField;

	/** Number of direct photons to emit */
	INT NumDirectPhotonsToEmit;
	/** Number of photons that were emitted until enough direct photons were gathered */
	INT NumPhotonsEmittedDirect;
	/** Photon map for direct photons */
	FPhotonOctree DirectPhotonMap;

	/** The target number of indirect photon paths to gather. */
	INT NumIndirectPhotonPaths;
	/** Number of indirect photons to emit */
	INT NumIndirectPhotonsToEmit;
	/** Number of photons that were emitted until enough first bounce photons were gathered */
	INT NumPhotonsEmittedFirstBounce;
	/** 
	 * Photon map for first bounce indirect photons.  
	 * This is separate from other indirect photons so we can access just first bounce photons and use them for guiding the final gather.
	 */
	FPhotonOctree FirstBouncePhotonMap;

	/** Number of photons that were emitted until enough second bounce photons were gathered */
	INT NumPhotonsEmittedSecondBounce;
	/** Photon map for second and up bounce photons. */
	FPhotonOctree SecondBouncePhotonMap;

	/** The target number of caustic photons to gather. */
	INT NumCausticPhotons;
	/** Number of photons that were emitted until enough caustic photons were gathered */
	INT NumPhotonsEmittedCaustic;
	/** Photon map for caustic photons, which only reflected off of specular surfaces before being deposited on a diffuse surface. */
	FPhotonOctree CausticPhotonMap;

	/** Fraction of direct photons deposited to calculate irradiance at. */
	FLOAT DirectIrradiancePhotonFraction;
	/** Fraction of indirect photons deposited to calculate irradiance at. */
	FLOAT IndirectIrradiancePhotonFraction;
	/** Photon map storing irradiance photons */
	FIrradiancePhotonOctree IrradiancePhotonMap;

	/** 
	 * Irradiance photons generated by photon emission.  
	 * Each array was generated on a separate thread, so these are stored as an array of irradiance photon arrays,
	 * Which avoids copying to one large array, since that can take a while due to the large irradiance photon memory size.
	 */
	TArray<TArray<FIrradiancePhoton>> IrradiancePhotons;

	/** Maximum distance to trace a ray through the scene. */
	FLOAT MaxRayDistance;

	/** Cached direction samples for hemisphere gathers, indexed by bounce number - 1. */
	TArray<TArray<FVector4> > CachedHemisphereSamples;

	/** The aggregate mesh used for raytracing. */
	FStaticLightingAggregateMesh AggregateMesh;
	
	/** The input scene describing geometry, materials and lights. */
	const FScene& Scene; 

	/** All meshes in the system. */
	TArray< FStaticLightingMesh* > Meshes;

	TArray<TArray<FStaticLightingMesh*> > VisibilityMeshes;

	/** All mappings in the system. */
	TArray< FStaticLightingMapping* > AllMappings;

	/** All mappings in the system for which lighting will be built. */
	TMap< FGuid, FStaticLightingMapping* > Mappings;

	/** The next index into Mappings which processing hasn't started for yet. */
	FThreadSafeCounter NextMappingToProcess;

	/** Stats on how many texels and vertices have been completed, written and read by all threads. */
	volatile INT NumTexelsCompleted;
	volatile INT NumVerticesCompleted;

	/** A list of the vertex mappings which static lighting has been computed for, but not yet applied.  This is accessed by multiple threads and should be written to using interlocked functions. */
	TCompleteStaticLightingList<FVertexMappingStaticLightingData> CompleteVertexMappingList;

	/** A list of the texture mappings which static lighting has been computed for, but not yet applied.  This is accessed by multiple threads and should be written to using interlocked functions. */
	TCompleteStaticLightingList<FTextureMappingStaticLightingData> CompleteTextureMappingList;

	/** List of complete visibility task data. */
	TCompleteTaskList<FPrecomputedVisibilityData> CompleteVisibilityTaskList;

	// Landscape mapping for Lighting Sample number estimation...
	TArray< FStaticLightingMapping* > LandscapeMappings;

	INT VolumeSizeX;
	INT VolumeSizeY;
	INT VolumeSizeZ;
	FLOAT DistanceFieldVoxelSize;
	FBox DistanceFieldVolumeBounds;
	TArray<FColor> VolumeDistanceField;

	/** */
	volatile INT NumOutstandingVolumeDataLayers;
	/** */
	volatile INT OutstandingVolumeDataLayerIndex;

	/** Triggered whenever a mapping is completed by a worker thread. */
	FEventWin CompletedMappingEvent;

	/** Number of threads to use for static lighting */
	const INT NumStaticLightingThreads;

	/** The threads spawned by the static lighting system for processing mappings. */
	TIndirectArray<FMappingProcessingThreadRunnable> Threads;

	/** Index of the next entry in DirectPhotonEmittingWorkRanges to process. */
	FThreadSafeCounter DirectPhotonEmittingWorkRangeIndex;
	TArray<FDirectPhotonEmittingWorkRange> DirectPhotonEmittingWorkRanges;
	TArray<FDirectPhotonEmittingOutput> DirectPhotonEmittingOutputs;

	/** Index of the next entry in IndirectPhotonEmittingWorkRanges to process. */
	FThreadSafeCounter IndirectPhotonEmittingWorkRangeIndex;
	TArray<FIndirectPhotonEmittingWorkRange> IndirectPhotonEmittingWorkRanges;
	TArray<FIndirectPhotonEmittingOutput> IndirectPhotonEmittingOutputs;

	/** Index of the next entry in IrradianceMarkWorkRanges to process. */
	FThreadSafeCounter IrradianceMarkWorkRangeIndex;
	TArray<FIrradianceMarkingWorkRange> IrradianceMarkWorkRanges;

	/** Index of the next entry in IrradianceCalculationWorkRanges to process. */
	FThreadSafeCounter IrradianceCalcWorkRangeIndex;
	TArray<FIrradianceCalculatingWorkRange> IrradianceCalculationWorkRanges;

	/** Index of the next mapping in AllMappings to cache irradiance photons on */
	FThreadSafeCounter NextMappingToCacheIrradiancePhotonsOn;
	/** Index into IrradiancePhotons of the array containing the photon being debugged, or INDEX_NONE if no photon is being debugged. */
	INT DebugIrradiancePhotonCalculationArrayIndex;
	/** Index into IrradiancePhotons(DebugIrradiancePhotonCalculationArrayIndex) of the photon being debugged. */
	INT DebugIrradiancePhotonCalculationPhotonIndex;


	/** Threads for caching irradiance photons. */
	TIndirectArray<FMappingProcessingThreadRunnable> IrradiancePhotonCachingThreads;

	/** Lightmass exporter (back to UE3) */
	class FLightmassSolverExporter& Exporter;

	// allow the vertex mapper access to the private functions
	friend class FStaticLightingVertexMappingProcessor;
	friend class FStaticLightingMappingContext;
	friend class FLightingCacheBase;
	friend class FStaticLightingThreadRunnable;
	friend class FDirectPhotonEmittingThreadRunnable;
	friend class FIndirectPhotonEmittingThreadRunnable;
	friend class FIrradiancePhotonMarkingThreadRunnable;
	friend class FIrradiancePhotonCalculatingThreadRunnable;
	friend class FMappingProcessingThreadRunnable;
	friend class FVolumeSamplePlacementRasterPolicy;
};

/**
 * Checks if a light is behind a triangle.
 * @param TrianglePoint - Any point on the triangle.
 * @param TriangleNormal - The (not necessarily normalized) triangle surface normal.
 * @param Light - The light to classify.
 * @return TRUE if the light is behind the triangle.
 */
extern UBOOL IsLightBehindSurface(const FVector4& TrianglePoint, const FVector4& TriangleNormal, const FLight* Light);

/**
 * Culls lights that are behind a triangle.
 * @param bTwoSidedMaterial - TRUE if the triangle has a two-sided material.  If so, lights behind the surface are not culled.
 * @param TrianglePoint - Any point on the triangle.
 * @param TriangleNormal - The (not necessarily normalized) triangle surface normal.
 * @param Lights - The lights to cull.
 * @return A map from Lights index to a boolean which is TRUE if the light is in front of the triangle.
 */
extern TBitArray<> CullBackfacingLights(UBOOL bTwoSidedMaterial, const FVector4& TrianglePoint, const FVector4& TriangleNormal, const TArray<FLight*>& Lights);

} //namespace Lightmass

