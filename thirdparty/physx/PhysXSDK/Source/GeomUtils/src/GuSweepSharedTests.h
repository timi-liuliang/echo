/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef GU_SWEEP_SHARED_TESTS_H
#define GU_SWEEP_SHARED_TESTS_H

#include "CmPhysXCommon.h"
#include "PxQueryReport.h"
#include "GuBoxConversion.h"
#include "GuGeomUtilsInternal.h"
#include "GuTriangleMesh.h"
#include "PxTriangleMeshGeometry.h"

namespace physx
{
#ifdef __SPU__
	#define GU_SWEEP_SHARED inline
#else
	#define GU_SWEEP_SHARED PX_FORCE_INLINE
#endif

#ifdef __SPU__
	#define SPU_INLINE
#else
	#define SPU_INLINE	PX_FORCE_INLINE
#endif

#define LOCAL_EPSILON 0.00001f	// PT: this value makes the 'basicAngleTest' pass. Fails because of a ray almost parallel to a triangle

#define HF_SWEEP_REPORT_BUFFER_SIZE 64

#ifdef __SPU__
#define FETCH_CONVEX_HULL_DATA(convexGeom)																							\
	PX_COMPILE_TIME_ASSERT(&((ConvexMesh*)NULL)->getHull()==NULL);																	\
																																	\
	PX_ALIGN_PREFIX(16)  PxU8 convexMeshBuffer[sizeof(ConvexMesh)+32] PX_ALIGN_SUFFIX(16);											\
	ConvexMesh* convexMesh = memFetchAsync<ConvexMesh>(convexMeshBuffer, MemFetchPtr(convexGeom.convexMesh), sizeof(ConvexMesh),1);	\
	memFetchWait(1); /* convexMesh*/																								\
																																	\
	const PxU32 nbPolys = convexMesh->getNbPolygonsFast();																			\
	const HullPolygonData* PX_RESTRICT polysEA = convexMesh->getPolygons();															\
	const PxU32 polysSize = sizeof(HullPolygonData)*nbPolys + sizeof(PxVec3)*convexMesh->getNbVerts();								\
																																	\
 	/*TODO: Need optimization with dma cache --jiayang*/																			\
	void* hullBuffer = PxAlloca(CELL_ALIGN_SIZE_16(polysSize+32));																	\
	HullPolygonData* polys = memFetchAsync<HullPolygonData>(hullBuffer, (uintptr_t)(polysEA), polysSize, 1);						\
																																	\
	ConvexHullData* hullData = &convexMesh->getHull();																				\
	hullData->mPolygons = polys;																									\
																																	\
	memFetchWait(1); // convex mesh polygons
#else
#define FETCH_CONVEX_HULL_DATA(convexGeom)																							\
	ConvexMesh* convexMesh = static_cast<ConvexMesh*>(convexGeom.convexMesh);														\
	ConvexHullData* hullData = &convexMesh->getHull();																				\
	const PxU32 nbPolys = hullData->mNbPolygons;	PX_UNUSED(nbPolys);
#endif

GU_SWEEP_SHARED void computeWorldToBoxMatrix(physx::Cm::Matrix34& worldToBox, const physx::Gu::Box& box)
{
	physx::Cm::Matrix34 boxToWorld;
	physx::buildMatrixFromBox(boxToWorld, box);
	worldToBox = boxToWorld.getInverseRT();
}

#ifdef __SPU__ // AP: this is to reduce code size on SPU
inline void buildFrom1(Gu::Box& dst, const PxVec3& center, const PxVec3& extents, const PxQuat& q)
{
	dst.center	= center;
	dst.extents	= extents;
	dst.rot		= PxMat33(q);
}
#else
#define buildFrom1 buildFrom
#endif

// AP: uninlining increases SPU size
PX_FORCE_INLINE PxU32 getTriangleIndex(PxU32 i, PxU32 cachedIndex)
{
	PxU32 triangleIndex;
	if(i==0)				triangleIndex = cachedIndex;
	else if(i==cachedIndex)	triangleIndex = 0;
	else					triangleIndex = i;
	return triangleIndex;
}

PX_FORCE_INLINE	void getScaledTriangle(const PxTriangleMeshGeometry& triGeom, const Cm::Matrix34& vertex2worldSkew, PxTriangle& triangle, PxTriangleID triangleIndex)
{
	Gu::TriangleMesh* tm = static_cast<Gu::TriangleMesh*>(triGeom.triangleMesh);
	tm->computeWorldTriangle(triangle, triangleIndex, vertex2worldSkew);
}
}


#endif
