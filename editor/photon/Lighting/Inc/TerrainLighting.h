/*=============================================================================
	TerrainLighting.h: Static lighting Terrain mesh/mapping definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#pragma once

namespace Lightmass
{

	/** Represents a Terrain component to the static lighting system. */
	class FTerrainComponentStaticLighting : public FStaticLightingMesh, public FStaticLightingTerrainMappingData
	{
	public:
	
		FTerrainComponentStaticLighting()
		{
			// Need to clear out everything...
			Terrain = NULL;
			TextureMapping.Guid = FGuid(0,0,0,0);
			TextureMapping.StaticLightingMeshInstance = FGuid(0,0,0,0);
			TextureMapping.bForceDirectLightMap = FALSE;
			TextureMapping.SizeX = 0;
			TextureMapping.SizeY = 0;
			TextureMapping.LightmapTextureCoordinateIndex = 0;
			TextureMapping.bBilinearFilter = FALSE;
			appMemzero(&LocalToWorld, sizeof(FMatrix));
			appMemzero(&Bounds, sizeof(FBoxSphereBounds));
			appMemzero(&PatchBounds, sizeof(TArray<FTerrainPatchBoundsData>));
			appMemzero(&QuadIndexToCoordinatesMap, sizeof(TArray<FIntPoint>));
		}
	
		/** Destructor. */
		~FTerrainComponentStaticLighting();

		/** The surface's vertices. */
		FStaticLightingVertex GetVertex(INT X, INT Y) const;

		// FStaticLightingMesh interface.
		virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const;
		virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const;

		virtual void Import( class FLightmassImporter& Importer );

		void Dump();

		/** The terrain this mesh is associated with. NOT SERIALIZED!!!! */
		class FTerrain* Terrain;

		/** The 'common' mapping data... */
		FStaticLightingTextureMapping TextureMapping;

		//
		FMatrix LocalToWorld;
		FBoxSphereBounds Bounds;
		TArray<FTerrainPatchBoundsData> PatchBounds;
		//@lmtodo. Material support for terrain....
		//TArray<FTerrainMaterialMask> BatchMaterials;
		//INT FullBatch;
		/** A map from quad index to the quad's coordinates. */
		TArray<FIntPoint> QuadIndexToCoordinatesMap;
	};


} //namespace Lightmass
