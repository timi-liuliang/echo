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

#include "GuContactBuffer.h"
#include "PxcContactMethodImpl.h"
#include "PxcNpCache.h"

using namespace physx;

namespace physx
{
bool PxcContactSpherePlane(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);
	PX_UNUSED(shape1);

	// Get actual shape data
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
	//const PxPlaneGeometry& shapePlane = shape1.get<const PxPlaneGeometry>();

	//Sphere in plane space
	const PxVec3 sphere = transform1.transformInv(transform0.p);
	
	//Make sure we have a normalized plane
	//The plane is implicitly n=<1,0,0> d=0 (in plane-space)
	//PX_ASSERT(PxAbs(shape1.mNormal.magnitudeSquared() - 1.0f) < 0.000001f);

	//Separation
	const PxReal separation = sphere.x - shapeSphere.radius;

	if(separation<=contactDistance)
	{
		const PxVec3 normal = transform1.q.getBasisVector0();
		const PxVec3 point  = transform0.p - normal * shapeSphere.radius;
		contactBuffer.contact(point, normal, separation);
		return true;
	}
	return false;
}
}
