/*=============================================================================
	Mesh.h: Mesh classes.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

// @lmtodo: This seems wrong to me to be needed (scene uses mesh, not the other way around), and is only needed for the MAX_TEXCOORDS
#include "..\Public\MeshExport.h"
#include "..\Public\SceneExport.h"

namespace Lightmass
{

class FStaticMeshLOD;
class FStaticMeshElement;


//----------------------------------------------------------------------------
//	Mesh base class
//----------------------------------------------------------------------------

class FBaseMesh : public FBaseMeshData
{
public:
	virtual void			Read( FILE* FileHandle, FScene& Scene);
	virtual void			Import( class FLightmassImporter& Importer );
};


//----------------------------------------------------------------------------
//	Static mesh class
//----------------------------------------------------------------------------

class FStaticMesh : public FBaseMesh, public FStaticMeshData
{
public:
	virtual void			Read( FILE* FileHandle, FScene& Scene);
	virtual void			Import( class FLightmassImporter& Importer );

	/**
	 * @return the given LOD by index
	 */
	inline const FStaticMeshLOD&	GetLOD(INT Index) const
	{
		return LODs(Index);
	}

protected:
	/** array of LODs (same number as FStaticMeshData.NumLODs) */
	TArray<FStaticMeshLOD>		LODs;
};


//----------------------------------------------------------------------------
//	Static mesh LOD class
//----------------------------------------------------------------------------

class FStaticMeshLOD : public FStaticMeshLODData
{
public:
	virtual void			Read( FILE* FileHandle);
	virtual void			Import( class FLightmassImporter& Importer );

	/**
	 * @return the given element by index
	 */
	inline const FStaticMeshElement&	GetElement(INT Index) const
	{
		return Elements(Index);
	}

	/**
	 * @return the given vertex index by index
	 */
	inline const WORD					GetIndex(INT Index) const
	{
		return Indices(Index);
	}

	/**
	 * @return the given vertex by index
	 */
	inline const FStaticMeshVertex&		GetVertex(INT Index) const
	{
		return Vertices(Index);
	}
protected:
	/** array of Elements for this LOD (same number as FStaticMeshLODData.NumElements) */
	TArray<FStaticMeshElement>	Elements;

	/** array of Indices for this LOD (same number as FStaticMeshLODData.NumIndices) */
	TArray<WORD>				Indices;

	/** array of vertices for this LOD (same number as FStaticMeshLODData.NumVertices) */
	TArray<FStaticMeshVertex>	Vertices;
};


//----------------------------------------------------------------------------
//	Static mesh element class
//----------------------------------------------------------------------------

class FStaticMeshElement : public FStaticMeshElementData
{
public:
protected:
};

}	// namespace Lightmass


