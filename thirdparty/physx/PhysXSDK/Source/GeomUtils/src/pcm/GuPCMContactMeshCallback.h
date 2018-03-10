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

#ifndef GU_PCM_CONTACT_MESH_CALLBACK_H
#define GU_PCM_CONTACT_MESH_CALLBACK_H

#include "GuMidphase.h"
#include "GuEntityReport.h"
#include "GuHeightFieldUtil.h"
#include "GuTriangleCache.h"
#include "GuConvexEdgeFlags.h"


namespace physx
{

namespace Gu
{

template <typename Derived>
struct PCMMeshContactGenerationCallback : MeshHitCallback<PxRaycastHit>
{
public:
	const Cm::FastVertex2ShapeScaling&		mMeshScaling;
	const PxU8* PX_RESTRICT					mExtraTrigData;
	bool									mIdtMeshScale;
	static const PxU32 CacheSize = 16;
	Gu::TriangleCache<CacheSize>			mCache;

	PCMMeshContactGenerationCallback(const Cm::FastVertex2ShapeScaling& meshScaling, const PxU8* extraTrigData, bool idtMeshScale)
	:	MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE),
		mMeshScaling(meshScaling), mExtraTrigData(extraTrigData), mIdtMeshScale(idtMeshScale)
	{
	}

	void flushCache() 
	{
		if (!mCache.isEmpty())
		{
			((Derived*)this)->template processTriangleCache< CacheSize >(mCache);
			mCache.reset();
		}
	}

	virtual PxAgain processHit(
		const PxRaycastHit& hit, const PxVec3& v0, const PxVec3& v1, const PxVec3& v2, PxReal&, const PxU32* vinds)
	{

		if (!((Derived*)this)->doTest(v0, v1, v2))
			return true;

		PxVec3 v[3];
		if(mIdtMeshScale)
		{
			v[0] = v0;
			v[1] = v1;
			v[2] = v2;
		}
		else
		{
			v[0] = mMeshScaling * v0;
			v[1] = mMeshScaling * v1;
			v[2] = mMeshScaling * v2;
		}

		PxU32 triangleIndex = hit.faceIndex;

		PxU8 extraData = PX_IS_SPU || !mExtraTrigData ?
			PxU8(Gu::ETD_CONVEX_EDGE_01|Gu::ETD_CONVEX_EDGE_12|Gu::ETD_CONVEX_EDGE_20)
			: mExtraTrigData[triangleIndex];
		if (mCache.isFull())
		{
			((Derived*)this)->template processTriangleCache< CacheSize >(mCache);
			mCache.reset();
		}
		mCache.addTriangle(v, vinds, triangleIndex, extraData);

		return true;
	}

protected:
	PCMMeshContactGenerationCallback& operator=(const PCMMeshContactGenerationCallback&);
};

template <typename Derived>
struct PCMHeightfieldContactGenerationCallback :  Gu::EntityReport<PxU32>
{
public:
	const Gu::HeightFieldUtil&				mHfUtil;
	const PxTransform&						mHeightfieldTransform;

	PCMHeightfieldContactGenerationCallback(const Gu::HeightFieldUtil& hfUtil, const PxTransform& heightfieldTransform)	:
		mHfUtil(hfUtil), mHeightfieldTransform(heightfieldTransform)
	{

	}

	virtual PxAgain onEvent(PxU32 nb, PxU32* indices)
	{
		const PxU32 CacheSize = 16;
		Gu::TriangleCache<CacheSize> cache;

		PxU32 nbPasses = (nb+(CacheSize-1))/CacheSize;
		PxU32 nbTrigs = nb;
		PxU32* inds0 = indices;

		PxU8 nextInd[] = {2,0,1};

		for(PxU32 i = 0; i < nbPasses; ++i)
		{
			cache.mNumTriangles = 0;
			PxU32 trigCount = PxMin(nbTrigs, CacheSize);
			nbTrigs -= trigCount;
			while(trigCount--)
			{
				PxU32 triangleIndex = *(inds0++);
				PxU32 vertIndices[3];

				PxTriangle currentTriangle;	// in world space

				PxU32 adjInds[3];
				mHfUtil.getTriangle(mHeightfieldTransform, currentTriangle, vertIndices, adjInds, triangleIndex, false, false);

				PxVec3 normal;
				currentTriangle.normal(normal);

				PxU8 triFlags = 0; //KS - temporary until we can calculate triFlags for HF

				for(PxU32 a = 0; a < 3; ++a)
				{

					if(adjInds[a] != 0xFFFFFFFF)
					{
						PxTriangle adjTri;
						PxU32 inds[3];
						mHfUtil.getTriangle(mHeightfieldTransform, adjTri, inds, NULL, adjInds[a], false, false);
						//We now compare the triangles to see if this edge is active

						PX_ASSERT(inds[0] == vertIndices[a] || inds[1] == vertIndices[a] || inds[2] == vertIndices[a]);
						PX_ASSERT(inds[0] == vertIndices[(a+1)%3] || inds[1] == vertIndices[(a+1)%3] || inds[2] == vertIndices[(a+1)%3]);


						PxVec3 adjNormal;
						adjTri.denormalizedNormal(adjNormal);
						PxU32 otherIndex = nextInd[a];
						PxF32 projD = adjNormal.dot(currentTriangle.verts[otherIndex] - adjTri.verts[0]);

						if(projD < 0.f)
						{
							adjNormal.normalize();

							PxF32 proj = adjNormal.dot(normal);

							if(proj < 0.997f)
							{
								triFlags |= (1 << (a+3));
							}
						}
					}
					else
						triFlags |= (1 << (a+3));
				}

				cache.addTriangle(currentTriangle.verts, vertIndices, triangleIndex, triFlags);
			}
			PX_ASSERT(cache.mNumTriangles <= 16);

			((Derived*)this)->template processTriangleCache< CacheSize >(cache);
		}
		return true;
	}	
protected:
	PCMHeightfieldContactGenerationCallback& operator=(const PCMHeightfieldContactGenerationCallback&);
};

}//Gu
}//physx

#endif
