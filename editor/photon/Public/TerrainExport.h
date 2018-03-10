/*=============================================================================
	TerrainExport.h: Terrain export data definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

#pragma pack(push, 1)

//----------------------------------------------------------------------------
//	Helper definitions
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//	Mesh export file header
//----------------------------------------------------------------------------
struct FTerrainFileHeader
{
	/** FourCC cookie: 'TERR' */
	UINT		Cookie;
	FGuid		FormatVersion;

	// These structs follow immediately after this struct.
	//
	//	FBaseTerrainData		BaseData;
	//	FTerrainData			TerrainData;
	//
};


//----------------------------------------------------------------------------
//	Base mesh
//----------------------------------------------------------------------------
struct FBaseTerrainData
{
	FGuid		Guid;
};

//----------------------------------------------------------------------------
//	Static mesh, builds upon FBaseMeshData
//----------------------------------------------------------------------------
struct FTerrainData
{
	FMatrix LocalToWorld;
	FMatrix WorldToLocal;
    FVector4 Location;
    FVector4 Rotation;
    FVector4 DrawScale3D;
    FLOAT DrawScale;
    INT NumSectionsX;
    INT NumSectionsY;
    INT SectionSize;
    FLOAT MaxCollisionDisplacement;
    INT MaxTesselationLevel;
    INT MinTessellationLevel;
    FLOAT TesselationDistanceScale;
    FLOAT TessellationCheckDistance;
    INT NumVerticesX;
    INT NumVerticesY;
    INT NumPatchesX;
    INT NumPatchesY;
    INT MaxComponentSize;
    INT StaticLightingResolution;
    UBOOL bIsOverridingLightResolution;
    UBOOL bBilinearFilterLightmapGeneration;
    UBOOL bCastShadow;
    UBOOL bForceDirectLightMap;
    UBOOL bCastDynamicShadow;
    UBOOL bAcceptsDynamicLights;

	INT NumMaterials;
	INT NumCollisionVertices;

	// The 'array' data will follow
//	TArray<FTerrainHeight> Heights;				- WORD
//	TArrayNoInit<FTerrainInfoData> InfoData;	- BYTE
//	TArrayNoInit<BYTE> CachedDisplacements;		- BYTE
//	TArray<FGuid> TerrainMaterials;				- GUID
};


#pragma pack(pop)

}	// namespace Lightmass
