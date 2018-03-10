/*=============================================================================
	Landscape.h: Static lighting Landscape mesh/mapping definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#define LANDSCAPE_ZSCALE	(1.0f/128.0f)

namespace Lightmass
{
	/** Represents the triangles of a Landscape primitive to the static lighting system. */
	class FLandscapeStaticLightingMesh : public FStaticLightingMesh, public FLandscapeStaticLightingMeshData
	{
	public:	
		FORCEINLINE void GetStaticLightingVertex(INT VertexIndex, FStaticLightingVertex& OutVertex) const;
		// FStaticLightingMesh interface.
		virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const;
		virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const;

		virtual void Import( class FLightmassImporter& Importer );

		// Accessors from FLandscapeDataInterface
		FORCEINLINE void VertexIndexToXY(INT VertexIndex, INT& OutX, INT& OutY) const;
		FORCEINLINE void QuadIndexToXY(INT QuadIndex, INT& OutX, INT& OutY) const;
		FORCEINLINE const FColor*	GetHeightData( INT LocalX, INT LocalY ) const;
		FORCEINLINE void GetTriangleIndices(INT QuadIndex,INT TriNum,INT& OutI0,INT& OutI1,INT& OutI2) const;

	private:
		TArray<FColor> HeightMap;
		// Cache
		INT NumVertices;
		INT NumQuads;
		FLOAT UVFactor;
	};

	/** Represents a landscape primitive with texture mapped static lighting. */
	class FLandscapeStaticLightingTextureMapping : public FStaticLightingTextureMapping
	{
	public:
		virtual void Import( class FLightmassImporter& Importer );
	};

} //namespace Lightmass
