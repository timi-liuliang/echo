/*=============================================================================
	FluidSurface.cpp: Static lighting FluidSurface mesh/mapping implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "stdafx.h"
#include "Importer.h"
#include "FluidSurface.h"

namespace Lightmass
{

	/** Represents the fluid surface mesh to the static lighting system. */
	static void GetStaticLightingVertex(
		const FVector4* QuadCorners,
		const FVector4* QuadUVCorners,
		UINT VertexIndex,
		const FMatrix& LocalToWorld,
		const FMatrix& LocalToWorldInverseTranspose,
		FStaticLightingVertex& OutVertex
		)
	{
		OutVertex.WorldPosition = LocalToWorld.TransformFVector(QuadCorners[VertexIndex]);
		OutVertex.WorldTangentX = LocalToWorld.TransformNormal(FVector4(1, 0, 0, 1)).SafeNormal();
		OutVertex.WorldTangentY = LocalToWorld.TransformNormal(FVector4(0, 1, 0, 1)).SafeNormal();
		OutVertex.WorldTangentZ = LocalToWorldInverseTranspose.TransformNormal(FVector4(0, 0, 1, 1)).SafeNormal();

		for(UINT UVIndex = 0; UVIndex < 1; UVIndex++)
		{
			OutVertex.TextureCoordinates[UVIndex] = FVector2D(QuadUVCorners[VertexIndex].X, QuadUVCorners[VertexIndex].Y);
		}
	}

	// FStaticLightingMesh interface.
	void FFluidSurfaceStaticLightingMesh::GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const
	{
		GetStaticLightingVertex(QuadCorners,QuadUVCorners,QuadIndices[TriangleIndex * 3 + 0],LocalToWorld,LocalToWorldInverseTranspose,OutV0);
		GetStaticLightingVertex(QuadCorners,QuadUVCorners,QuadIndices[TriangleIndex * 3 + 1],LocalToWorld,LocalToWorldInverseTranspose,OutV1);
		GetStaticLightingVertex(QuadCorners,QuadUVCorners,QuadIndices[TriangleIndex * 3 + 2],LocalToWorld,LocalToWorldInverseTranspose,OutV2);
		ElementIndex = 0;
	}

	void FFluidSurfaceStaticLightingMesh::GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
	{
		OutI0 = QuadIndices[TriangleIndex * 3 + 0];
		OutI1 = QuadIndices[TriangleIndex * 3 + 1];
		OutI2 = QuadIndices[TriangleIndex * 3 + 2];
	}

	void FFluidSurfaceStaticLightingMesh::Import( class FLightmassImporter& Importer )
	{
		// import super class
		FStaticLightingMesh::Import( Importer );
		Importer.ImportData((FFluidSurfaceStaticLightingMeshData*) this);
		check(MaterialElements.Num() > 0);
	}

	void FFluidSurfaceStaticLightingTextureMapping::Import( class FLightmassImporter& Importer )
	{
		FStaticLightingTextureMapping::Import(Importer);

		// Can't use the FStaticLightingMapping Import functionality for this
		// as it only looks in the StaticMeshInstances map...
		Mesh = Importer.GetFluidMeshInstances().FindRef(Guid);
		check(Mesh);
	}

} //namespace Lightmass
