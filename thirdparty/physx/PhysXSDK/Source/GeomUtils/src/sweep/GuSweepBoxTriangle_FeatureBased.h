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

#ifndef GU_SWEEP_BOX_TRIANGLE_FEATURE_BASED_H
#define GU_SWEEP_BOX_TRIANGLE_FEATURE_BASED_H

#include "CmPhysXCommon.h"

namespace physx
{
	class PxTriangle;
	class PxBounds3;
	class PxVec3;

	namespace Gu
	{
		bool sweepBoxTriangle(	const PxTriangle& tri, const PxBounds3& box,
								const PxVec3& motion, const PxVec3& oneOverMotion,
								PxVec3& hit, PxVec3& normal, PxReal& d);

	} // namespace Gu
}

#endif
