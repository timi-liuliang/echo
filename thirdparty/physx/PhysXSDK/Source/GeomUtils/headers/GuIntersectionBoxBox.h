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

#ifndef GU_INTERSECTION_BOX_BOX_H
#define GU_INTERSECTION_BOX_BOX_H

#include "PxMat33.h"
#include "GuBox.h"
#include "PxBounds3.h"

namespace physx
{
namespace Gu
{
	PX_PHYSX_COMMON_API bool intersectOBBOBB(const PxVec3& e0, const PxVec3& c0, const PxMat33& r0, const PxVec3& e1, const PxVec3& c1, const PxMat33& r1, bool full_test);

	PX_FORCE_INLINE bool intersectOBBAABB(const Gu::Box& obb, const PxBounds3& aabb)
	{
		PxVec3 center = aabb.getCenter();
		PxVec3 extents = aabb.getExtents();
		return intersectOBBOBB(obb.extents, obb.center, obb.rot, extents, center, PxMat33(PxIdentity), true);
	}

} // namespace Gu

}

#endif
