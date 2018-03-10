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

#ifndef GU_CONVEX_UTILS_INTERNALS_H
#define GU_CONVEX_UTILS_INTERNALS_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"

namespace physx
{
class PxMeshScale;
class PxConvexMeshGeometry;
class PxTransform;
class PxConvexMesh;

namespace Cm
{
	class Matrix34;
	class FastVertex2ShapeScaling;
}

namespace Gu
{
	class Box;

	PX_PHYSX_COMMON_API void computeHullOBB(
		Gu::Box& hullOBB, const PxBounds3& hullAABB, float offset, const PxTransform& transform0, const Cm::Matrix34& world0,
		const Cm::Matrix34& world1, const Cm::FastVertex2ShapeScaling& meshScaling, bool idtScaleMesh);

	// src = input
	// computes a box in vertex space (including skewed scale) from src world box
	PX_PHYSX_COMMON_API void computeVertexSpaceOBB(
		Gu::Box& dst, const Gu::Box& src, const PxTransform& meshPose, const PxMeshScale& meshScale);

	PX_PHYSX_COMMON_API void computeOBBAroundConvex(
		Gu::Box& obb, const PxConvexMeshGeometry& convexGeom, const PxConvexMesh* cm, const PxTransform& convexPose);

}  // namespace Gu

}

#endif
