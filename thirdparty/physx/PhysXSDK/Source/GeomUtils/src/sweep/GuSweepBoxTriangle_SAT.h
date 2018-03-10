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

#ifndef GU_SWEEP_BOX_TRIANGLE_SAT_H
#define GU_SWEEP_BOX_TRIANGLE_SAT_H

#include "CmPhysXCommon.h"
#include "PxVec3.h"

namespace physx
{
	class PxTriangle;

namespace Gu
{
	int triBoxSweepTestBoxSpace(const PxTriangle& tri, const PxVec3& extents, const PxVec3& dir, const PxVec3& oneOverDir, float tmax, float& toi, bool doBackfaceCulling);

} // namespace Gu

}

#endif
