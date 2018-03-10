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

#ifndef GU_RAYTRI_OVERLAP
#define GU_RAYTRI_OVERLAP

#include "PsVecMath.h"

using namespace physx::shdfnd::aos;
using namespace physx;
using namespace Cm;
using namespace Gu;

class SimpleRayTriOverlap
{
public:
	PX_FORCE_INLINE SimpleRayTriOverlap(const PxVec3& origin, const PxVec3& dir, bool bothSides, PxReal geomEpsilon)
		: mOrigin(origin), mDir(dir), mBothSides(bothSides), mGeomEpsilon(geomEpsilon)
	{
	}

	// PT: TODO: this is redundant with Gu::IntersectionRayTriangle.h
	PX_FORCE_INLINE Ps::IntBool overlap(const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2, PxRaycastHit& hit) const
	{
		// AP: looks like this computation is based on "Fast Minimum Storage Ray Triangle Intersection" paper by Moeller et al
		const PxVec3 edge1 = vert1 - vert0;
		const PxVec3 edge2 = vert2 - vert0;

		const PxVec3 p = mDir.cross(edge2);
		const PxReal det = edge1.dot(p);
		const PxReal epsilon = mGeomEpsilon;

		if (!mBothSides)
		{
			if (det<epsilon)
				return Ps::IntFalse;

			//epsilon *= PxAbs(det); // scale the epsilon to improve precision

			const PxVec3 t = mOrigin - vert0;
			const PxReal u = t.dot(p); // (origin-v0).dot(dir x e2)

			if(u < -epsilon || u > det + epsilon)	
				return Ps::IntFalse;

			const PxVec3 q = t.cross(edge1);

			const PxReal v = mDir.dot(q);
			if(v < -epsilon || (u + v) > det + epsilon)	
				return Ps::IntFalse;

			const float oneOverDet = 1.0f / det;
			const PxReal d = edge2.dot(q)*oneOverDet;
			if(d<-epsilon) // test if the ray intersection t is really negative
				return Ps::IntFalse;

			hit.distance	= PxMax(d, 0.0f);
			hit.u			= oneOverDet * u;
			hit.v			= oneOverDet * v;
		} else
		{
			if (PxAbs(det)<epsilon)
				return Ps::IntFalse;
			PxReal invDet = 1.0f/det;

			const PxVec3 t = mOrigin - vert0;
			const PxReal u = t.dot(p)*invDet; // (origin-v0).dot(dir x e2)

			if(u < -epsilon || u > 1.0f + epsilon)	
				return Ps::IntFalse;

			const PxVec3 q = t.cross(edge1);

			const PxReal v = mDir.dot(q)*invDet;
			if(v < -epsilon || (u + v) > 1.0f + epsilon)	
				return Ps::IntFalse;

			const PxReal d = edge2.dot(q)*invDet;
			if(d<-epsilon) // test if the ray intersection t is really negative
				return Ps::IntFalse;

			hit.distance	= PxMax(d, 0.0f);
			hit.u			= u;
			hit.v			= v;
		}

		return Ps::IntTrue;
	}

	PxVec3	mOrigin;
	PxVec3	mDir;
	bool	mBothSides;
	PxReal	mGeomEpsilon;
};

#endif // header guard
