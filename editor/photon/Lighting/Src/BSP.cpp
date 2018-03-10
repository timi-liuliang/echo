/*=============================================================================
	BSP.cpp: Static lighting BSP mesh/mapping implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Importer.h"

namespace Lightmass
{
	FBSPSurfaceStaticLighting::~FBSPSurfaceStaticLighting()
	{
	}
	
	void FBSPSurfaceStaticLighting::GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const
	{
 		OutV0 = Vertices(TriangleVertexIndices(TriangleIndex * 3 + 0));
 		OutV1 = Vertices(TriangleVertexIndices(TriangleIndex * 3 + 1));
 		OutV2 = Vertices(TriangleVertexIndices(TriangleIndex * 3 + 2));
		ElementIndex = TriangleLightmassSettings(TriangleIndex);
	}

	void FBSPSurfaceStaticLighting::GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
	{
 		OutI0 = TriangleVertexIndices(TriangleIndex * 3 + 0);
 		OutI1 = TriangleVertexIndices(TriangleIndex * 3 + 1);
 		OutI2 = TriangleVertexIndices(TriangleIndex * 3 + 2);
	}

	void FBSPSurfaceStaticLighting::Import( FLightmassImporter& Importer )
	{
		FStaticLightingMesh::Import( Importer );
		Mapping.Import( Importer );		
		// bsp mapping/mesh are the same
		Mapping.Mesh = this;

		Importer.ImportData( (FBSPSurfaceStaticLightingData*) this );
		Importer.ImportArray( Vertices, NumVertices );
		Importer.ImportArray( TriangleVertexIndices, NumTriangles * 3 );
		Importer.ImportArray( TriangleLightmassSettings, NumTriangles );

		// Ignore invalid BSP lightmap UV's since they are generated and not something that artists have direct control over.
		//@todo - fix the root cause that results in some texels having their centers mapped to multiple texels.
		bColorInvalidTexels = FALSE;
	}

} //namespace Lightmass
