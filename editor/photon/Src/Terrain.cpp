/*=============================================================================
	Terrain.cpp: Terrain classes implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "stdafx.h"
#include "Terrain.h"
#include "Importer.h"

namespace Lightmass
{

//----------------------------------------------------------------------------
//	Terrain base class
//----------------------------------------------------------------------------

void FBaseTerrain::Import( class FLightmassImporter& Importer )
{
	Importer.ImportData((FBaseTerrainData*)this);
}

//----------------------------------------------------------------------------
//	Terrain class
//----------------------------------------------------------------------------

void FTerrain::Import( class FLightmassImporter& Importer )
{
	// import super class
	FBaseTerrain::Import(Importer);

	// import the shared data structure
//	Importer.ImportData((FTerrainData*)this);
// 	INT CheckSize = sizeof(FTerrainData);
// 	FTerrainData* TerrainData = (FTerrainData*)this;
// 	Importer.ImportData(TerrainData);

	FTerrainData TempData;
 	Importer.ImportData(&TempData);

	LocalToWorld = TempData.LocalToWorld;
	WorldToLocal = TempData.WorldToLocal;
	Location = TempData.Location;
	Rotation = TempData.Rotation;
	DrawScale3D = TempData.DrawScale3D;
	DrawScale = TempData.DrawScale;
	NumSectionsX = TempData.NumSectionsX;
	NumSectionsY = TempData.NumSectionsY;
	SectionSize = TempData.SectionSize;
	MaxCollisionDisplacement = TempData.MaxCollisionDisplacement;
	MaxTesselationLevel = TempData.MaxTesselationLevel;
	MinTessellationLevel = TempData.MinTessellationLevel;
	TesselationDistanceScale = TempData.TesselationDistanceScale;
	TessellationCheckDistance = TempData.TessellationCheckDistance;
	NumVerticesX = TempData.NumVerticesX;
	NumVerticesY = TempData.NumVerticesY;
	NumPatchesX = TempData.NumPatchesX;
	NumPatchesY = TempData.NumPatchesY;
	MaxComponentSize = TempData.MaxComponentSize;
	StaticLightingResolution = TempData.StaticLightingResolution;
	bIsOverridingLightResolution = TempData.bIsOverridingLightResolution;
	bBilinearFilterLightmapGeneration = TempData.bBilinearFilterLightmapGeneration;
	bCastShadow = TempData.bCastShadow;
	bForceDirectLightMap = TempData.bForceDirectLightMap;
	bCastDynamicShadow = TempData.bCastDynamicShadow;
	bAcceptsDynamicLights = TempData.bAcceptsDynamicLights;
	NumMaterials = TempData.NumMaterials;
	NumCollisionVertices = TempData.NumCollisionVertices;

	if (Heights.Num() != 0)
	{
		debugf(TEXT("Terrain Import Overwrite - should have read %d bytes!!!"), sizeof(FTerrainData));
	}
	//@lmtodo. BUG IN IMPORTDATA CALL ABOVE???
	// Ends up stepping on the Heights array...
	appMemzero(&Heights, sizeof(TArray<WORD>));
	// Caused by alignment issues between UE3 and LM?
	// Packing is definied differently between the two??

	INT NumHeights = NumVerticesX * NumVerticesY;
	Importer.ImportArray(Heights, NumHeights);
	Importer.ImportArray(InfoData, NumHeights);
	Importer.ImportArray(CachedDisplacements, NumHeights);
	Importer.ImportArray(TerrainMaterials, NumMaterials);
	Importer.ImportArray(CollisionVertices, NumCollisionVertices);

	//Dump();
}

void FTerrain::Dump() const
{
	debugf(TEXT("TERRAIN DUMP"));
	debugf(TEXT("\tGuid                             : %s"), *(Guid.String()));
	debugf(TEXT("\tLocalToWorld                     : "));
	debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), LocalToWorld.M[0][0], LocalToWorld.M[0][1], LocalToWorld.M[0][2], LocalToWorld.M[0][3]);
	debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), LocalToWorld.M[1][0], LocalToWorld.M[1][1], LocalToWorld.M[1][2], LocalToWorld.M[1][3]);
	debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), LocalToWorld.M[2][0], LocalToWorld.M[2][1], LocalToWorld.M[2][2], LocalToWorld.M[2][3]);
	debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), LocalToWorld.M[3][0], LocalToWorld.M[3][1], LocalToWorld.M[3][2], LocalToWorld.M[3][3]);
	debugf(TEXT("\tWorldToLocal                     : "));
	debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), WorldToLocal.M[0][0], WorldToLocal.M[0][1], WorldToLocal.M[0][2], WorldToLocal.M[0][3]);
	debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), WorldToLocal.M[1][0], WorldToLocal.M[1][1], WorldToLocal.M[1][2], WorldToLocal.M[1][3]);
	debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), WorldToLocal.M[2][0], WorldToLocal.M[2][1], WorldToLocal.M[2][2], WorldToLocal.M[2][3]);
	debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), WorldToLocal.M[3][0], WorldToLocal.M[3][1], WorldToLocal.M[3][2], WorldToLocal.M[3][3]);
	debugf(TEXT("\tLocation                         : %-8.5f,%-8.5f,%-8.5f,%-8.5f"), Location.X, Location.Y, Location.Z, Location.W);
    debugf(TEXT("\tRotation                         : %-8.5f,%-8.5f,%-8.5f,%-8.5f"), Rotation.X, Rotation.Y, Rotation.Z, Rotation.W);
    debugf(TEXT("\tDrawScale3D                      : %-8.5f,%-8.5f,%-8.5f,%-8.5f"), DrawScale3D.X, DrawScale3D.Y, DrawScale3D.Z, DrawScale3D.W);
    debugf(TEXT("\tDrawScale                        : %-8.5f"), DrawScale);
    debugf(TEXT("\tNumSectionsX                     : %d"), NumSectionsX);
    debugf(TEXT("\tNumSectionsY                     : %d"), NumSectionsY);
    debugf(TEXT("\tSectionSize                      : %d"), SectionSize);
    debugf(TEXT("\tMaxCollisionDisplacement         : %-8.5f"), MaxCollisionDisplacement);
    debugf(TEXT("\tMaxTesselationLevel              : %d"), MaxTesselationLevel);
    debugf(TEXT("\tMinTessellationLevel             : %d"), MinTessellationLevel);
    debugf(TEXT("\tTesselationDistanceScale         : %-8.5f"), TesselationDistanceScale);
    debugf(TEXT("\tTessellationCheckDistance        : %-8.5f"), TessellationCheckDistance);
    debugf(TEXT("\tNumVerticesX                     : %d"), NumVerticesX);
    debugf(TEXT("\tNumVerticesY                     : %d"), NumVerticesY);
    debugf(TEXT("\tNumPatchesX                      : %d"), NumPatchesX);
    debugf(TEXT("\tNumPatchesY                      : %d"), NumPatchesY);
    debugf(TEXT("\tMaxComponentSize                 : %d"), MaxComponentSize);
    debugf(TEXT("\tStaticLightingResolution         : %d"), StaticLightingResolution);
	debugf(TEXT("\tbIsOverridingLightResolution     : %s"), bIsOverridingLightResolution ? TEXT("TRUE") : TEXT("FALSE"));
    debugf(TEXT("\tbBilinearFilterLightmapGeneration: %s"), bBilinearFilterLightmapGeneration ? TEXT("TRUE") : TEXT("FALSE"));
    debugf(TEXT("\tbCastShadow                      : %s"), bCastShadow ? TEXT("TRUE") : TEXT("FALSE"));
    debugf(TEXT("\tbForceDirectLightMap             : %s"), bForceDirectLightMap ? TEXT("TRUE") : TEXT("FALSE"));
    debugf(TEXT("\tbCastDynamicShadow               : %s"), bCastDynamicShadow ? TEXT("TRUE") : TEXT("FALSE"));
    debugf(TEXT("\tbAcceptsDynamicLights            : %s"), bAcceptsDynamicLights ? TEXT("TRUE") : TEXT("FALSE"));
	debugf(TEXT("\tNumMaterials                     : %d"), NumMaterials);
	debugf(TEXT("\tNumCollisionVertices             : %d"), NumCollisionVertices);

	FString DebugOut;
	debugf(TEXT("\tHeights                          : %d"), Heights.Num());
	DebugOut = TEXT("\t\t");
	for (INT Idx = 0; Idx < Heights.Num(); Idx++)
	{
		DebugOut += FString::Printf(TEXT("%4d,"), Heights(Idx));
		if ((Idx + 1) % 8 == 0)
		{
			debugf(*DebugOut);
			DebugOut = TEXT("\t\t");
		}
	}

	debugf(TEXT("\tInfoData                        : %d"), InfoData.Num());
	DebugOut = TEXT("\t\t");
	for (INT Idx = 0; Idx < InfoData.Num(); Idx++)
	{
		DebugOut += FString::Printf(TEXT("0x%02x,"), InfoData(Idx));
		if ((Idx + 1) % 16 == 0)
		{
			debugf(*DebugOut);
			DebugOut = TEXT("\t\t");
		}
	}

	debugf(TEXT("\tCachedDisplacements             : %d"), CachedDisplacements.Num());
	DebugOut = TEXT("\t\t");
	for (INT Idx = 0; Idx < CachedDisplacements.Num(); Idx++)
	{
		DebugOut += FString::Printf(TEXT("0x%02x,"), CachedDisplacements(Idx));
		if ((Idx + 1) % 16 == 0)
		{
			debugf(*DebugOut);
			DebugOut = TEXT("\t\t");
		}
	}

//	TArray<FGuid>		TerrainMaterials;

//	TArray<FVector4>	CollisionVertices;
	debugf(TEXT("\tCollisionVertices                : %d"), CollisionVertices.Num());
	for (INT Idx = 0; Idx < CollisionVertices.Num(); Idx++)
	{
		debugf(TEXT("\t\t%5d: %-8.5f,%-8.5f,%-8.5f"), 
			Idx,
			CollisionVertices(Idx).X, CollisionVertices(Idx).Y, CollisionVertices(Idx).Z);
	}

	debugf(TEXT("END TERRAIN DUMP"));
}

}	// namespace Lightmass


