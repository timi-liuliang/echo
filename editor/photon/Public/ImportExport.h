/*=============================================================================
	ImportExport.h: Import into UE3 export data definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once
namespace Lightmass
{	
/** Whether or not to request compression on heavyweight input file types */
#define LM_COMPRESS_INPUT_DATA 1

/** Whether or not to compress output lightmap data before sending back over Swarm */
#define LM_COMPRESS_OUTPUT_DATA 1

	/** The number of coefficients that are stored for each light sample. */ 
	static const INT LM_NUM_STORED_LIGHTMAP_COEF = 3;
	/** The number of directional coefficients which the lightmap stores for each light sample. */ 
	static const INT LM_NUM_DIRECTIONAL_LIGHTMAP_COEF = 2;
	/** The index at which simple coefficients are stored in any array containing all LM_NUM_STORED_LIGHTMAP_COEF coefficients. */ 
	static const INT LM_SIMPLE_LIGHTMAP_COEF_INDEX = 2;

	/** Output channel types (extensions) */
	static const TCHAR LM_TEXTUREMAPPING_EXTENSION[]	= TEXT("tmap");
	static const TCHAR LM_VERTEXMAPPING_EXTENSION[]		= TEXT("vmap");
	static const TCHAR LM_VOLUMESAMPLES_EXTENSION[]		= TEXT("vols");
	static const TCHAR LM_VOLUMEDEBUGOUTPUT_EXTENSION[]	= TEXT("vold");
	static const TCHAR LM_PRECOMPUTEDVISIBILITY_EXTENSION[]	= TEXT("vis");
	static const TCHAR LM_DOMINANTSHADOW_EXTENSION[]	= TEXT("doms");
	static const TCHAR LM_MESHAREALIGHTDATA_EXTENSION[]	= TEXT("arealights");
	static const TCHAR LM_DEBUGOUTPUT_EXTENSION[]		= TEXT("dbgo");

	/** Input channel types (extensions) */
#if LM_COMPRESS_INPUT_DATA
	static const TCHAR LM_SCENE_EXTENSION[]				= TEXT("scenegz");
	static const TCHAR LM_STATICMESH_EXTENSION[]		= TEXT("meshgz");
	static const TCHAR LM_TERRAIN_EXTENSION[]			= TEXT("terrgz");
	static const TCHAR LM_SPEEDTREE_EXTENSION[]			= TEXT("speedgz");
	static const TCHAR LM_MATERIAL_EXTENSION[]			= TEXT("mtrlgz");
#else
	static const TCHAR LM_SCENE_EXTENSION[]				= TEXT("scene");
	static const TCHAR LM_STATICMESH_EXTENSION[]		= TEXT("mesh");
	static const TCHAR LM_TERRAIN_EXTENSION[]			= TEXT("terr");
	static const TCHAR LM_SPEEDTREE_EXTENSION[]			= TEXT("speed");
	static const TCHAR LM_MATERIAL_EXTENSION[]			= TEXT("mtrl");
#endif

	/** Channel versions */

#if LM_COMPRESS_OUTPUT_DATA
	static const FGuid LM_TEXTUREMAPPING_VERSION	= FGuid(0x0d6d0c6c, 0x84d14c54, 0xa0c4773e, 0x3dc9cdd2);
	static const FGuid LM_VERTEXMAPPING_VERSION		= FGuid(0x6bd8f62d, 0x896f453c, 0xa9465252, 0x0b2f9d4a);
#else
	static const FGuid LM_TEXTUREMAPPING_VERSION	= FGuid(0xfded0471, 0xb9354fb9, 0x8ec567e0, 0xfceaf610);
	static const FGuid LM_VERTEXMAPPING_VERSION		= FGuid(0x979c98fe, 0x8b39456f, 0xbc149f30, 0xcacb9d1a);
#endif

	static const FGuid LM_VOLUMESAMPLES_VERSION		= FGuid(0x2f34c6db, 0xfe6144d8, 0x8f2bf84b, 0x0f941365);
	static const FGuid LM_PRECOMPUTEDVISIBILITY_VERSION	= FGuid(0x344a430b, 0x0e674a43, 0xb0883654, 0x46251171);
	static const FGuid LM_VOLUMEDEBUGOUTPUT_VERSION	= FGuid(0xb4bb7675, 0x185042b5, 0xaa5244b3, 0x6b7b561d);
	static const FGuid LM_DOMINANTSHADOW_VERSION	= FGuid(0x153eeae2, 0x51624c71, 0x81f129aa, 0xcdc9d364);
	static const FGuid LM_MESHAREALIGHTDATA_VERSION	= FGuid(0x6deaad7f, 0xaeda4f9e, 0x9997bb51, 0x5fe4bded);
	static const FGuid LM_DEBUGOUTPUT_VERSION		= FGuid(0x133c46ae, 0x0e3f4c60, 0xb3a33b6f, 0x406e5d5f);
	static const FGuid LM_SCENE_VERSION				= FGuid(0xfe50c828, 0x56ee4777, 0xbcbc6c90, 0x0a5426d3);
	static const FGuid LM_STATICMESH_VERSION		= FGuid(0x581b2a30, 0xa75b4611, 0x967a7389, 0xf386fb24);
	static const FGuid LM_TERRAIN_VERSION			= FGuid(0x2f7c902b, 0x7b7d4f2d, 0xa53774c5, 0x04301938);
	static const FGuid LM_SPEEDTREE_VERSION			= FGuid(0x7391ba85, 0x3a9d4ffb, 0x9a1410d3, 0xf2651d61);
	static const FGuid LM_MATERIAL_VERSION			= FGuid(0x18286259, 0x5b4742c7, 0xb23fd922, 0x131b2675);	

	/** Alert source object type identifiers... */
	enum ESourceObjectType
	{
		SOURCEOBJECTTYPE_Unknown = 0,
		SOURCEOBJECTTYPE_Scene,
		SOURCEOBJECTTYPE_Material,
		SOURCEOBJECTTYPE_BSP,
		SOURCEOBJECTTYPE_StaticMesh,
		SOURCEOBJECTTYPE_Terrain,
		SOURCEOBJECTTYPE_Fluid,
		SOURCEOBJECTTYPE_SpeedTree,
		SOURCEOBJECTTYPE_TextureMapping,
		SOURCEOBJECTTYPE_VertexMapping,
		SOURCEOBJECTTYPE_Mapping
	};

	#pragma pack(push, 1)
	
	/** 
	 * Incident lighting for a single sample, as produced by a lighting build. 
	 * FGatheredLightSample is used for gathering lighting instead of this format as FLightSampleData is not additive.
	 */
	struct FLightSampleData
	{
		FLightSampleData()
		{
			bIsMapped = FALSE;
			appMemzero(Coefficients,sizeof(Coefficients));
		}

		/** 
		 * Coefficients[0] stores the normalized average color, 
		 * Coefficients[1] stores the maximum color component in each lightmap basis direction,
		 * and Coefficients[2] stores the simple lightmap which is colored incident lighting along the vertex normal.
		 */
		FLOAT Coefficients[LM_NUM_STORED_LIGHTMAP_COEF][3];

		/** True if this sample maps to a valid point on a triangle.  This is only meaningful for texture lightmaps. */
		UBOOL bIsMapped;

		/**
		 * Export helper
		 * @param Component Which directional lightmap component to retrieve
		 * @return An FColor for this component, clamped to White
		 */
		FColor GetColor(INT Component) const
		{
			return FColor(
				(BYTE)Clamp<INT>((INT)(Coefficients[Component][0] * 255), 0, 255), 
				(BYTE)Clamp<INT>((INT)(Coefficients[Component][1] * 255), 0, 255), 
				(BYTE)Clamp<INT>((INT)(Coefficients[Component][2] * 255), 0, 255),
				0);
		}
	};

	/**
	 * The quantized coefficients for a single light-map texel.
	 */
	struct FQuantizedLightSampleData
	{
		BYTE Coverage;
		BYTE Coefficients[LM_NUM_STORED_LIGHTMAP_COEF][3];
	};

	struct FLightMapDataBase
	{
#if LM_COMPRESS_OUTPUT_DATA
		/** Size of compressed lightmap data */
		UINT CompressedDataSize;

		/** Size of uncompressed lightmap data */
		UINT UncompressedDataSize;
#endif

		/** Scale applied to the quantized light samples */
		FLOAT Scale[LM_NUM_STORED_LIGHTMAP_COEF][3];
	};

	/** LightMap data 1D */
	struct FLightMapData1DData : public FLightMapDataBase
	{
		FLightMapData1DData(INT InSamples)
		{
			NumSamples = InSamples;
		}

		/** The samples for this light map */
		INT NumSamples;
	};

	/** LightMap data 2D */
	struct FLightMapData2DData : public FLightMapDataBase
	{
		FLightMapData2DData(UINT InSizeX,UINT InSizeY):
			SizeX(InSizeX),
			SizeY(InSizeY)
		{
		}

		/** The width of the light-map. */
		UINT SizeX;
		/** The height of the light-map. */
		UINT SizeY;
	};

	struct FShadowMapDataBase
	{
#if LM_COMPRESS_OUTPUT_DATA
		/** Size of compressed shadowmap data */
		UINT CompressedDataSize;

		/** Size of uncompressed shadowmap data */
		UINT UncompressedDataSize;
#endif
	};

	/** ShadowMap data 1D */
	struct FShadowMapData1DData : public FShadowMapDataBase
	{
		FShadowMapData1DData(INT InNumSamples) :
			NumSamples(InNumSamples)
		{
		}

		/** The occlusion samples for a 1D array of points. */
		INT NumSamples;
	};

	/** A sample of the visibility factor between a light and a single point. */
	struct FShadowSampleData
	{
		/** The fraction of light that reaches this point from the light, between 0 and 1. */
		FLOAT Visibility;
		/** True if this sample maps to a valid point on a surface. */
		UBOOL bIsMapped;

		/**
		 * Export helper
		 * @param Component Which directional lightmap component to retrieve
		 * @return An FColor for this component, clamped to White
		 */
		FColor GetColor(INT Component) const
		{
			BYTE Gray = (BYTE)Clamp<INT>((INT)(Visibility * 255.f), 0, 255);
			return FColor(Gray, Gray, Gray, 0);
		}

	};

	/** The quantized value for a single shadowmap texel */
	struct FQuantizedShadowSampleData
	{
		BYTE Visibility;
		BYTE Coverage;
	};

	/** ShadowMap data 2D */
	struct FShadowMapData2DData : public FShadowMapDataBase
	{
		FShadowMapData2DData(UINT InSizeX,UINT InSizeY):
			SizeX(InSizeX),
			SizeY(InSizeY)
		{
		}

		/** The width of the shadow-map. */
		UINT SizeX;
		/** The height of the shadow-map. */
		UINT SizeY;
	};

	struct FSignedDistanceFieldShadowSampleData
	{
		/** Normalized and encoded distance to the nearest shadow transition, in the range [0,1], where .5 is the transition. */
		FLOAT Distance;
		/** Normalized penumbra size, in [0,1] */
		FLOAT PenumbraSize;
		/** True if this sample maps to a valid point on a surface. */
		UBOOL bIsMapped;
	};

	/** The quantized value for a single signed-distance field texel */
	struct FQuantizedSignedDistanceFieldShadowSampleData
	{
		BYTE Distance;
		BYTE PenumbraSize;
		BYTE Coverage;
	};

	/** 2D distance field data. */
	struct FSignedDistanceFieldShadowMapData2DData : public FShadowMapData2DData
	{
		FSignedDistanceFieldShadowMapData2DData(UINT InSizeX,UINT InSizeY):
			FShadowMapData2DData(InSizeX, InSizeY)
		{
		}
	};

	/** Lighting for a point in space. */
	class FVolumeLightingSampleData
	{
	public:
		/** World space position and radius. */
		FVector4 PositionAndRadius;
		/** Average direction of the incoming indirect lighting. */
		FVector4 IndirectDirection;
		/** Average direction of the incoming environment lighting. */
		FVector4 EnvironmentDirection;
		/** RGBE radiance of indirect lighting along IndirectDirection. */
		FColor IndirectRadiance;
		/** RGBE radiance of environment lighting along IndirectDirection. */
		FColor EnvironmentRadiance;
		/** RGBE indirect and environment lighting along other directions. */
		FColor AmbientRadiance;
		/** Whether the sample is shadowed by all affecting dominant lights. */
		BYTE bShadowedFromDominantLights;
	};

	/** Distance to the nearest shadow casting geometry for a single dominant light shadow map cell. */
	class FDominantLightShadowSampleData
	{
	public:
		WORD Distance;
	};

	class FDominantLightShadowInfoData
	{
	public:
		/** Transform from world space to the coordinate space that FDominantLightShadowSampleData's are stored in. */
		FMatrix WorldToLight;
		/** Light space bounds of the importance volume. */
		FVector4 LightSpaceImportanceBoundMin;
		FVector4 LightSpaceImportanceBoundMax;
		/** Dimensions of the generated shadow map. */
		INT ShadowMapSizeX;
		INT ShadowMapSizeY;
	};

	static const FGuid MeshAreaLightDataGuid = FGuid(0xe11f4760, 0xfa454d2b, 0xa090c388, 0x33326646);

	static const FGuid VolumeDistanceFieldGuid = FGuid(0x4abf306e, 0x4c2f4a6e, 0x9feb5fa4, 0x5b910a8f);
	
	class FMeshAreaLightData
	{
	public:
		INT LevelId;
		FVector4 Position;
		FVector4 Direction;
		FLOAT Radius;
		FLOAT ConeAngle;
		FColor Color;
		FLOAT Brightness;
		FLOAT FalloffExponent;
	};

	/** 
	 * Types used for transfering debug information back to UE3. 
	 * NOTE: These must stay binary compatible with the corresponding versions in UE3.
	 */

	struct FDebugStaticLightingRay
	{
		FVector4 Start;
		FVector4 End;
		UBOOL bHit;
		UBOOL bPositive;

		FDebugStaticLightingRay() {}
		FDebugStaticLightingRay(const FVector4& InStart, const FVector4& InEnd, UBOOL bInHit, UBOOL bInPositive = FALSE) :
			Start(InStart),
			End(InEnd),
			bHit(bInHit),
			bPositive(bInPositive)
		{}
	};

	struct FDebugStaticLightingVertex
	{
		FVector4 VertexNormal;
		FVector4 VertexPosition;
	};

	struct FDebugLightingCacheRecord
	{
		UBOOL bNearSelectedTexel;
		UBOOL bAffectsSelectedTexel;
		INT RecordId;
		FDebugStaticLightingVertex Vertex;
		FLOAT Radius;

		FDebugLightingCacheRecord() :
			bAffectsSelectedTexel(FALSE)
		{}
	};

	struct FDebugPhoton
	{
		INT Id;
		FVector4 Position;
		FVector4 Direction;
		FVector4 Normal;

		FDebugPhoton() :
			Id(-1)
		{}

		FDebugPhoton(INT InId, const FVector4& InPosition, const FVector4& InDirection, const FVector4& InNormal) :
			Id(InId),
			Position(InPosition),
			Direction(InDirection),
			Normal(InNormal)
		{}
	};

	struct FDebugOctreeNode
	{
		FVector4 Center;
		FVector4 Extent;

		FDebugOctreeNode(const FVector4& InCenter, const FVector4& InExtent) :
			Center(InCenter),
			Extent(InExtent)
		{}
	};

	/** Guid used by UE3 to determine when the volume lighting debug channel with the same Guid can be opened. */
	static const FGuid VolumeLightingDebugOutputGuid = FGuid(0x1e8119ff, 0xa46f48f8, 0x92b18d49, 0x172c5832);
	/** Guid used by UE3 to determine when the volume lighting sample channel with the same Guid can be opened. */
	static const FGuid PrecomputedVolumeLightingGuid = FGuid(0xce97c5c3, 0xab614fd3, 0xb2da55c0, 0xe6c33fb4);

	#pragma pack(pop)

} // namespace Lightmass
