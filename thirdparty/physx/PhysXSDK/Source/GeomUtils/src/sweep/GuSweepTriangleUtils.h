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

#ifndef GU_SWEEP_TRIANGLE_UTILS_H
#define GU_SWEEP_TRIANGLE_UTILS_H

#include "CmPhysXCommon.h"
#include "PxVec3.h"
#include "GuSweepSharedTests.h"

namespace physx
{

namespace Gu
{
	void computeSphereTriImpactData(PxVec3& hit, PxVec3& normal, const PxVec3& center, const PxVec3& dir, float t, const PxTriangle& tri);

	void computeBoxTriImpactData(	PxVec3& hit, PxVec3& normal, const PxVec3& boxExtents, const PxVec3& localDir, const PxVec3& localMotion,
									const PxVec3& oneOverMotion, const PxTriangle& triInBoxSpace);

	void computeEdgeEdgeNormal(PxVec3& normal, const PxVec3& p1, const PxVec3& p2_p1, const PxVec3& p3, const PxVec3& p4_p3, const PxVec3& dir, float d);

	PX_FORCE_INLINE PxU32 getInitIndex(const PxU32* PX_RESTRICT cachedIndex, PxU32 nbTris)
	{
		PxU32 initIndex = 0;
		if(cachedIndex)
		{
			PX_ASSERT(*cachedIndex < nbTris);
			PX_UNUSED(nbTris);
			initIndex = *cachedIndex;
		}
		return initIndex;
	}

#ifdef _XBOX
	// PT: returning a float is the fastest on Xbox!
	PX_FORCE_INLINE float cullTriangle(const PxTriangle& currentTri, const PxVec3& dir, PxReal radius, PxReal t, const PxReal dpc0)
#else
	GU_SWEEP_SHARED bool cullTriangle(const PxTriangle& currentTri, const PxVec3& dir, PxReal radius, PxReal t, const PxReal dpc0)
#endif
	{
		const PxReal dp0 = currentTri.verts[0].dot(dir);
		const PxReal dp1 = currentTri.verts[1].dot(dir);
		const PxReal dp2 = currentTri.verts[2].dot(dir);

#ifdef _XBOX
		// PT: we have 3 ways to write that on Xbox:
		// - with the original code: suffers from a lot of FCMPs
		// - with the cndt stuff below, cast to an int: it's faster, but suffers from a very bad LHS from the float-to-int
		// - with the cndt stuff not cast to an int: we get only one FCMP instead of many, the LHS is gone, that's the fastest solution. Even though it looks awkward.
		// AP: new correct implementation
		PxReal dp = dp0;
		dp = physx::intrinsics::selectMin(dp, dp1);
		dp = physx::intrinsics::selectMin(dp, dp2);

		using physx::intrinsics::fsel;

		//if(dp>dpc0 + t + radius) return false;
		const float cndt0 = fsel(dp - (dpc0 + t + radius + 0.01f), 0.0f, 1.0f);

		//if(dp0<dpc0 && dp1<dpc0 && dp2<dpc0) return false; <=>
		//if(dp0>=dpc0 || dp1>=dpc0 || dp2>=dpc0) return true;
		const float cndt1 = fsel(dp0-dpc0, 1.0f, 0.0f) + fsel(dp1-dpc0, 1.0f, 0.0f) + fsel(dp2-dpc0, 1.0f, 0.0f);

		return cndt0*cndt1;
		//PxReal resx = cndt0*cndt1;
#else
		PxReal dp = dp0;
		dp = physx::intrinsics::selectMin(dp, dp1);
		dp = physx::intrinsics::selectMin(dp, dp2);

		if(dp>dpc0 + t + radius + 0.01f)
		{
			//PX_ASSERT(resx == 0.0f);
			return false;
		}

		// ExperimentalCulling
		if(dp0<dpc0 && dp1<dpc0 && dp2<dpc0)
		{
			//PX_ASSERT(resx == 0.0f);
			return false;
		}

		//PX_ASSERT(resx != 0.0f);
		return true;
#endif
	}

	//
	//                     point
	//                      o
	//                   __/|
	//                __/ / |
	//             __/   /  |(B)
	//          __/  (A)/   |
	//       __/       /    |                dir
	//  p0 o/---------o---------------o--    -->
	//                t (t<=fT)       t (t>fT)
	//                return (A)^2    return (B)^2
	//
	//     |<-------------->|
	//             fT
	//
	//
	PX_FORCE_INLINE PxReal squareDistance(const PxVec3& p0, const PxVec3& dir, PxReal t, const PxVec3& point)
	{
		PxVec3 Diff = point - p0;
	/*	const PxReal fT = (Diff.dot(dir));
		if(fT>0.0f)
		{
			if(fT>=t)
				Diff -= dir*t;	// Take travel distance of point p0 into account (shortens the distance)
			else
				Diff -= fT*dir;
		}*/

		PxReal fT = (Diff.dot(dir));
		fT = physx::intrinsics::selectMax(fT, 0.0f);
		fT = physx::intrinsics::selectMin(fT, t);
		Diff -= fT*dir;

		return Diff.magnitudeSquared();
	}

	GU_SWEEP_SHARED bool coarseCulling(const PxVec3& center, const PxVec3& dir, PxReal t, PxReal radius, const PxTriangle& tri)
	{
		// ### could be precomputed
		const PxVec3 triCenter = (tri.verts[0] + tri.verts[1] + tri.verts[2]) * (1.0f/3.0f);

		// PT: distance between the triangle center and the swept path (an LSS)
		// Same as: distancePointSegmentSquared(center, center+dir*t, TriCenter);
		PxReal d = PxSqrt(squareDistance(center, dir, t, triCenter)) - radius - 0.0001f;

		if (d < 0.0f)	// The triangle center lies inside the swept sphere
			return true;

		d*=d;

		// ### distances could be precomputed
	/*	if(d <= (TriCenter-tri.verts[0]).magnitudeSquared())
		return true;
		if(d <= (TriCenter-tri.verts[1]).magnitudeSquared())
			return true;
		if(d <= (TriCenter-tri.verts[2]).magnitudeSquared())
			return true;
		return false;*/
		const PxReal d0 = (triCenter-tri.verts[0]).magnitudeSquared();
		const PxReal d1 = (triCenter-tri.verts[1]).magnitudeSquared();
		const PxReal d2 = (triCenter-tri.verts[2]).magnitudeSquared();
		PxReal triRadius = physx::intrinsics::selectMax(d0, d1);
		triRadius = physx::intrinsics::selectMax(triRadius, d2);
		if(d <= triRadius)
			return true;
		return false;
	}

	PX_FORCE_INLINE bool rejectTriangle(const PxVec3& center, const PxVec3& unitDir, PxReal curT, PxReal radius, const PxTriangle& currentTri, const PxReal dpc0)
	{
		if(!coarseCulling(center, unitDir, curT, radius, currentTri))
			return true;
#ifdef _XBOX
		if(cullTriangle(currentTri, unitDir, radius, curT, dpc0)==0.0f)
			return true;
#else
		if(!cullTriangle(currentTri, unitDir, radius, curT, dpc0))
			return true;
#endif
		return false;
	}

	PX_FORCE_INLINE bool shouldFlipNormal(const PxVec3& normal, bool meshBothSides, bool isDoubleSided, const PxVec3& triangleNormal, const PxVec3& dir)
	{
		// PT: this function assumes that input normal is opposed to the ray/sweep direction. This is always
		// what we want except when we hit a single-sided back face with 'meshBothSides' enabled.

		if(!meshBothSides || isDoubleSided)
			return false;

		PX_ASSERT(normal.dot(dir) <= 0.0f);	// PT: if this fails, the logic below cannot be applied
		PX_UNUSED(normal);
		return triangleNormal.dot(dir) > 0.0f;	// PT: true for back-facing hits
	}

} // namespace Gu

}

#endif
