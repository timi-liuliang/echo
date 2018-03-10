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


#include "GuGeometryUnion.h"

#include "PxcNpCache.h"
#include "PxcNpThreadContext.h"
#include "PxcContactMethodImpl.h"

using namespace physx;
using namespace Gu;

namespace physx
{
bool PxcPCMContactSphereSphere(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);

	using namespace Ps::aos;
	const PxSphereGeometry& shapeSphere0 = shape0.get<const PxSphereGeometry>();
	const PxSphereGeometry& shapeSphere1 = shape1.get<const PxSphereGeometry>();
	
	const FloatV cDist	= FLoad(contactDistance);
	const Vec3V p0 =  V3LoadA(&transform0.p.x);
	const Vec3V p1 =  V3LoadA(&transform1.p.x);

	const FloatV r0		= FLoad(shapeSphere0.radius);
	const FloatV r1		= FLoad(shapeSphere1.radius);
	

	const Vec3V _delta = V3Sub(p0, p1);
	const FloatV distanceSq = V3Dot(_delta, _delta);
	const FloatV radiusSum = FAdd(r0, r1);
	const FloatV inflatedSum = FAdd(radiusSum, cDist);
	
	if(FAllGrtr(FMul(inflatedSum, inflatedSum), distanceSq))
	{
		const FloatV eps	=  FLoad(0.00001f);
		const FloatV nhalf	= FLoad(-0.5f);
		const FloatV magn = FSqrt(distanceSq);
		const BoolV bCon = FIsGrtrOrEq(eps, magn);
		const Vec3V normal = V3Sel(bCon, V3UnitX(), V3ScaleInv(_delta, magn)); 
		const FloatV scale = FMul(FSub(FAdd(r0, magn), r1), nhalf);
		const Vec3V point = V3ScaleAdd(normal, scale, p0);
		const FloatV dist =  FSub(magn, radiusSum);
		
		PX_ASSERT(contactBuffer.count < ContactBuffer::MAX_CONTACTS);
		Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];
		V4StoreA(Vec4V_From_Vec3V(normal), (PxF32*)&contact.normal.x);
		V4StoreA(Vec4V_From_Vec3V(point), (PxF32*)&contact.point.x);
		FStore(dist, &contact.separation);

		contact.internalFaceIndex0 = PXC_CONTACT_NO_FACE_INDEX;
		contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;

		return true;
	}
	return false;
}

}
