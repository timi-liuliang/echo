/*=============================================================================
	SpeedTree.cpp: Static lighting speedtree implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Importer.h"
#include "SpeedTree.h"

namespace Lightmass
{
/**
* Computes the local-space vertex position for a leaf card vertex.
* @param PivotPoint - The leaf card vertex's pivot point.
* @param VertexData - The leaf card vertex's data.
* @return the local-space vertex position.
*/
static FVector4 GetLeafCardVertexPosition(const FVector4& PivotPoint,const FSpeedTreeVertex& VertexData)
{
	return PivotPoint + VertexData.CornerOffset;
}
	
/**
 * Creates a static lighting vertex to represent the given static mesh vertex.
 * @param VertexBuffer - The static mesh's vertex buffer.
 * @param VertexIndex - The index of the static mesh vertex to access.
 * @param OutVertex - Upon return, contains a static lighting vertex representing the specified static mesh vertex.
 */
static void GetStaticLightingVertex(
	const FSpeedTreeVertex& InVertex,
	const FMatrix& LocalToWorld,
	const FMatrix& LocalToWorldInverseTranspose,
	FStaticLightingVertex& OutVertex,
	ESpeedTreeMeshType MeshType
	)
{
	FVector4 VertexPosition = InVertex.Position;
	if (MeshType == STMT_LeafCards)
	{
		VertexPosition = GetLeafCardVertexPosition(VertexPosition, InVertex);
	}

	OutVertex.WorldPosition = LocalToWorld.TransformFVector(VertexPosition);
	OutVertex.WorldTangentX = LocalToWorld.TransformNormal(InVertex.TangentX).SafeNormal();
	OutVertex.WorldTangentY = LocalToWorld.TransformNormal(InVertex.TangentY).SafeNormal();
	OutVertex.WorldTangentZ = LocalToWorldInverseTranspose.TransformNormal(InVertex.TangentZ).SafeNormal();
	OutVertex.TextureCoordinates[0] = InVertex.TexCoord;
}

void FSpeedTree::Import(FLightmassImporter& Importer)
{
	// base class import
	FBaseMesh::Import(Importer);

	// import the shared speed tree data
	Importer.ImportData((FSpeedTreeData*)this);

	// now we have the counts of all the indices and vertex types, so import them all
	Importer.ImportArray(Indices, NumIndices);
	Importer.ImportArray(BranchVertices, NumBranchVertices);
	Importer.ImportArray(FrondVertices, NumFrondVertices);
	Importer.ImportArray(LeafMeshVertices, NumLeafMeshVertices);
	Importer.ImportArray(LeafCardVertices, NumLeafCardVertices);
	Importer.ImportArray(BillboardVertices, NumBillboardVertices);
}

void FSpeedTreeStaticLightingMesh::GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const
{
	// Read the triangle's vertex indices.
	const INT I0 = SpeedTree->GetIndices()(ElementFirstIndex + TriangleIndex * 3 + 0);
	const INT I1 = SpeedTree->GetIndices()(ElementFirstIndex + TriangleIndex * 3 + 1);
	const INT I2 = SpeedTree->GetIndices()(ElementFirstIndex + TriangleIndex * 3 + 2);

	// Read the triangle's vertices.
	OutV0 = CachedStaticLightingVertices(I0);
	OutV1 = CachedStaticLightingVertices(I1);
	OutV2 = CachedStaticLightingVertices(I2);
	ElementIndex = MeshType - 1;
}

void FSpeedTreeStaticLightingMesh::GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
{
	// Read the triangle's vertex indices.
	OutI0 = SpeedTree->GetIndices()(ElementFirstIndex + TriangleIndex * 3 + 0) - ElementMinVertexIndex;
	OutI1 = SpeedTree->GetIndices()(ElementFirstIndex + TriangleIndex * 3 + 1) - ElementMinVertexIndex;
	OutI2 = SpeedTree->GetIndices()(ElementFirstIndex + TriangleIndex * 3 + 2) - ElementMinVertexIndex;
}

void FSpeedTreeStaticLightingMesh::Import( class FLightmassImporter& Importer )
{
	// import super class
	FStaticLightingMesh::Import(Importer);

	// import the shared data
	Importer.ImportData((FSpeedTreeStaticLightingMeshData*) this);

	// calculate the inverse transpose
	LocalToWorldInverseTranspose = LocalToWorld.Inverse().Transpose();

	// we have the guid for the mesh, now hook it up to the actual static mesh
	SpeedTree = Importer.ConditionalImportObject<FSpeedTree>(InstanceSpeedTreeGuid, LM_SPEEDTREE_VERSION, LM_SPEEDTREE_EXTENSION, LM_SPEEDTREE_CHANNEL_FLAGS, Importer.GetSpeedTrees());
	checkf(SpeedTree, TEXT("Failed to import speed tree with GUID %s"), *InstanceSpeedTreeGuid.String());

	/** Cache the static lighting vertices for this mesh type. */
	const TArray<FSpeedTreeVertex>& STVertices = SpeedTree->GetVertices(MeshType);
	CachedStaticLightingVertices.Empty(STVertices.Num());
	CachedStaticLightingVertices.AddZeroed(STVertices.Num());
	for (INT VertexIndex = 0; VertexIndex < STVertices.Num(); VertexIndex++)
	{
		GetStaticLightingVertex(STVertices(VertexIndex), LocalToWorld, LocalToWorldInverseTranspose, CachedStaticLightingVertices(VertexIndex), MeshType);
	}

	check(SpeedTree->NumIndices % 3 == 0 && (INT)SpeedTree->NumIndices / 3 >= NumTriangles);
	check(STVertices.Num() >= (INT)NumVertices);
	for (INT TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		INT TriangleVertexIndices[3];
		GetTriangleIndices(
			TriangleIndex,
			TriangleVertexIndices[0],
			TriangleVertexIndices[1],
			TriangleVertexIndices[2]
		);

		for (INT i = 0; i < 3; i++)
		{
			checkf(TriangleVertexIndices[i] >= 0 && TriangleVertexIndices[i] < NumVertices, 
				TEXT("Speedtree index out of vertex array bounds: [%u/%u], TriangleIndex %u.%u"), 
				TriangleVertexIndices[i], NumVertices, TriangleIndex, i);
		}
	}
}


/** Initialization constructor. */
FSpeedTreeStaticLightingMapping::FSpeedTreeStaticLightingMapping(FStaticLightingMesh* InMesh)
{
}

void FSpeedTreeStaticLightingMapping::Import( class FLightmassImporter& Importer )
{
	Importer.ImportData( (FStaticLightingMappingData*) this );
	Importer.ImportData( (FStaticLightingVertexMappingData*) this );
	Mesh = Importer.GetSpeedTreeMeshInstances().FindRef(Guid);
	check(Mesh);
}

} //namespace Lightmass
