/*=============================================================================
	Mesh.cpp: Mesh classes.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Mesh.h"
#include "Importer.h"
#include "Scene.h"

namespace Lightmass
{

//----------------------------------------------------------------------------
//	Mesh base class
//----------------------------------------------------------------------------
void FBaseMesh::Read( FILE* FileHandle, FScene& Scene)
{
	fread( (FBaseMeshData*)this, sizeof(FBaseMeshData), 1, FileHandle);
}

void FBaseMesh::Import( FLightmassImporter& Importer )
{
	Importer.ImportData( (FBaseMeshData*)this );
}

//----------------------------------------------------------------------------
//	Static mesh class
//----------------------------------------------------------------------------
void FStaticMesh::Read( FILE* FileHandle, FScene& Scene)
{
	// load super class
	FBaseMesh::Read( FileHandle, Scene);

	// import the shared data structure
	fread( (FStaticMeshData*)this, sizeof(FStaticMeshData), 1, FileHandle);

	debugfSlow(TEXT("Importing a static mesh with %d LODs [%s]"), NumLODs, *Guid.String());
	checkf(NumLODs > 0, TEXT("Imported a static mesh with 0 LODs. Uhoh"));

	// create the LODs
	LODs.Empty( NumLODs);

	// import each of the LODs
	for ( UINT LODIndex=0; LODIndex<NumLODs; LODIndex++)
	{
		FStaticMeshLOD* LOD = new(LODs) FStaticMeshLOD;

		// import each LOD separately
		LOD->Read( FileHandle);
	}
}

void FStaticMesh::Import( FLightmassImporter& Importer )
{
	// import super class
	FBaseMesh::Import(Importer);

	// import the shared data structure
	Importer.ImportData( (FStaticMeshData*)this );

	debugfSlow(TEXT("Importing a static mesh with %d LODs [%s]"), NumLODs, *Guid.String());
	checkf(NumLODs > 0, TEXT("Imported a static mesh with 0 LODs. Uhoh"));

	// create the LODs
	LODs.Empty(NumLODs);

	// import each of the LODs
	for (UINT LODIndex = 0; LODIndex < NumLODs; LODIndex++)
	{
		FStaticMeshLOD* LOD = new(LODs) FStaticMeshLOD;

		// import each LOD separately
		LOD->Import(Importer);
	}
}

//----------------------------------------------------------------------------
//	Static mesh LOD class
//----------------------------------------------------------------------------
void FStaticMeshLOD::Read( FILE* FileHandle)
{
	// import the shared data structure
	fread( (FStaticMeshLODData*)this, sizeof(FStaticMeshLODData), 1, FileHandle);

	FScene::ReadArray( Elements, NumElements, FileHandle);

	for (UINT MeshElementIndex = 0; MeshElementIndex < NumElements; MeshElementIndex++)
	{
		// Only triangle lists are supported for now
		check(Elements(MeshElementIndex).FirstIndex % 3 == 0);
	}

	FScene::ReadArray( Indices, NumIndices, FileHandle);
	FScene::ReadArray( Vertices, NumVertices, FileHandle);
}

void FStaticMeshLOD::Import( class FLightmassImporter& Importer )
{
	// import the shared data structure
	Importer.ImportData((FStaticMeshLODData*)this);

	Importer.ImportArray(Elements, NumElements);

	for (UINT MeshElementIndex = 0; MeshElementIndex < NumElements; MeshElementIndex++)
	{
		// Only triangle lists are supported for now
		check(Elements(MeshElementIndex).FirstIndex % 3 == 0);
	}
	Importer.ImportArray(Indices, NumIndices);
	Importer.ImportArray(Vertices, NumVertices);
}

}