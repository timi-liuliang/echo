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

#ifndef GU_SWEEP_SPHERE_TRIANGLE_H
#define GU_SWEEP_SPHERE_TRIANGLE_H

#include "CmPhysXCommon.h"
#include "PxVec3.h"
#include "PxQueryReport.h"

namespace physx
{
	class PxTriangle;

namespace Gu
{
	bool sweepTriSphere(const PxTriangle& tri, const PxVec3& normal, const PxVec3& center, PxReal radius, const PxVec3& dir, PxReal& min_dist);

	bool sweepSphereTriangles(	PxU32 nbTris, const PxTriangle* PX_RESTRICT triangles,								// Triangle data
								const PxVec3& center, const PxReal radius,											// Sphere data
								const PxVec3& unitDir, PxReal distance,												// Ray data
								const PxU32* PX_RESTRICT cachedIndex,												// Cache data
								PxVec3& _hit, PxVec3& _normal, PxReal& _t, PxU32& _index, PxHitFlags& _outFlags,	// Results
								PxVec3& triNormal,
								bool isDoubleSided, bool meshBothSides);											// Query modifiers

} // namespace Gu

}

#endif
