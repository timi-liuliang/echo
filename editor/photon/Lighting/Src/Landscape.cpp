/*=============================================================================
	Landscape.cpp: Static lighting Landscape mesh/mapping implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "stdafx.h"
#include "Importer.h"
#include "Landscape.h"

namespace Lightmass
{
	// Accessors from FLandscapeDataInterface
	void FLandscapeStaticLightingMesh::VertexIndexToXY(INT VertexIndex, INT& OutX, INT& OutY) const
	{
		OutX = VertexIndex % NumVertices;
		OutY = VertexIndex / NumVertices;
	}

	void FLandscapeStaticLightingMesh::QuadIndexToXY(INT QuadIndex, INT& OutX, INT& OutY) const
	{
		OutX = QuadIndex % NumQuads;
		OutY = QuadIndex / NumQuads;
	}

	const FColor* FLandscapeStaticLightingMesh::GetHeightData( INT LocalX, INT LocalY ) const
	{
		return &HeightMap(LocalX + LocalY * NumVertices );
	}

	void FLandscapeStaticLightingMesh::GetTriangleIndices(INT QuadIndex,INT TriNum,INT& OutI0,INT& OutI1,INT& OutI2) const
	{
		//OutI0 = 0; OutI1 = 1; OutI2 = 2;
		INT QuadX, QuadY;
		QuadIndexToXY( QuadIndex, QuadX, QuadY );
		switch(TriNum)
		{
		case 0:
			OutI0 = (QuadX + 0) + (QuadY + 0) * NumVertices;
			OutI1 = (QuadX + 1) + (QuadY + 1) * NumVertices;
			OutI2 = (QuadX + 1) + (QuadY + 0) * NumVertices;
			break;
		case 1:
			OutI0 = (QuadX + 0) + (QuadY + 0) * NumVertices;
			OutI1 = (QuadX + 0) + (QuadY + 1) * NumVertices;
			OutI2 = (QuadX + 1) + (QuadY + 1) * NumVertices;
			break;
		}
	}

	// from FStaticLightMesh....
	void FLandscapeStaticLightingMesh::GetStaticLightingVertex(INT VertexIndex, FStaticLightingVertex& OutVertex) const
	{
		INT X, Y;
		VertexIndexToXY(VertexIndex, X, Y);

		//GetWorldPositionTangents(X, Y, OutVertex.WorldPosition, OutVertex.WorldTangentX, OutVertex.WorldTangentY, OutVertex.WorldTangentZ);
		INT LocalX = X-ExpandQuadsX;
		INT LocalY = Y-ExpandQuadsY;

		const FColor* Data = GetHeightData( X, Y );

		OutVertex.WorldTangentZ.X = 2.f / 255.f * (FLOAT)Data->B - 1.f;
		OutVertex.WorldTangentZ.Y = 2.f / 255.f * (FLOAT)Data->A - 1.f;
		OutVertex.WorldTangentZ.Z = appSqrt(Max(1.f - (Square(OutVertex.WorldTangentZ.X)+Square(OutVertex.WorldTangentZ.Y)), 0.f));
		OutVertex.WorldTangentX = FVector4(OutVertex.WorldTangentZ.Z, 0.f, -OutVertex.WorldTangentZ.X);
		OutVertex.WorldTangentY = OutVertex.WorldTangentZ ^ OutVertex.WorldTangentX;

		// Assume there is no rotation, so we don't need to do any LocalToWorld.
		WORD Height = (Data->R << 8) + Data->G;

		OutVertex.WorldPosition = LocalToWorld.TransformFVector( FVector4( LocalX, LocalY, ((FLOAT)Height - 32768.f) * LANDSCAPE_ZSCALE ) );
		//debugf(TEXT("%d, %d, %d, %d, %d, %d, X:%f, Y:%f, Z:%f "), SectionBaseX+LocalX-ExpandQuadsX, SectionBaseY+LocalY-ExpandQuadsY, ClampedLocalX, ClampedLocalY, SectionBaseX, SectionBaseY, WorldPos.X, WorldPos.Y, WorldPos.Z);

		INT LightmapUVIndex = 1;

		OutVertex.TextureCoordinates[0] = FVector2D((FLOAT)X / NumVertices, (FLOAT)Y / NumVertices); 
		OutVertex.TextureCoordinates[LightmapUVIndex].X = (X * StaticLightingResolution) * UVFactor;
		OutVertex.TextureCoordinates[LightmapUVIndex].Y = (Y * StaticLightingResolution) * UVFactor;
	}

	// FStaticLightingMesh interface.
	void FLandscapeStaticLightingMesh::GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const
	{
		INT I0, I1, I2;
		//GetTriangleIndices(TriangleIndex,I0, I1, I2);
		I0 = I1 = I2 = 0;
		INT QuadIndex = TriangleIndex >> 1;
		INT QuadTriIndex = TriangleIndex & 1;

		GetTriangleIndices(QuadIndex, QuadTriIndex, I0, I1, I2);
		GetStaticLightingVertex(I0,OutV0);
		GetStaticLightingVertex(I1,OutV1);
		GetStaticLightingVertex(I2,OutV2);

		ElementIndex = 0;
	}

	void FLandscapeStaticLightingMesh::GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
	{
		//OutI0 = OutI1 = OutI2 = 0;
		INT QuadIndex = TriangleIndex >> 1;
		INT QuadTriIndex = TriangleIndex & 1;

		GetTriangleIndices(QuadIndex, QuadTriIndex, OutI0, OutI1, OutI2);
	}

	void FLandscapeStaticLightingMesh::Import( class FLightmassImporter& Importer )
	{
		// import super class
		FStaticLightingMesh::Import( Importer );
		Importer.ImportData((FLandscapeStaticLightingMeshData*) this);

		// we have the guid for the mesh, now hook it up to the actual static mesh
		INT ReadSize = Square(ComponentSizeQuads + 2*ExpandQuadsX + 1);
		checkf(ReadSize > 0, TEXT("Failed to import Landscape Heightmap data!"));
		Importer.ImportArray(HeightMap, ReadSize);
		check(HeightMap.Num() == ReadSize);

		NumVertices = ComponentSizeQuads + 2*ExpandQuadsX + 1;
		NumQuads = NumVertices - 1;
		UVFactor = LightMapRatio / (FLOAT)(NumVertices * StaticLightingResolution);
	}

	void FLandscapeStaticLightingTextureMapping::Import( class FLightmassImporter& Importer )
	{
		FStaticLightingTextureMapping::Import(Importer);

		// Can't use the FStaticLightingMapping Import functionality for this
		// as it only looks in the StaticMeshInstances map...
		Mesh = Importer.GetLandscapeMeshInstances().FindRef(Guid);
		check(Mesh);
	}

} //namespace Lightmass
