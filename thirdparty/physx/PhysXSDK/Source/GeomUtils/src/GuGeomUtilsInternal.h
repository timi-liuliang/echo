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

#ifndef GU_GEOM_UTILS_INTERNAL_H
#define GU_GEOM_UTILS_INTERNAL_H

#include "CmPhysXCommon.h"
#include "PxBounds3.h"
#include "GuCapsule.h"
#include "GuBox.h"
#include "PxCapsuleGeometry.h"
#include "PxBoxGeometry.h"
#include "PsMathUtils.h"
#include "PsUtilities.h"

#define GU_EPSILON_SAME_DISTANCE 1e-3f

namespace physx
{
namespace Gu
{
	class Plane;
	class Capsule;
	class Box;
	class Segment;

	PX_PHYSX_COMMON_API const PxU8*		getBoxEdges();

	PX_PHYSX_COMMON_API void			computeBoxPoints(const PxBounds3& bounds, PxVec3* PX_RESTRICT pts);
	PX_PHYSX_COMMON_API void			computeBoundsAroundVertices(PxBounds3& bounds, PxU32 nbVerts, const PxVec3* PX_RESTRICT verts);

	PX_PHYSX_COMMON_API void			computeBoxAroundCapsule(const Capsule& capsule, Box& box);
	PX_FORCE_INLINE		void			computePxBoxAroundCapsule(const PxCapsuleGeometry& capsuleGeom, PxBoxGeometry& box)
	{
		box.halfExtents = PxVec3(capsuleGeom.radius + (capsuleGeom.halfHeight), capsuleGeom.radius, capsuleGeom.radius);
	}

	PX_PHYSX_COMMON_API PxPlane			getPlane(const PxTransform& pose);
	PX_PHYSX_COMMON_API PxTransform		getCapsuleTransform(const Gu::Capsule& capsule, PxReal& halfHeight);

	PX_FORCE_INLINE void getCapsuleSegment(const PxTransform& transform, const PxCapsuleGeometry& capsuleGeom, Gu::Segment& segment)
	{
		const PxVec3 tmp = transform.q.getBasisVector0() * capsuleGeom.halfHeight;
		segment.p0 = transform.p + tmp;
		segment.p1 = transform.p - tmp;
	}

	PX_FORCE_INLINE	void getCapsule(Gu::Capsule& capsule, const PxCapsuleGeometry& capsuleGeom, const PxTransform& pose)
	{
		getCapsuleSegment(pose, capsuleGeom, capsule);
		capsule.radius = capsuleGeom.radius;
	}

	// AP: common api prefix is needed for use in PxcSweepConvexMesh
	PX_PHYSX_COMMON_API void computeSweptBox(
		Gu::Box& box, const PxVec3& extents, const PxVec3& center, const PxMat33& rot, const PxVec3& unitDir, const PxReal distance);

	/**
	*	PT: computes "alignment value" used to select the "best" triangle in case of identical impact distances (for sweeps).
	*	This simply computes how much a triangle is aligned with a given sweep direction.
	*	Captured in a function to make sure it is always computed correctly, i.e. working for double-sided triangles.
	*
	*	\param		triNormal	[in] triangle's normal
	*	\param		unitDir		[in] sweep direction (normalized)
	*	\return		alignment value in [-1.0f, 0.0f]. -1.0f for fully aligned, 0.0f for fully orthogonal.
	*/
	PX_FORCE_INLINE PxReal computeAlignmentValue(const PxVec3& triNormal, const PxVec3& unitDir)
	{
		// PT: initial dot product gives the angle between the two, with "best" triangles getting a +1 or -1 score
		// depending on their winding. We take the absolute value to ignore the impact of winding. We negate the result
		// to make the function compatible with the initial code, which assumed single-sided triangles and expected -1
		// for best triangles.
		return -PxAbs(triNormal.dot(unitDir));
	}

	/**
	*	PT: sweeps: determines if a newly touched triangle is "better" than best one so far.
	*	In this context "better" means either clearly smaller impact distance, or a similar impact
	*	distance but a normal more aligned with the sweep direction.
	*
	*	\param		triImpactDistance	[in] new triangle's impact distance
	*	\param		triAlignmentValue	[in] new triangle's alignment value (as computed by computeAlignmentValue)
	*	\param		bestImpactDistance	[in] current best triangle's impact distance
	*	\param		bestAlignmentValue	[in] current best triangle's alignment value (as computed by computeAlignmentValue)
	*	\param		distEpsilon			[in] tris have "similar" impact distances if the difference is smaller than 2*distEpsilon
	*	\return		true if new triangle is better
	*/
	PX_FORCE_INLINE bool keepTriangle(	float triImpactDistance, float triAlignmentValue,
										float bestImpactDistance, float bestAlignmentValue,
										float distEpsilon)
	{
		// PT: make it a relative epsilon to make sure it still works with large distances
		distEpsilon *= PxMax(1.0f, PxMax(triImpactDistance, bestImpactDistance));

		// AP: if new distance is more than epsilon closer than old distance
		// or if new distance is no more than epsilon farther than oldDistance and "face is more opposing than previous"
		return (triImpactDistance < bestImpactDistance-distEpsilon || (triImpactDistance < bestImpactDistance+distEpsilon && triAlignmentValue < bestAlignmentValue));
	}

}  // namespace Gu

}

#endif
