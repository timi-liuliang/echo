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

#ifndef GU_SWEEP_CAPSULE_TRIANGLE_H
#define GU_SWEEP_CAPSULE_TRIANGLE_H

#include "CmPhysXCommon.h"
#include "PxVec3.h"
#include "PxQueryReport.h"

namespace physx
{
	class PxTriangle;

namespace Gu
{
	class Box;
	class Capsule;

	bool sweepCapsuleTriangles(	PxU32 nbTris, const PxTriangle* PX_RESTRICT triangles,							// Triangle data
								const Capsule& capsule,															// Capsule data
								const PxVec3& unitDir, const PxReal distance,									// Ray data
								const PxU32* PX_RESTRICT cachedIndex,											// Cache data
								PxHitFlags& outFlags, PxF32& t, PxVec3& normal, PxVec3& hit, PxU32& hitIndex,	// Results
								PxVec3& triNormal,
								PxHitFlags hintFlags, bool isDoubleSided,										// Query modifiers
								const Box* cullBox=NULL);														// Cull data

} // namespace Gu

}

#endif
