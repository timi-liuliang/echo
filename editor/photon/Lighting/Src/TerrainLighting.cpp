/*=============================================================================
	TerrainLighting.cpp: Static lighting Terrain mesh/mapping definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Importer.h"
#include "Terrain.h"

namespace Lightmass
{
	#define TERRAIN_ZSCALE				(1.0f/128.0f)
	#define TERRAIN_MAXTESSELATION		16

	/** Destructor. */
	FTerrainComponentStaticLighting::~FTerrainComponentStaticLighting()
	{
	}

	/** The surface's vertices. */
	FStaticLightingVertex FTerrainComponentStaticLighting::GetVertex(INT X, INT Y) const
	{
		FStaticLightingVertex Result;

		const INT PatchX = SectionBaseX + X;
		const INT PatchY = SectionBaseY + Y;

		FVector4 WorldTangentX;
		FVector4 WorldTangentY;
		FVector4 WorldTangentZ;
		FLOAT Displacement;

		FLOAT Z = (FLOAT)Terrain->Height(PatchX, PatchY);
		const FLOAT HeightNegX = (FLOAT)Terrain->Height(PatchX - 1,PatchY);
		const FLOAT HeightPosX = (FLOAT)Terrain->Height(PatchX + 1,PatchY);
		const FLOAT HeightNegY = (FLOAT)Terrain->Height(PatchX,PatchY - 1);
		const FLOAT HeightPosY = (FLOAT)Terrain->Height(PatchX,PatchY + 1);
		const FLOAT SampleDerivX = (HeightPosX - HeightNegX) / (FLOAT)1.0f / 2.0f;
		const FLOAT SampleDerivY = (HeightPosY - HeightNegY) / (FLOAT)1.0f / 2.0f;
		FVector4 TempWorldTangentX = FVector4(1,0,SampleDerivX * TERRAIN_ZSCALE,1.0f);
		FVector4 TempWorldTangentY = FVector4(0,1,SampleDerivY * TERRAIN_ZSCALE,1.0f);

		WorldTangentX = LocalToWorld.TransformNormal(TempWorldTangentX).SafeNormal();
		WorldTangentY = LocalToWorld.TransformNormal(TempWorldTangentY).SafeNormal();
		WorldTangentZ = (WorldTangentX ^ WorldTangentY).SafeNormal();

		Displacement = Terrain->GetCachedDisplacement(PatchX,PatchY,0,0);

		INT ClampedLocalX = Clamp(PatchX,0,Terrain->NumVerticesX - 1) - SectionBaseX;
		INT ClampedLocalY = Clamp(PatchY,0,Terrain->NumVerticesY - 1) - SectionBaseY;

		Result.WorldPosition = LocalToWorld.TransformFVector(FVector4(ClampedLocalX,ClampedLocalY,(-32768.0f + Z) * TERRAIN_ZSCALE,1.0f)) + WorldTangentZ * Displacement;

		Result.WorldTangentX = WorldTangentX;
		Result.WorldTangentY = WorldTangentY;
		Result.WorldTangentZ = WorldTangentZ;

		check((X + ExpandQuadsX) >= 0);
		check((Y + ExpandQuadsY) >= 0);

		// Need to remap the UVs to a 0-1 range for the material texture lookup
		Result.TextureCoordinates[0] = FVector2D((FLOAT)PatchX/Terrain->NumPatchesX,(FLOAT)PatchY/Terrain->NumPatchesY);
		Result.TextureCoordinates[1].X = ((X + ExpandQuadsX) * Terrain->StaticLightingResolution + 0.5f) / (FLOAT)TextureMapping.CachedSizeX;
		Result.TextureCoordinates[1].Y = ((Y + ExpandQuadsY) * Terrain->StaticLightingResolution + 0.5f) / (FLOAT)TextureMapping.CachedSizeY;

		return Result;
	}

	// FStaticLightingMesh interface.
	void FTerrainComponentStaticLighting::GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const
	{
		const INT QuadIndex = TriangleIndex / 2;
		const FIntPoint& QuadCoordinates = QuadIndexToCoordinatesMap(QuadIndex);

		if (Terrain->IsTerrainQuadFlipped(QuadCoordinates.X, QuadCoordinates.Y) == FALSE)
		{
			if(TriangleIndex & 1)
			{
				OutV0 = GetVertex(QuadCoordinates.X + 0,QuadCoordinates.Y + 0);
				OutV1 = GetVertex(QuadCoordinates.X + 0,QuadCoordinates.Y + 1);
				OutV2 = GetVertex(QuadCoordinates.X + 1,QuadCoordinates.Y + 1);
			}
			else
			{
				OutV0 = GetVertex(QuadCoordinates.X + 0,QuadCoordinates.Y + 0);
				OutV1 = GetVertex(QuadCoordinates.X + 1,QuadCoordinates.Y + 1);
				OutV2 = GetVertex(QuadCoordinates.X + 1,QuadCoordinates.Y + 0);
			}
		}
		else
		{
			if(TriangleIndex & 1)
			{
				OutV0 = GetVertex(QuadCoordinates.X + 0,QuadCoordinates.Y + 0);
				OutV1 = GetVertex(QuadCoordinates.X + 0,QuadCoordinates.Y + 1);
				OutV2 = GetVertex(QuadCoordinates.X + 1,QuadCoordinates.Y + 0);
			}
			else
			{
				OutV0 = GetVertex(QuadCoordinates.X + 1,QuadCoordinates.Y + 0);
				OutV1 = GetVertex(QuadCoordinates.X + 0,QuadCoordinates.Y + 1);
				OutV2 = GetVertex(QuadCoordinates.X + 1,QuadCoordinates.Y + 1);
			}
		}
		ElementIndex = 0;
	}

	void FTerrainComponentStaticLighting::GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
	{
		const INT QuadIndex = TriangleIndex / 2;
		const FIntPoint& QuadCoordinates = QuadIndexToCoordinatesMap(QuadIndex);

		if (Terrain->IsTerrainQuadFlipped(QuadCoordinates.X, QuadCoordinates.Y) == FALSE)
		{
			if(TriangleIndex & 1)
			{
				OutI0 = QuadIndex * 4 + 0;
				OutI1 = QuadIndex * 4 + 2;
				OutI2 = QuadIndex * 4 + 3;
			}
			else
			{
				OutI0 = QuadIndex * 4 + 0;
				OutI1 = QuadIndex * 4 + 3;
				OutI2 = QuadIndex * 4 + 1;
			}
		}
		else
		{
			if(TriangleIndex & 1)
			{
				OutI0 = QuadIndex * 4 + 0;
				OutI1 = QuadIndex * 4 + 2;
				OutI2 = QuadIndex * 4 + 1;
			}
			else
			{
				OutI0 = QuadIndex * 4 + 1;
				OutI1 = QuadIndex * 4 + 2;
				OutI2 = QuadIndex * 4 + 3;
			}
		}
	}

	void FTerrainComponentStaticLighting::Import( FLightmassImporter& Importer )
	{
		debugfSlow(TEXT("Importing terrain."));

		FStaticLightingMesh::Import(Importer);

 		Lightmass::FStaticLightingMappingData LightingMappingData;
		Importer.ImportData(&LightingMappingData);
 		Lightmass::FStaticLightingTextureMappingData TextureMappingData;
		Importer.ImportData(&TextureMappingData);
		TextureMapping.Guid = LightingMappingData.Guid;
		TextureMapping.bForceDirectLightMap = LightingMappingData.bForceDirectLightMap;
 		TextureMapping.SizeX = TextureMappingData.SizeX;
 		TextureMapping.SizeY = TextureMappingData.SizeY;
		TextureMapping.CachedSizeX = TextureMappingData.SizeX;
		TextureMapping.CachedSizeY = TextureMappingData.SizeY;
		TextureMapping.LightmapTextureCoordinateIndex = TextureMappingData.LightmapTextureCoordinateIndex;
		TextureMapping.bBilinearFilter = TextureMappingData.bBilinearFilter;
		TextureMapping.Mesh = this;

		Importer.ImportData((FStaticLightingTerrainMappingData*)this);

		// Look-up the terrain...
		Terrain = Importer.ConditionalImportObject<FTerrain>(TerrainGuid, LM_TERRAIN_VERSION, LM_TERRAIN_EXTENSION, LM_TERRAIN_CHANNEL_FLAGS, Importer.GetTerrains());
		checkf(Terrain, TEXT("Failed to import terrain with GUID %s"), *TerrainGuid.String());

		Importer.ImportData(&LocalToWorld);

		Importer.ImportData(&(Bounds.Origin));
		Importer.ImportData(&(Bounds.BoxExtent));
		Importer.ImportData(&(Bounds.SphereRadius));

		INT NumPatchBounds = SectionSizeX * SectionSizeY;
 		Importer.ImportArray(PatchBounds, NumPatchBounds);

		INT NumQuadIndices = 0;
		Importer.ImportData(&NumQuadIndices);
 		Importer.ImportArray(QuadIndexToCoordinatesMap, NumQuadIndices);
	}

	void FTerrainComponentStaticLighting::Dump()
	{
		debugf(TEXT("TERRAIN MAPPING DUMP:"));
		debugf(TEXT("\tTerrain     : %s"), Terrain ? *(Terrain->Guid.String()) : TEXT("???"));
		//FStaticLightingTextureMapping TextureMapping;
		//
		debugf(TEXT("\tLocalToWorld: "));
		debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), LocalToWorld.M[0][0], LocalToWorld.M[0][1], LocalToWorld.M[0][2], LocalToWorld.M[0][3]);
		debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), LocalToWorld.M[1][0], LocalToWorld.M[1][1], LocalToWorld.M[1][2], LocalToWorld.M[1][3]);
		debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), LocalToWorld.M[2][0], LocalToWorld.M[2][1], LocalToWorld.M[2][2], LocalToWorld.M[2][3]);
		debugf(TEXT("\t\t%-8.5f,%-8.5f,%-8.5f,%-8.5f"), LocalToWorld.M[3][0], LocalToWorld.M[3][1], LocalToWorld.M[3][2], LocalToWorld.M[3][3]);
		debugf(TEXT("\tBounds      : "));
		debugf(TEXT("\t\tOrigin      : %-8.5f,%-8.5f,%-8.5f"), Bounds.Origin.X, Bounds.Origin.Y, Bounds.Origin.Z);
		debugf(TEXT("\t\tBoxExtent   : %-8.5f,%-8.5f,%-8.5f"), Bounds.BoxExtent.X, Bounds.BoxExtent.Y, Bounds.BoxExtent.Z);
		debugf(TEXT("\t\tSphereRadius: %-8.5f"), Bounds.SphereRadius);
		debugf(TEXT("\tPatchBounds   : %d"), PatchBounds.Num());
		for (INT PatchIndex = 0; PatchIndex < PatchBounds.Num(); PatchIndex++)
		{
			debugf(TEXT("\t\t%5d - %-8.5f,%-8.5f,%-8.5f"), 
				PatchIndex,
				PatchBounds(PatchIndex).MinHeight,
 				PatchBounds(PatchIndex).MaxHeight,
 				PatchBounds(PatchIndex).MaxDisplacement
				);
		}
		//TArray<FIntPoint> QuadIndexToCoordinatesMap;
	}

} //namespace Lightmass
