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

#ifndef PX_PHYSICS_COMMON_MPHTRACE_H
#define PX_PHYSICS_COMMON_MPHTRACE_H

#define MIDPHASE_TRACE 0

#if MIDPHASE_TRACE

#include "PxTransform.h"
#include "PxMeshScale.h"
#include "PsThread.h"

using namespace physx;

struct MPTContext
{
	const char*			tag;
	const PxTransform	transform;
	const PxMeshScale	scale;
	PX_FORCE_INLINE PxMeshScale getScale() const // don't use in perf critical code
	{
		PxQuat scRot;
		PxVec3 sc = PxDiagonalize(vertex2ShapeSkew, scRot);
		return PxMeshScale(sc, scRot);
	}

	MPTContext(const char* tg, const PxTransform& xf, const PxMeshScale& sc) : tag(tg), transform(xf), scale(sc) {}
	MPTContext(const char* tg, const PxTransform& xf, const FastVertex2ShapeScaling& sc) : tag(tg), transform(xf), scale(sc.getScale()) {}
	void operator=(const MPTContext& rhs) { *this = rhs; }
};

extern PxU32 mptTlsSlot;

#define MPT_INIT \
	mptTlsSlot = shdfnd::TlsAlloc();

#define MPT_SET_CONTEXT(a,b,c) \
	MPTContext mptCtx((a), (b), (c)); shdfnd::TlsSet(mptTlsSlot, &mptCtx);

#define MPT_GET_CONTEXT	((MPTContext*)shdfnd::TlsGet(mptTlsSlot))

#define MESH_WRITE_TRACE \
		PxU32& meshId = model.mRTree->mUnused; \
		bool newMesh = (meshId == 0); \
		if (newMesh) \
			meshId = getMeshId()+1; \
		MPTContext* ctx = MPT_GET_CONTEXT; \
		writeTrace( \
			ctx->tag, ctx->transform, ctx->scale, rTreeCallback.touchedLeaves, \
			newMesh, meshId-1, mi->GetVerts(), mi->GetNbVertices(), mi->GetTris(), mi->GetNbTriangles()*3, *has16BitIndices, \
			&orig, &dir, &maxT, PxU32(bothSides), inflate, NULL, NULL);

#define MESH_WRITE_TRACE_OBB \
	PxU32& meshId = (PxU32&)model.mRTree->mUnused; \
	bool newMesh = (meshId == 0); \
	if (newMesh) \
		meshId = getMeshId()+1; \
	MPTContext* ctx = MPT_GET_CONTEXT; \
	writeTrace( \
		ctx->tag, ctx->transform, ctx->scale, rTreeCallback.touchedLeaves, \
		newMesh, meshId-1, mi->GetVerts(), mi->GetNbVertices(), mi->GetTris(), mi->GetNbTriangles()*3, *has16BitIndices, \
		NULL, NULL, NULL, PxU32(bothTriangleSidesCollide), NULL, &obb, NULL);

// export HF as indexed triangles
#define HF_WRITE_TRACE \
				bool newHf = mHeightField->mSampleStride <= 4; \
				MPTContext* ctx = MPT_GET_CONTEXT; \
				if (newHf) \
				{ \
					PxU32 meshId = getMeshId(); \
					((Gu::HeightField*)mHeightField)->mSampleStride = (meshId+5); \
					Ps::Array<PxVec3> verts; \
					Ps::Array<PxU32> inds; \
					const PxI32 nbCols = (PxI32)mHeightField->getNbColumnsFast(), nbRows = (PxI32)mHeightField->getNbRowsFast(); \
					PxU32 numVerts, numInds; \
					generateMesh(PxU32(nbCols), PxU32(nbRows), verts, inds, numVerts, numInds); \
					PxReal maxT(1); \
					writeTrace(ctx->tag, ctx->transform, PxMeshScale(), faces, true, meshId, verts.begin(), numVerts, inds.begin(), numInds, \
						false, &aP0, &rayDir, &maxT, true, overlapObjectExtent, NULL, mHfGeom); \
				} else { \
					PxReal maxT(1); \
					PxU32 meshId = mHeightField->mSampleStride-5; \
					writeTrace(ctx->tag, ctx->transform, PxMeshScale(), faces, false, meshId, NULL, 0, NULL, 0, false, \
						&aP0, &rayDir, &maxT, true, overlapObjectExtent, NULL, mHfGeom); \
				}

#else

#define MPT_INIT
#define MPT_SET_CONTEXT(a, b, c)
#define MESH_WRITE_TRACE
#define MESH_WRITE_TRACE_OBB
#define HF_WRITE_TRACE

#endif

#endif
