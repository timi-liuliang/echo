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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "PsIntrinsics.h"
#include "GuMidphase.h"
#include "GuIntersectionRayBoxSIMD.h"
#include "GuGeomUtilsInternal.h"
#include "CmMemFetch.h"
#include "CmMatrix34.h"
#include "PsVecMath.h"
#include "GuHeightField.h"
#include "PxMeshQuery.h"
#include "PxHeightFieldGeometry.h"
#include "PxTriangle.h"
#include "GuMidphaseTrace.h"
#include "GuRayTriOverlap.h"


#ifdef PX_WIIU  
#pragma ghs nowarning 236 //controlling expression is constant
#endif

using Gu::RTree;

// This callback comes from RTree and decodes LeafTriangle indices stored in rtree into actual triangles
// This callback is needed because RTree doesn't know that it stores triangles since it's a general purpose spatial index

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

template <int tInflate, bool tRayTest>
struct RayRTreeCallback : RTree::CallbackRaycast, RTree::Callback
{
	const RTreeMidphaseData& model;
	MeshHitCallback<PxRaycastHit>& outerCallback;
	PxU32 has16BitIndices;
	MemFetchPtr* mTris;
	const MemFetchPtr* mVerts;
	const PxVec3* mInflate;
	const SimpleRayTriOverlap rayCollider;
	PxReal maxT;
	PxRaycastHit closestHit; // recorded closest hit over the whole traversal (only for callback mode eCLOSEST)
	PxVec3 cv0, cv1, cv2;	// PT: make sure these aren't last in the class, to safely V4Load them
	PxU32 cis[3];
	bool hadClosestHit;
	const bool closestMode;
	Vec3V inflateV, rayOriginV, rayDirV;
	#if MIDPHASE_TRACE
	Ps::Array<PxU32> touchedLeaves;
	#endif

	RayRTreeCallback(
		const RTreeMidphaseData& model_, MeshHitCallback<PxRaycastHit>& callback,
		PxU32 has16BitIndices_, MemFetchPtr* tris, MemFetchPtr* verts,
		const PxVec3& origin, const PxVec3& dir, PxReal maxT_, bool bothSides, const PxVec3* inflate)
		:	model(model_), outerCallback(callback), has16BitIndices(has16BitIndices_),
			mTris(tris), mVerts(verts), mInflate(inflate), rayCollider(origin, dir, bothSides, model.mGeomEpsilon), maxT(maxT_),
			closestMode(callback.inClosestMode())
	{
		PX_ASSERT(closestHit.distance == PX_MAX_REAL);
		hadClosestHit = false;
		if (tInflate)
			inflateV = V3LoadU(*mInflate);
		rayOriginV = V3LoadU(rayCollider.mOrigin);
		rayDirV = V3LoadU(rayCollider.mDir);
	}

	PX_FORCE_INLINE void getVertIndices(PxU32 triIndex, PxU32& i0, PxU32 &i1, PxU32 &i2)
	{
		if(has16BitIndices)
		{
			const PxU16* p = reinterpret_cast<const PxU16*>(*mTris + triIndex*3*sizeof(PxU16));
			i0 = p[0]; i1 = p[1]; i2 = p[2];
		}
		else
		{
			const PxU32* p = reinterpret_cast<const PxU32*>(*mTris + triIndex*3*sizeof(PxU32));
			i0 = p[0]; i1 = p[1]; i2 = p[2];
		}
	}

	virtual PX_FORCE_INLINE bool processResults(PxU32 NumTouched, PxU32* Touched, PxF32& newMaxT)
	{
		PX_ASSERT(NumTouched == 1);
		//pxPrintf("in processResults, opccollider\n");
		PX_ASSERT(NumTouched > 0);
		// Loop through touched leaves
		PxRaycastHit tempHit;
		for(PxU32 leaf = 0; leaf<NumTouched; leaf++)
		{
			// Each leaf box has a set of triangles
			LeafTriangles currentLeaf;
			currentLeaf.Data = Touched[leaf];
			PxU32 nbLeafTris = currentLeaf.GetNbTriangles();			
			PxU32 baseLeafTriIndex = currentLeaf.GetTriangleIndex();

#if PX_IS_SPU
			// on SPU we fetch verts on 8 parallel DMA channels
			const PxU32 N = 8;
			for(PxU32 iTri = 0; iTri < nbLeafTris; iTri+=N)
			{
				PxVec3 v[N+1][3];	// PT: +1 to make sure we can V4Load them
				PxU32 inds[N][3];
				PxU32 countLeft = iTri+N > nbLeafTris ? nbLeafTris-iTri : N;
				MeshInterface::getTriangleVertsN<N>(has16BitIndices, *mTris, *mVerts, baseLeafTriIndex+iTri, countLeft, v, inds);
				for(PxU32 jj = 0; jj < countLeft; jj++)
				{
					const PxU32 triangleIndex = baseLeafTriIndex+iTri+jj;
					const PxVec3& v0 = v[jj][0];
					const PxVec3& v1 = v[jj][1];
					const PxVec3& v2 = v[jj][2];
					const PxU32* vinds = inds[jj];

#else // #if PX_IS_SPU
			for(PxU32 i = 0; i < nbLeafTris; i++)
			{{ // double brace to make the inner loop compatible with the double loop SPU section above, without code duplication
				PxU32 i0, i1, i2;
				const PxU32 triangleIndex = baseLeafTriIndex+i;
				getVertIndices(triangleIndex, i0, i1, i2);

				const PxVec3* verts = reinterpret_cast<PxVec3*>(*mVerts);
				const PxVec3& v0 = verts[i0], &v1 = verts[i1], &v2 = verts[i2];
				const PxU32 vinds[3] = { i0, i1, i2 };
#endif // #if PX_IS_SPU

				if (tRayTest)
				{
					Ps::IntBool overlap;
					if (tInflate)
					{
						// AP: mesh skew is already included here (ray is pre-transformed)
						Vec3V v0v = V3LoadU(v0), v1v = V3LoadU(v1), v2v = V3LoadU(v2);
						Vec3V minB = V3Min(V3Min(v0v, v1v), v2v), maxB = V3Max(V3Max(v0v, v1v), v2v);

						// PT: we add an epsilon to max distance, to make sure we don't reject triangles that are just at the same
						// distance as best triangle so far. We need to keep all of these to make sure we return the one with the
						// best normal.
						const float relativeEpsilon = GU_EPSILON_SAME_DISTANCE * PxMax(1.0f, maxT);
						FloatV tNear, tFar;
						overlap = Gu::intersectRayAABB2(
							V3Sub(minB, inflateV), V3Add(maxB, inflateV), rayOriginV, rayDirV, FLoad(maxT+relativeEpsilon), tNear, tFar);
						if (overlap)
						{
							// can't clip to tFar here because hitting the AABB doesn't guarantee that we can clip
							// (since we can still miss the actual tri)
							tempHit.distance = maxT;
							tempHit.faceIndex = triangleIndex;
							tempHit.u = tempHit.v = 0.0f;
						}
					} else
						overlap = rayCollider.overlap(v0, v1, v2, tempHit) && tempHit.distance <= maxT;
					if(!overlap)
						continue;
				}
				tempHit.faceIndex = triangleIndex;
				tempHit.flags = PxHitFlag::ePOSITION|PxHitFlag::eDISTANCE;

				#if MIDPHASE_TRACE
				touchedLeaves.pushBack(tempHit.faceIndex);
				#endif

				// Intersection point is valid if dist < segment's length
				// We know dist>0 so we can use integers
				if (closestMode)
				{
					if(tempHit.distance < closestHit.distance)
					{
						PX_ASSERT(tempHit.distance >= 0.0f);
						closestHit = tempHit;
						newMaxT = PxMin(tempHit.distance, newMaxT);
						cv0 = v0; cv1 = v1; cv2 = v2;
						cis[0] = vinds[0]; cis[1] = vinds[1]; cis[2] = vinds[2];
						hadClosestHit = true;
					}
				} else
				{
					PxReal shrunkMaxT = newMaxT;
					//pxPrintf("calling processHit\n");
					PxAgain again = outerCallback.processHit(tempHit, v0, v1, v2, shrunkMaxT, vinds);
					if (!again)
						return false;
					if (shrunkMaxT < newMaxT)
					{
						newMaxT = shrunkMaxT;
						maxT = shrunkMaxT;
					}
				}

				if (outerCallback.inAnyMode()) // early out if in ANY mode
					return false;
			}} // for SPU code sharing

		} // for(PxU32 leaf = 0; leaf<NumTouched; leaf++)

		return true;
	}

	virtual bool processResults(PxU32 numTouched, PxU32* touched)
	{
		PxF32 dummy;
		return RayRTreeCallback::processResults(numTouched, touched, dummy);
	}


	virtual ~RayRTreeCallback()
	{
		if (hadClosestHit)
		{
			PX_ASSERT(outerCallback.inClosestMode());
			outerCallback.processHit(closestHit, cv0, cv1, cv2, maxT, cis);
		}
	}

private:
	RayRTreeCallback& operator=(const RayRTreeCallback&);
};

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif


void MeshRayCollider::collideOBB(
	const Gu::Box& obb, bool bothTriangleSidesCollide, const RTreeMidphaseData& model, MeshHitCallback<PxRaycastHit>& callback,
	bool checkObbIsAligned
	)
{
	const MeshInterface* mi = model.mIMesh;
	MemFetchSmallBuffer buf0, buf1, buf2;
	PxU32* has16BitIndices = memFetchAsync<PxU32>(MemFetchPtr(mi)+PX_OFFSET_OF(MeshInterface, mHas16BitIndices), 5, buf0);
	MemFetchPtr* mTris = memFetchAsync<MemFetchPtr>(MemFetchPtr(mi)+PX_OFFSET_OF(MeshInterface, mTris), 5, buf1);
	MemFetchPtr* mVerts = memFetchAsync<MemFetchPtr>(MemFetchPtr(mi)+PX_OFFSET_OF(MeshInterface, mVerts), 5, buf2);
	memFetchWait(5);

	const PxU32 maxResults = Gu::RTreePage::SIZE; // maxResults=rtree page size for more efficient early out
	PxU32 buf[maxResults];
	RayRTreeCallback<false, false> rTreeCallback(
		model, callback, *has16BitIndices, mTris, mVerts, PxVec3(0), PxVec3(0), 0.0f, bothTriangleSidesCollide, NULL);
	if (checkObbIsAligned && PxAbs(PxQuat(obb.rot).w) > 0.9999f && !PX_IS_SPU)
	{
		PxVec3 aabbExtents = obb.computeAABBExtent();
		model.mRTree->traverseAABB(obb.center - aabbExtents, obb.center + aabbExtents, maxResults, buf, &rTreeCallback);
	} else
		model.mRTree->traverseOBB(obb, maxResults, buf, &rTreeCallback);

	MESH_WRITE_TRACE_OBB
}

template <int tInflate, int tRayTest>
void MeshRayCollider::collide(
	const PxVec3& orig, const PxVec3& dir, PxReal maxT, bool bothSides,
	const RTreeMidphaseData& model, MeshHitCallback<PxRaycastHit>& callback,
	const PxVec3* inflate)
{
	const MeshInterface* mi = model.mIMesh;
	MemFetchSmallBuffer buf0, buf1, buf2;
	PxU32* has16BitIndices = memFetchAsync<PxU32>(MemFetchPtr(mi)+PX_OFFSET_OF(MeshInterface, mHas16BitIndices), 5, buf0);
	MemFetchPtr* mTris = memFetchAsync<MemFetchPtr>(MemFetchPtr(mi)+PX_OFFSET_OF(MeshInterface, mTris), 5, buf1);
	MemFetchPtr* mVerts = memFetchAsync<MemFetchPtr>(MemFetchPtr(mi)+PX_OFFSET_OF(MeshInterface, mVerts), 5, buf2);
	memFetchWait(5);

	const PxU32 maxResults = Gu::RTreePage::SIZE; // maxResults=rtree page size for more efficient early out
	PxU32 buf[maxResults];
	if (maxT == 0.0f && !PX_IS_SPU) // AABB traversal path
	{
		RayRTreeCallback<tInflate, false> rTreeCallback(
			model, callback, *has16BitIndices, mTris, mVerts, orig, dir, maxT, bothSides, inflate);
		PxVec3 inflate1 = tInflate ? *inflate : PxVec3(0); // both maxT and inflate can be zero, so need to check tInflate
		model.mRTree->traverseAABB(orig-inflate1, orig+inflate1, maxResults, buf, &rTreeCallback);
	}
	else // ray traversal path
	{
		if (PX_IS_SPU)
			maxT = PxMax(maxT, 1e-3f); // make sure maxT is non-zero on SPU since we skip traverseAABB path because of memory constraints
		RayRTreeCallback<tInflate, tRayTest> rTreeCallback(
			model, callback, *has16BitIndices, mTris, mVerts, orig, dir, maxT, bothSides, inflate);
		model.mRTree->traverseRay<tInflate>(orig, dir, maxResults, buf, &rTreeCallback, inflate, maxT);
	}

	MESH_WRITE_TRACE
}


#define TINST(a,b) \
template void MeshRayCollider::collide<a,b>( \
	const PxVec3& orig, const PxVec3& dir, PxReal maxT, bool bothSides, const RTreeMidphaseData& model, \
	MeshHitCallback<PxRaycastHit>& callback, const PxVec3* inflate);

TINST(0,0)
TINST(1,0)
TINST(0,1)
TINST(1,1)

#undef TINST

