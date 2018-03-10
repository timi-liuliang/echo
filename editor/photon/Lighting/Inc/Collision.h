/*=============================================================================
	Collision.h: Static lighting collision system definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "LMOctree.h"

namespace Lightmass
{	

/** Flags set on a FLightRay that control how the ray is intersected with the scene. */
enum ELightRayIntersectionFlags
{
	LIGHTRAY_NONE				= 1<<0,
	/** Whether the ray should intersect with triangles from the ray's mesh. */
	LIGHTRAY_SELFSHADOWDISABLE	= 1<<1,
	/** Whether the ray should only intersect with static, opaque materials, and exlude masked and translucent materials. */
	LIGHTRAY_STATIC_AND_OPAQUEONLY	= 1<<2,
	/** Whether to flip what is considered a backface.  This is useful for getting consistent backface culling regardless of which side of a ray the trace starts at. */
	LIGHTRAY_FLIP_SIDEDNESS		= 1<<3
};

/** A line segment representing a direct light path through the scene. */
class FLightRay
{
public: 

	FVector4 Start;
	FVector4 End;
	FVector4 Direction;
	FLOAT Length;

	/** The mapping that the ray originated from, used for conditional intersections. */
	const FStaticLightingMapping* Mapping;
	const FStaticLightingMesh* Mesh;
	const FLight* Light;

	DWORD TraceFlags;

	FLightRay() {}

	/** Initialization constructor. */
	FLightRay(const FVector4& InStart, const FVector4& InEnd, const FStaticLightingMapping* InMapping, const FLight* InLight, DWORD InTraceFlags = LIGHTRAY_NONE)
	:	Start(InStart)
	,	End(InEnd)
	,	Direction(InEnd - InStart)
	,	Mapping(InMapping)
	,	Mesh(InMapping ? InMapping->Mesh : NULL)
	,	Light(InLight)
	,	TraceFlags(InTraceFlags)
	{
		Length = 1.0f;
	}

	/** Clips the light ray from the original start to an intersection point. */
	void ClipAgainstIntersectionFromStart(const FVector4& IntersectionPoint)
	{
		End = IntersectionPoint;
		Direction = End - Start;
	}

	/** Clips the light ray from the original end to an intersection point. */
	void ClipAgainstIntersectionFromEnd(const FVector4& IntersectionPoint)
	{
		Start = IntersectionPoint;
		Direction = End - Start;
	}
};

class FStaticLightingAggregateMeshDataProvider;  /* needs predeclaration... */

/** Information about a single mesh that got aggregated. */
struct FStaticLightingMeshInfo
{
	/** First index of the mesh into FStaticLightingAggregateMesh::Vertices, UVs and LightmapUVs */
	const INT BaseIndex;
	const FStaticLightingMesh* Mesh;

	FStaticLightingMeshInfo(INT InBaseIndex, const FStaticLightingMesh* InMesh) :
		BaseIndex(InBaseIndex),
		Mesh(InMesh)
	{
		checkSlow(InMesh);
	}
};

/** Each FTriangleSOA in the kDOP references 4 of these, one for each triangle it represents. */
struct FTriangleSOAPayload
{
	/** Constructor. */
	FTriangleSOAPayload(const FStaticLightingMeshInfo* InMeshInfo, const FStaticLightingMapping* InMapping, INT InElementIndex, INT VertexIndex1, INT VertexIndex2, INT VertexIndex3 )
	:	MeshInfo(InMeshInfo)
	,	Mapping(InMapping)
	,	ElementIndex(InElementIndex)
	{
		VertexIndex[0] = VertexIndex1;
		VertexIndex[1] = VertexIndex2;
		VertexIndex[2] = VertexIndex3;
		checkSlow(MeshInfo->Mesh != NULL);
		checkSlow(ElementIndex >= 0 && ElementIndex < MeshInfo->Mesh->GetNumElements());
	}

	/** Information about the Mesh that uses the triangle. */
	const FStaticLightingMeshInfo* MeshInfo;
	/** The Mapping that uses the triangle. */
	const FStaticLightingMapping* Mapping;
	/** Mesh element index */
	INT ElementIndex;
	/** Index into FStaticLightingAggregateMesh::Vertices, UVs and LightmapUVs, for each of the vertices of the triangle. */
	INT VertexIndex[3];
};

/** The static lighting mesh. */
class FStaticLightingAggregateMesh
{
public:

	/** Initialization constructor. */
	FStaticLightingAggregateMesh(const FScene& InScene);

	virtual ~FStaticLightingAggregateMesh();

	/**
	 * Merges a mesh into the shadow mesh.
	 * @param Mesh - The mesh the triangle comes from.
	 */
	void AddMesh(const FStaticLightingMesh* Mesh, const FStaticLightingMapping* Mapping);

	/**
	 * Pre-allocates memory ahead of time, before calling AddMesh() a bunch of times.
	 *
	 * @param NumMeshes		- Expected number of meshes which will be added
	 * @param NumVertices	- Expected number of vertices which will be added
	 * @param NumTriangles	- Expected number of triangles which will be added
	 */
	void ReserveMemory( INT NumMeshes, INT NumVertices, INT NumTriangles );

	/** Prepares the mesh for raytracing. */
	void PrepareForRaytracing();

	void DumpStats() const;

	FBox GetBounds() const;

	/** The total surface area of everything in the aggregate mesh */
	FLOAT GetSurfaceArea() const;
	/** The total surface area of everything in the aggregate mesh within the importance volume, if there is one */
	FLOAT GetSurfaceAreaWithinImportanceVolume() const;

	/**
	 * Checks a light ray for intersection with the shadow mesh.
	 * @param LightRay - The line segment to check for intersection.
	 * @param bFindClosestIntersection - TRUE if the intersection must return the closest intersection.  FALSE if it may return any intersection.
	 *		This can be used as an optimization for rays which only need to know if there was an intersection or not, but not any other information about the intersection.
	 *		Note: bFindClosestIntersection == FALSE currently does not handle masked materials correctly, it treats them as if they were opaque.
	 *		However, bFindClosestIntersection == FALSE does work correctly in conjunction with LIGHTRAY_STATIC_AND_OPAQUEONLY.
	 * @param bCalculateTransmission - Whether to keep track of transmission or not.  If this is TRUE, bFindClosestIntersection must also be TRUE.
	 * @param bDirectShadowingRay - Whether this ray is being used to calculate direct shadowing.
	 * @param CoherentRayCache - The calling thread's collision cache.
	 * @param [out] Intersection - The intersection of between the light ray and the mesh.
	 * @return TRUE if there is an intersection, FALSE otherwise
	 */
	UBOOL IntersectLightRay(
		const FLightRay& LightRay,
		UBOOL bFindClosestIntersection,
		UBOOL bCalculateTransmission,
		UBOOL bDirectShadowingRay,
		class FCoherentRayCache& CoherentRayCache,
		FLightRayIntersection& Intersection) const;

private:

	const FScene& Scene;

	friend class FStaticLightingAggregateMeshDataProvider;

	/** The world-space kDOP which is used by the simple meshes in the world. */
	TkDOPTree<const FStaticLightingAggregateMeshDataProvider,DWORD> kDopTree;

	/** The triangles used to build the kDOP, valid until PrepareForRaytracing is called. */
	TArray<FkDOPBuildCollisionTriangle<DWORD> > kDOPTriangles;
 
	/** TriangleSOA payload. Each TriangleSOA in the kDOP references 4 of these (one for each of the 4 triangles in a TriangleSOA). */
	TArray<FTriangleSOAPayload> TrianglePayloads;

	/** Information about the meshes used in the kDOP tree. */
	TArray<const FStaticLightingMeshInfo*> MeshInfos;

	/** 
	 * The vertices used by the kDOP. 
	 * @todo - should all of these vertex attributes be stored in the same array? (ArrayOfStructures instead of SoA)
	 */
	TArray<FVector4> Vertices;

	/** The texture coordinates used by the kDOP. */
	TArray<FVector2D> UVs;

	/** The lightmap coordinates used by the kDOP. */
	TArray<FVector2D> LightmapUVs;

	/** The bounding box of everything in the aggregate mesh. */
	FBox SceneBounds;

	/** The total surface area of everything in the aggregate mesh */
	FLOAT SceneSurfaceArea;
	/** The total surface area of everything in the aggregate mesh within the importance volume, if there is one */
	FLOAT SceneSurfaceAreaWithinImportanceVolume;
};

/** Information which is cached while processing a group of coherent rays. */
class FCoherentRayCache
{
public:
	QWORD NumFirstHitRaysTraced;
	QWORD NumBooleanRaysTraced;
	FLOAT FirstHitRayTraceTime;
	FLOAT BooleanRayTraceTime;

	/** 
	 * Stores the index of the last hit kDOP node when doing a boolean visibility check. 
	 * Used to optimize coherent boolean visibliity traces.
	 */
	DWORD kDOPNodeIndex;

	/** Initialization constructor. */
	FCoherentRayCache() :
		NumFirstHitRaysTraced(0),
		NumBooleanRaysTraced(0),
		FirstHitRayTraceTime(0),
		BooleanRayTraceTime(0),
		kDOPNodeIndex(0xFFFFFFFF)
	{}

	void Clear()
	{
		kDOPNodeIndex = 0xFFFFFFFF;
	}
};

} //namespace Lightmass
