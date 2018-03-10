/*=============================================================================
	MeshExport.h: Mesh export data definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once
namespace Lightmass
{

enum { MAX_TEXCOORDS=4 };

#pragma pack(push, 1)

//----------------------------------------------------------------------------
//	Helper definitions
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//	Mesh export file header
//----------------------------------------------------------------------------
struct FMeshFileHeader
{
	/** FourCC cookie: 'MESH' */
	UINT			Cookie;
	FGuid		FormatVersion;

	// These structs follow immediately after this struct.
	//
	//	FBaseMeshData			BaseMeshData;
	//	FStaticMeshData			MeshData;
	//	StaticMeshLODAggregate	MeshLODs[ MeshData.NumLODs ];
	//
	//	Where:
	//
	// 	struct StaticMeshLODAggregate
	// 	{
	//		FStaticMeshLODData		LOD;
	// 		FStaticMeshElementData	MeshElements[ LOD.NumElements ];
	//		UINT16					Indices[ LOD.NumIndices ];
	//		FStaticMeshVertex		Vertices[ LOD.NumVertices ];
	// 	};
};
 

//----------------------------------------------------------------------------
//	Base mesh
//----------------------------------------------------------------------------
struct FBaseMeshData
{
	FGuid		Guid;
};

//----------------------------------------------------------------------------
//	Static mesh, builds upon FBaseMeshData
//----------------------------------------------------------------------------
struct FStaticMeshData
{
	UINT			LightmapCoordinateIndex;
	UINT			NumLODs;
};

//----------------------------------------------------------------------------
//	Static mesh LOD
//----------------------------------------------------------------------------
struct FStaticMeshLODData
{
	UINT			NumElements;
	/** Total number of triangles for all elements in the LOD. */
	UINT			NumTriangles;
	/** Total number of indices in the LOD. */
	UINT			NumIndices;
	/** Total number of vertices in the LOD. */
	UINT			NumVertices;
};

//----------------------------------------------------------------------------
//	Static mesh element
//----------------------------------------------------------------------------
struct FStaticMeshElementData
{
	UINT		FirstIndex;
	UINT		NumTriangles;
	BITFIELD	bEnableShadowCasting : 1;
};

//----------------------------------------------------------------------------
//	Static mesh vertex
//----------------------------------------------------------------------------
struct FStaticMeshVertex
{
	FVector4		Position;
	FVector4		TangentX;
	FVector4		TangentY;
	FVector4		TangentZ;
	FVector2D		UVs[MAX_TEXCOORDS];
};

//----------------------------------------------------------------------------
//	SpeedTree
//----------------------------------------------------------------------------
struct FSpeedTreeVertex
{
	FVector4		Position;
	FVector4		TangentX;
	FVector4		TangentY;
	FVector4		TangentZ;
	FVector2D		TexCoord;
	
	// sub mesh type specific extra data storage
	// @lmtodo: Make a subclass of this and put this only in the ones that need it
	FVector4 CornerOffset;
};

/** 
 *	Enumerates the types of materials in a SpeedTreeComponent.
 *	Needs to match the 'enum ESpeedTreeMeshType' in SpeedTree.h in the engine.
 */
enum ESpeedTreeMaterial
{
	STMAT_Branches = 0,
	STMAT_Fronds,
	STMAT_LeafMeshes,
	STMAT_LeafCards,
	STMAT_Billboards,
	STMAT_MAX
};

struct FSpeedTreeData
{
	/** Size of index buffer */
	UINT NumIndices;

	UINT NumBranchVertices;
	UINT NumFrondVertices;
	UINT NumLeafMeshVertices;
	UINT NumLeafCardVertices;
	UINT NumBillboardVertices;
};

#pragma pack(pop)

}	// namespace Lightmass
