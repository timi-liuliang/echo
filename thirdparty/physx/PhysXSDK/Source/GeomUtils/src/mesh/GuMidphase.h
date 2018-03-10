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

#ifndef OPC_RAYCOLLIDER_H
#define OPC_RAYCOLLIDER_H

#include "PxSimpleTypes.h"
#include "PxQueryReport.h"
#include "PxVec3.h"
#include "GuMeshInterface.h"
#include "GuRTree.h"
#include "PsMathUtils.h"
#include "GuBox.h"
#include "intersection/GuIntersectionRayBox.h"

namespace physx
{
	
namespace Cm
{
class Matrix34;
}

namespace Gu {

class MeshInterface;
struct RTreeMidphaseData;

struct CallbackMode { enum Enum { eANY, eCLOSEST, eMULTIPLE }; };

template<typename HitType>
struct MeshHitCallback
{
	CallbackMode::Enum mode;

	MeshHitCallback(CallbackMode::Enum aMode) : mode(aMode) {}

	bool inAnyMode() const { return mode == CallbackMode::eANY; }
	bool inClosestMode() const { return mode == CallbackMode::eCLOSEST; }
	bool inMultipleMode() const { return mode == CallbackMode::eMULTIPLE; }

	virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
		const HitType& hit, const PxVec3& v0, const PxVec3& v1, const PxVec3& v2, PxReal& shrunkMaxT, const PxU32* vIndices) = 0;

	virtual ~MeshHitCallback() {}
};

PX_INLINE void computeSweptAABBAroundOBB(
	const Gu::Box& obb, PxVec3& sweepOrigin, PxVec3& sweepExtents, PxVec3& sweepDir, PxReal& sweepLen)
{
	PxU32 other1, other2;
	// largest axis of the OBB is the sweep direction, sum of abs of two other is the swept AABB extents
	PxU32 lai = Ps::largestAxis(obb.extents, other1, other2);
	PxVec3 longestAxis = obb.rot[lai]*obb.extents[lai];
	PxVec3 absOther1 = obb.rot[other1].abs()*obb.extents[other1];
	PxVec3 absOther2 = obb.rot[other2].abs()*obb.extents[other2];
	sweepOrigin = obb.center - longestAxis;
	sweepExtents = absOther1 + absOther2 + PxVec3(GU_MIN_AABB_EXTENT); // see comments for GU_MIN_AABB_EXTENT
	sweepLen = 2.0f; // length is already included in longestAxis
	sweepDir = longestAxis;
}

struct MeshRayCollider
{
	template <int tInflate, int tRayTest>
	PX_PHYSX_COMMON_API static void collide(
		const PxVec3& orig, const PxVec3& dir, // dir is not normalized (full length), both in mesh space (unless meshWorld is non-zero)
		PxReal maxT, // maxT is from [0,1], if maxT is 0.0f, AABB traversal will be used
		bool bothTriangleSidesCollide, const RTreeMidphaseData& model, MeshHitCallback<PxRaycastHit>& callback,
		const PxVec3* inflate = NULL);

	PX_PHYSX_COMMON_API static void collideOBB(
		const Gu::Box& obb, bool bothTriangleSidesCollide, const RTreeMidphaseData& model, MeshHitCallback<PxRaycastHit>& callback,
		bool checkObbIsAligned = true); // perf hint, pass false if obb is rarely axis aligned
};

} } // namespace physx::Ice

#endif
