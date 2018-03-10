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

//#include "GuGJKWrapper.h"
#include "GuVecBox.h"
#include "GuVecSphere.h"
#include "GuGeometryUnion.h"

#include "PxcNpCache.h"
#include "PxcNpThreadContext.h"    
#include "PxcContactMethodImpl.h"

using namespace physx;
using namespace Gu;

namespace physx
{

bool PxcPCMContactSphereBox(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);

	using namespace Ps::aos;
	// Get actual shape data
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
	const PxBoxGeometry& shapeBox = shape1.get<const PxBoxGeometry>();
	//

	//const PsTransformV transf0(transform0);
	const Vec3V sphereOrigin = V3LoadA(&transform0.p.x);
	//const PsTransformV transf1(transform1);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&transform1.p.x);

	const FloatV radius = FLoad(shapeSphere.radius);
	
	const PsTransformV transf1(p1, q1);
	
	const FloatV cDist = FLoad(contactDistance);

	const Vec3V boxExtents = V3LoadU(shapeBox.halfExtents);

	//translate sphere center into the box space
	const Vec3V sphereCenter = transf1.transformInv(sphereOrigin);

	const Vec3V nBoxExtents = V3Neg(boxExtents);

	//const FloatV radSq = FMul(radius, radius);

	const FloatV inflatedSum = FAdd(radius, cDist);
	const FloatV sqInflatedSum = FMul(inflatedSum, inflatedSum);

	const Vec3V p = V3Clamp(sphereCenter, nBoxExtents, boxExtents);
	const Vec3V v = V3Sub(sphereCenter, p);
	const FloatV lengthSq = V3Dot(v, v);

	PX_ASSERT(contactBuffer.count < ContactBuffer::MAX_CONTACTS);

	if(FAllGrtr(sqInflatedSum, lengthSq))//intersect
	{
		const BoolV bTrue = BTTTT();
		//check whether the spherCenter is inside the box
		const BoolV bInsideBox = V3IsGrtrOrEq(boxExtents, V3Abs(sphereCenter));
		if(BAllEq(BAllTrue3(bInsideBox), bTrue))//sphere center inside the box
		{
			//Pick directions and sign
			const Vec3V absP = V3Abs(p);
			const Vec3V distToSurface = V3Sub(boxExtents, absP);//dist from embedded center to box surface along 3 dimensions.
			
			const FloatV x = V3GetX(distToSurface);
			const FloatV y = V3GetY(distToSurface);
			const FloatV z = V3GetZ(distToSurface);

			const Vec3V xV = V3Splat(x);
			const Vec3V zV = V3Splat(z);

			//find smallest element of distToSurface
			const BoolV con0 = BAllTrue3(V3IsGrtrOrEq(distToSurface, zV));
			const BoolV con1 = BAllTrue3(V3IsGrtrOrEq(distToSurface, xV));
			const Vec3V sign = V3Sign(p);
	
			const Vec3V tmpX = V3Mul(V3UnitX(), sign);
			const Vec3V tmpY = V3Mul(V3UnitY(), sign);
			const Vec3V tmpZ = V3Mul(V3UnitZ(), sign);
			
			const Vec3V locNorm= V3Sel(con0, tmpZ, V3Sel(con1, tmpX, tmpY));////local coords contact normal
			const FloatV dist = FNeg(FSel(con0, z, FSel(con1, x, y)));

			//separation so far is just the embedding of the center point; we still have to push out all of the radius.
			const Vec3V point = sphereOrigin;
			const Vec3V normal = transf1.rotate(locNorm);
			const FloatV penetration = FSub(dist, radius);


			Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];
			V4StoreA(Vec4V_From_Vec3V(normal), (PxF32*)&contact.normal.x);
			V4StoreA(Vec4V_From_Vec3V(point), (PxF32*)&contact.point.x);
			FStore(penetration, &contact.separation);

			contact.internalFaceIndex0 = PXC_CONTACT_NO_FACE_INDEX;
			contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;
			//context.mContactBuffer.contact(point, normal, penetration);
		}
		else
		{
			//get the closest point from the center to the box surface
			const FloatV recipLength = FRsqrt(lengthSq);
			const FloatV length = FRecip(recipLength);
			const Vec3V locNorm = V3Scale(v, recipLength);
			const FloatV penetration = FSub(length, radius);
			const Vec3V normal = transf1.rotate(locNorm);
			const Vec3V point = transf1.transform(p);

			PX_ASSERT(contactBuffer.count < ContactBuffer::MAX_CONTACTS);
			Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];
			V4StoreA(Vec4V_From_Vec3V(normal), (PxF32*)&contact.normal.x);
			V4StoreA(Vec4V_From_Vec3V(point), (PxF32*)&contact.point.x);
			FStore(penetration, &contact.separation);

			contact.internalFaceIndex0 = PXC_CONTACT_NO_FACE_INDEX;
			contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;

			//context.mContactBuffer.contact(point, normal, penetration);
		}
		return true;
	}
	return false;
	
}

//bool PxcGJKContactSphereBox(const Gu::GeometryUnion& shape0, const Gu::GeometryUnion& shape1,
//					  const PxTransform& transform0, const PxTransform& transform1,
//					  PxReal contactDistance, PxcNpCache& cache, PxcNpThreadContext& context)
//{
//	using namespace Ps::aos;
//	// Get actual shape data
//	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
//	const PxBoxGeometry& shapeBox = shape1.get<const PxBoxGeometry>();
//	/*PX_ASSERT(cache.shape0 == NULL || cache.shape0 == &shapeBox0);
//	PX_ASSERT(cache.shape1 == NULL || cache.shape1 == &shapeBox1);
//	cache.shape0 = (void*)(&shapeBox0);
//	cache.shape1 = (void*)(&shapeBox1);*/
//
//	PX_ASSERT(transform1.q.isSane());
//	PX_ASSERT(transform0.q.isSane());
//
//
//	PsTransformV transf0(transform0);
//	PsTransformV transf1(transform1);
//
//	const Vec3V positionOffset = V3Scale(V3Add(transf0.p, transf1.p), FloatV_From_F32(0.5f));
//	transf0.p = V3Sub(transf0.p, positionOffset);
//	transf1.p = V3Sub(transf1.p, positionOffset);
//
//	const FloatV r = FloatV_From_F32(shapeSphere.radius);
//
//	const Vec3V boxExtents = Vec3V_From_PxVec3(shapeBox.halfExtents);
//	const Mat33V boxBase = QuatGetMat33V(transf1.q);
//
//	const Gu::SphereV	sphere(transf0.p, r);
//	const Gu::BoxV		box(transf1.p, boxExtents, boxBase);
//
//	Vec3V closestA, closestB;
//	Vec3V normal; // from a to b
//	FloatV penDep;
//	Gu::PxGJKStatus status = Gu::GJKPenetration<Gu::SphereV, Gu::BoxV>(sphere, box, closestA, closestB, normal, penDep);
//	if(status == Gu::GJK_CONTACT)
//	{
//		context.mContactBuffer.contact(V3Add(closestA, positionOffset), normal, FNeg(penDep));
//		return true;
//	}
//	return false;
//}

}
