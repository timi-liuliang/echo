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

#include "PsIntrinsics.h"
#include "PsMathUtils.h"
#include "GuGeomUtilsInternal.h"
#include "GuBox.h"
#include "GuCapsule.h"

using namespace physx;

/**
*	Computes an OBB surrounding the capsule.
*	\param		box		[out] the OBB
*/
void Gu::computeBoxAroundCapsule(const Gu::Capsule& capsule, Gu::Box& box)
{
	// Box center = center of the two capsule's endpoints
	box.center = capsule.computeCenter();

	// Box extents
	const PxF32 d = (capsule.p0 - capsule.p1).magnitude();
	box.extents.x = capsule.radius + (d * 0.5f);
	box.extents.y = capsule.radius;
	box.extents.z = capsule.radius;

	// Box orientation
	if(d==0.0f)
	{
		box.rot = PxMat33(PxIdentity);
	}
	else
	{
		PxVec3 dir, right, up;
		Ps::computeBasis(capsule.p0, capsule.p1, dir, right, up);
		box.setAxes(dir, right, up);
	}
}
