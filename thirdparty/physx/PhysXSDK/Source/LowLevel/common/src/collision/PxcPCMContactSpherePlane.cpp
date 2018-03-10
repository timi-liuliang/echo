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
bool PxcPCMContactSpherePlane(CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;
	PX_UNUSED(npCache);
	PX_UNUSED(shape1);

	// Get actual shape data
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();

	//sphere transform
	const Vec3V p0 = Vec3V_From_Vec4V(V4LoadU(&transform0.p.x));

	//plane transform
	const Vec3V p1 = Vec3V_From_Vec4V(V4LoadU(&transform1.p.x));
	const QuatV q1 = QuatVLoadU(&transform1.q.x);

	const FloatV radius = FLoad(shapeSphere.radius);
	const FloatV contactDist = FLoad(contactDistance);

	const PsTransformV transf1(p1, q1);
	//Sphere in plane space
	const Vec3V sphereCenterInPlaneSpace = transf1.transformInv(p0);
	

	//Separation
	const FloatV separation = FSub(V3GetX(sphereCenterInPlaneSpace), radius);

	if(FAllGrtrOrEq(contactDist, separation))
	{
		//get the plane normal
		const Vec3V worldNormal = QuatGetBasisVector0(q1);
		const Vec3V worldPoint = V3NegScaleSub(worldNormal, radius, p0);
		Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];
		//Fast allign store
		V4StoreA(Vec4V_From_Vec3V(worldNormal), (PxF32*)&contact.normal.x);
		V4StoreA(Vec4V_From_Vec3V(worldPoint), (PxF32*)&contact.point.x);
		FStore(separation, &contact.separation);
		contact.internalFaceIndex0 = PXC_CONTACT_NO_FACE_INDEX;
		contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;
	
		return true;
	}
	return false;
}
}
