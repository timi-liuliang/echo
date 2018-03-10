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

#include "PxcContactMethodImpl.h"
#include "PxcNpCache.h"
#include "GuContactBuffer.h"
#include "GuGeomUtilsInternal.h"

using namespace physx;
using namespace Gu;

namespace physx
{
bool PxcContactPlaneCapsule(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);
	PX_UNUSED(shape0);

	// Get actual shape data
	//const PxPlaneGeometry& shapePlane = shape.get<const PxPlaneGeometry>();
	const PxCapsuleGeometry& shapeCapsule = shape1.get<const PxCapsuleGeometry>();

	const PxTransform capsuleToPlane = transform0.transformInv(transform1);

	//Capsule in plane space
	Gu::Segment segment;
	getCapsuleSegment(capsuleToPlane, shapeCapsule, segment);
	
	const PxVec3 negPlaneNormal = transform0.q.getBasisVector0();
	
	bool contact = false;

	const PxReal separation0 = segment.p0.x - shapeCapsule.radius;
	const PxReal separation1 = segment.p1.x - shapeCapsule.radius;
	if(separation0 <= contactDistance)
	{
		const PxVec3 temp(segment.p0.x - shapeCapsule.radius, segment.p0.y, segment.p0.z);
		const PxVec3 point = transform0.transform(temp);
		contactBuffer.contact(point, -negPlaneNormal, separation0);
		contact = true;
	}

	if(separation1 <= contactDistance)
	{
		const PxVec3 temp(segment.p1.x - shapeCapsule.radius, segment.p1.y, segment.p1.z);
		const PxVec3 point = transform0.transform(temp);
		contactBuffer.contact(point, -negPlaneNormal, separation1);
		contact = true;
	}
	return contact;
}
}
