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

using namespace physx;

#ifndef PX_VMX
//This version is ported 1:1 from novodex
static PX_FORCE_INLINE bool ContactSphereBox(const PxVec3& sphereOrigin, 
							 PxReal sphereRadius,
							 const PxVec3& boxExtents,
//							 const PxcCachedTransforms& boxCacheTransform, 
							 const PxTransform& boxTransform, 
							 PxVec3& point, 
							 PxVec3& normal, 
							 PxReal& separation, 
							 PxReal contactDistance)
{
//	const PxTransform& boxTransform = boxCacheTransform.getShapeToWorld();

	//returns true on contact
	const PxVec3 delta = sphereOrigin - boxTransform.p; // s1.center - s2.center;
	PxVec3 dRot = boxTransform.rotateInv(delta); //transform delta into OBB body coords.

	//check if delta is outside ABB - and clip the vector to the ABB.
	bool outside = false;

	if (dRot.x < -boxExtents.x)
	{ 
		outside = true; 
		dRot.x = -boxExtents.x;
	}
	else if (dRot.x >  boxExtents.x)
	{ 
		outside = true; 
		dRot.x = boxExtents.x;
	}

	if (dRot.y < -boxExtents.y)
	{ 
		outside = true; 
		dRot.y = -boxExtents.y;
	}
	else if (dRot.y >  boxExtents.y)
	{ 
		outside = true; 
		dRot.y = boxExtents.y;
	}

	if (dRot.z < -boxExtents.z)
	{ 
		outside = true; 
		dRot.z =-boxExtents.z;
	}
	else if (dRot.z >  boxExtents.z)
	{ 
		outside = true; 
		dRot.z = boxExtents.z;
	}

	if (outside) //if clipping was done, sphere center is outside of box.
	{
		point = boxTransform.rotate(dRot); //get clipped delta back in world coords.
		normal = delta - point; //what we clipped away.	
		const PxReal lenSquared = normal.magnitudeSquared();
		const PxReal inflatedDist = sphereRadius + contactDistance;
		if (lenSquared > inflatedDist * inflatedDist) 
			return false;	//disjoint

		//normalize to make it into the normal:
		separation = PxRecipSqrt(lenSquared);
		normal *= separation;	
		separation *= lenSquared;
		//any plane that touches the sphere is tangential, so a vector from contact point to sphere center defines normal.
		//we could also use point here, which has same direction.
		//this is either a faceFace or a vertexFace contact depending on whether the box's face or vertex collides, but we did not distinguish. 
		//We'll just use vertex face for now, this info isn't really being used anyway.
		//contact point is point on surface of cube closest to sphere center.
		point += boxTransform.p;
		separation -= sphereRadius;
		return true;
	}
	else
	{
		//center is in box, we definitely have a contact.
		PxVec3 locNorm;	//local coords contact normal

		/*const*/ PxVec3 absdRot;
		absdRot = PxVec3(PxAbs(dRot.x), PxAbs(dRot.y), PxAbs(dRot.z));
		/*const*/ PxVec3 distToSurface = boxExtents - absdRot;	//dist from embedded center to box surface along 3 dimensions.

		//find smallest element of distToSurface
		if (distToSurface.y < distToSurface.x)
		{
			if (distToSurface.y < distToSurface.z)
			{
				//y
				locNorm = PxVec3(0.0f, dRot.y > 0.0f ? 1.0f : -1.0f, 0.0f);
				separation = -distToSurface.y;
			}
			else
			{
				//z
				locNorm = PxVec3(0.0f,0.0f, dRot.z > 0.0f ? 1.0f : -1.0f);
				separation = -distToSurface.z;
			}
		}
		else
		{
			if (distToSurface.x < distToSurface.z)
			{
				//x
				locNorm = PxVec3(dRot.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
				separation = -distToSurface.x;
			}
			else
			{
				//z
				locNorm = PxVec3(0.0f,0.0f, dRot.z > 0.0f ? 1.0f : -1.0f);
				separation = -distToSurface.z;
			}
		}
		//separation so far is just the embedding of the center point; we still have to push out all of the radius.
		point = sphereOrigin;
		normal = boxTransform.rotate(locNorm);
		separation -= sphereRadius;
		return true;
	}
}

#else

#include "PsVecMath.h"
#include "CmMatrix34.h"
using namespace Ps::aos;

//VMX128 version
//////////////////////////////////////////////////////////////////////////////////////////////////////

//This version is ported 1:1 from novodex
static PX_FORCE_INLINE bool ContactSphereBox(const PxVec3& sphereOrigin, 
							 const PxReal& sphereRadius,
							 const PxVec3& boxExtents,
//							 const PxcCachedTransforms&  boxCacheTransform,
							 //const Cm::Matrix34& boxTransform,
							 const PxTransform& boxTransform2, 
							 PxVec3& point, 
							 PxVec3& normal, 
							 PxReal& separation,
							 PxReal contactDistance)
 {
	 BoolV ffff = BFFFF();
	 Vec3V v3zero = V3Zero();
	 Vec3V v3one = V3One();
	 Vec3V v3minusone = V3Neg(v3one);
	 Vec3V v3unitz = V3UnitZ();

//	const Cm::Matrix34& boxTransform = boxCacheTransform.getShapeToWorldMatrix();

	//returns true on contact
/*	PxVec3 delta = sphereOrigin - boxTransform.p; // s1.center - s2.center;
	PxVec3 dRot = boxTransform.rotateInv(delta); //transform delta into OBB body coords.*/

	Cm::Matrix34 boxTransform(boxTransform2);

	Vec3V btBasis0 = V3LoadU(boxTransform.base0);
	Vec3V btBasis1 = V3LoadU(boxTransform.base1);
	Vec3V btBasis2 = V3LoadU(boxTransform.base2);
	Vec3V btBasis3 = V3LoadU(boxTransform.base3);


	Vec3V delta = V3Sub(V3LoadU(sphereOrigin), btBasis3);
	Vec3V dRot = M33TrnspsMulV3(Mat33V(btBasis0, btBasis1, btBasis2), delta);

	Vec3V boxExtents4 = V3LoadU(boxExtents);
	Vec3V negBoxExtents4 = V3Neg(boxExtents4);

	/*
		//check if delta is outside ABB - and clip the vector to the ABB.
	bool outside = false;

	if (dRot.x < -boxExtents.x)
	{ 
		outside = true; 
		dRot.x = -boxExtents.x;
	}
	else if (dRot.x >  boxExtents.x)
	{ 
		outside = true; 
		dRot.x = boxExtents.x;
	}

	if (dRot.y < -boxExtents.y)
	{ 
		outside = true; 
		dRot.y = -boxExtents.y;
	}
	else if (dRot.y >  boxExtents.y)
	{ 
		outside = true; 
		dRot.y = boxExtents.y;
	}

	if (dRot.z < -boxExtents.z)
	{ 
		outside = true; 
		dRot.z =-boxExtents.z;
	}
	else if (dRot.z >  boxExtents.z)
	{ 
		outside = true; 
		dRot.z = boxExtents.z;
	}
	*/

	BoolV minMask = V3IsGrtr(negBoxExtents4, dRot);
	BoolV maxMask = V3IsGrtr(dRot, boxExtents4);

	BoolV changeMask = BOr(minMask, maxMask);

	dRot = V3Clamp(dRot, negBoxExtents4, boxExtents4);

	// 	if (outside) //if clipping was done, sphere center is outside of box.

	if(!BAllEq(changeMask, ffff))
	{

		/*
		point = boxTransform.rotate(dRot); //get clipped delta back in world coords.
		normal = delta - point; //what we clipped away.	
		PxReal lenSquared = normal.magnitudeSquared();
		if (lenSquared > sphereRadius * sphereRadius) 
			return false;	//disjoint
		*/

		Vec3V point4 = M33MulV3(Mat33V(btBasis0, btBasis1, btBasis2), dRot);
		Vec3V normal4 = V3Sub(delta, point4);

		FloatV lenSquared = V3Dot(normal4, normal4);
		FloatV sphereRadius4 = FLoad(sphereRadius);
		FloatV gContactDistance4 = FLoad(contactDistance);
		FloatV inflatedDist4 = FAdd(sphereRadius4, gContactDistance4);
		FloatV inflatedDistSq = FMul(inflatedDist4, inflatedDist4);

		if(FAllGrtr(lenSquared, inflatedDistSq))//should we avoid the branch and just return later?
			return false;

		/*
		//normalize to make it into the normal:
		separation = PxRecipSqrt(lenSquared);
		normal *= separation;	
		separation *= lenSquared;
		//any plane that touches the sphere is tangential, so a vector from contact point to sphere center defines normal.
		//we could also use point here, which has same direction.
		//this is either a faceFace or a vertexFace contact depending on whether the box's face or vertex collides, but we did not distinguish. 
		//We'll just use vertex face for now, this info isn't really being used anyway.
		//contact point is point on surface of cube closest to sphere center.
		point += boxTransform.p;
		separation -= sphereRadius;
		return true;
		*/

		FloatV seperation4  = FSqrt(lenSquared);
		FloatV rcpSeperation4 = FRsqrt(lenSquared);

		normal4 = V3Scale(normal4, rcpSeperation4);

		point4 = V3Add(point4, btBasis3);
		seperation4 = FSub(seperation4, sphereRadius4);

		FStore(seperation4, &separation);
		V3StoreU(normal4, normal);
		V3StoreU(point4, point);

		return true;
	}
	else
	{

		/*
		//center is in box, we definitely have a contact.
		PxVec3 locNorm;	//local coords contact normal

		PxVec3 absdRot;
		absdRot.set(PxAbs(dRot.x), PxAbs(dRot.y), PxAbs(dRot.z));
		PxVec3 distToSurface = boxExtents - absdRot;	//dist from embedded center to box surface along 3 dimensions.
		*/

		Vec3V absDRot = V3Abs(dRot);
		Vec3V distToSurface = V3Sub(boxExtents4, absDRot);

		/*
		//find smallest element of distToSurface
		if (distToSurface.y < distToSurface.x)
		{
			if (distToSurface.y < distToSurface.z)
			{
				//y
				locNorm.set(0.0f, dRot.y > 0.0f ? 1.0f : -1.0f, 0.0f);
				separation = -distToSurface.y;
			}
			else
			{
				//z
				locNorm.set(0.0f,0.0f, dRot.z > 0.0f ? 1.0f : -1.0f);
				separation = -distToSurface.z;
			}
		}
		else
		{
			if (distToSurface.x < distToSurface.z)
			{
				//x
				locNorm.set(dRot.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
				separation = -distToSurface.x;
			}
			else
			{
				//z
				locNorm.set(0.0f,0.0f, dRot.z > 0.0f ? 1.0f : -1.0f);
				separation = -distToSurface.z;
			}
		}
		*/
		// find smallest element and mask

		Vec3V xyzDTS = distToSurface;
		Vec3V yzxDTS = V3PermYZX(distToSurface);//TODO: can permute better
		Vec3V zxyDTS = V3PermZXY(distToSurface);

		BoolV mask1 = V3IsGrtr(yzxDTS, xyzDTS);
		BoolV mask2 = V3IsGrtrOrEq(zxyDTS, xyzDTS);	// the "equal" part is important to make sure that the returned normal will not be 0 if 
																			// two distances are the same (and smaller than the third)


		// The following is necessary to catch the case where all distances are the same (to make sure the returned normal is not 0)
		BoolV maskEqual1 = V3IsEq(xyzDTS, yzxDTS);
		BoolV maskEqual2 = V3IsEq(xyzDTS, zxyDTS);
		BoolV maskAllEqual = BAnd(maskEqual1, maskEqual2);

		/*
		//separation so far is just the embedding of the center point; we still have to push out all of the radius.
		point = sphereOrigin;
		normal = boxTransform.rotate(locNorm);
		separation -= sphereRadius;

		*/
		Vec3V seperation4 = V3Min(xyzDTS, yzxDTS);
		seperation4 = V3Min(seperation4, zxyDTS);

		seperation4= V3Neg(seperation4);

		BoolV normalMask = V3IsGrtr(dRot, v3zero);
		Vec3V normal4 = V3Sel(normalMask, v3one, v3minusone);

		Vec3V normal4z = V3Mul(v3unitz, normal4);  // if all distances to the surface are the same, we return the z normal

		normal4 = V3Sel(BAnd(mask1,mask2), normal4, v3zero);

		normal4 = V3Sel(maskAllEqual, normal4z, normal4);  // use z as normal if all distances to the surface are equal

		point = sphereOrigin;
		
		Vec3V sphereRadius4 = V3Load(sphereRadius);

		normal4 = M33MulV3(Mat33V(btBasis0, btBasis1, btBasis2), normal4);
		seperation4 = V3Sub(seperation4, sphereRadius4);
		V3StoreU(normal4, normal);
		PxVec3 PX_ALIGN(16, pxseparation);
		V3StoreU(seperation4, pxseparation);
		separation=pxseparation.x;

		return true;
	}
/*
	else
	{
		//center is in box, we definitely have a contact.
		PxVec3 locNorm;	//local coords contact normal

		PxVec3 absdRot;
		absdRot.set(PxAbs(dRot.x), PxAbs(dRot.y), PxAbs(dRot.z));
		PxVec3 distToSurface = boxExtents - absdRot;	//dist from embedded center to box surface along 3 dimensions.

		
		//separation so far is just the embedding of the center point; we still have to push out all of the radius.
		point = sphereOrigin;
		normal = boxTransform.rotate(locNorm);
		separation -= sphereRadius;
		return true;
	}*/
}

#endif


//ML: Need to test it, but should has big win in xbox
#if 0
static PX_FORCE_INLINE bool ContactSphereBox(const Ps::aos::Vec3VArg sphereOrigin, 
											 const Ps::aos::FloatVArg sphereRadius,
											 const Ps::aos::Vec3VArg boxExtents, 
											 const Ps::aos::PsTransformV& boxTransform, 
											 Ps::aos::Vec3V& point, 
											 Ps::aos::Vec3V& normal, 
											 Ps::aos::FloatV& separation, 
											 const Ps::aos::FloatV contactDistance)
{
	using namespace Ps::aos;

	const FloatV zero = FZero();
	const Vec3V delta = V3Sub(sphereOrigin, boxTransform.p);
	const Vec3V _dRot = boxTransform.rotateInv(delta);

	//check if delta is outside AABB - and clip the vector to the AABB.
	const Vec3V nBoxExtents = V3Neg(boxExtents);
	const BoolV con = BAnyTrue3(BOr(V3IsGrtr(_dRot, boxExtents), V3IsGrtr(nBoxExtents, _dRot)));
	bool outside = BAllEq(con, BTTTT())==1;
	const Vec3V dRot = V3Clamp(_dRot, nBoxExtents, boxExtents);


	if (outside) //if clipping was done, sphere center is outside of box.
	{
		const Vec3V p = boxTransform.rotate(dRot); //get clipped delta back in world coords.
		const Vec3V n = V3Sub(delta, p); //what we clipped away.	
		const FloatV inflatedDist = FAdd(sphereRadius, contactDistance);
		const FloatV length = V3Length(n);
		const FloatV recipLength = FRecip(length);

		//any plane that touches the sphere is tangential, so a vector from contact point to sphere center defines normal.
		//we could also use point here, which has same direction.
		//this is either a faceFace or a vertexFace contact depending on whether the box's face or vertex collides, but we did not distinguish. 
		//We'll just use vertex face for now, this info isn't really being used anyway.
		//contact point is point on surface of cube closest to sphere center.
		const BoolV con = FIsGrtr(length, inflatedDist);
		normal = V3Sel(con, n, V3Mul(n, recipLength));
		point = V3Sel(con, p, V3Add(p, boxTransform.p));
		separation = FSub(length , sphereRadius);
		return (BAllEq(con, BFFFF()) == 1);
	}
	else
	{
		const Vec3V absdRot = V3Abs(dRot);
		const Vec3V distToSurface = V3Sub(boxExtents, absdRot);//dist from embedded center to box surface along 3 dimensions.

		const FloatV x = V3GetX(distToSurface);
		const FloatV y = V3GetY(distToSurface);
		const FloatV z = V3GetZ(distToSurface);

		//find smallest element of distToSurface
		const BoolV con0 = BAllTrue3(V3IsGrtrOrEq(distToSurface, z));
		const BoolV con1 = BAllTrue3(V3IsGrtrOrEq(distToSurface, x));
		const Vec3V tmpX = V3Sel(FIsGrtr(V3GetX(dRot), zero), V3UnitX(), V3Neg(V3UnitX()));
		const Vec3V tmpY = V3Sel(FIsGrtr(V3GetY(dRot), zero), V3UnitY(), V3Neg(V3UnitY()));
		const Vec3V tmpZ = V3Sel(FIsGrtr(V3GetZ(dRot), zero), V3UnitZ(), V3Neg(V3UnitZ()));

		const Vec3V locNorm= V3Sel(con0, tmpZ, V3Sel(con1, tmpX, tmpY));////local coords contact normal
		const FloatV dist = FSel(con0, FNeg(z), FSel(con1, FNeg(x), FNeg(y)));

		//separation so far is just the embedding of the center point; we still have to push out all of the radius.
		point = sphereOrigin;
		normal = boxTransform.rotate(locNorm);
		separation = FSub(dist, sphereRadius);
		return true;
	}
}
#endif

// Sphere-box contact generation
namespace physx
{
bool PxcContactSphereBox(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);

	// Get actual shape data
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
	const PxBoxGeometry& shapeBox = shape1.get<const PxBoxGeometry>();

	PxVec3 normal;
	PxVec3 point;
	PxReal separation;

	const PxTransform& shapeToWorld0 = transform0;
	//#ifndef PX_VMX
	const PxTransform& boxTransform = transform1;
	/*#else
	Cm::Matrix34 boxTransform(transform1);
	#endif*/

	if(ContactSphereBox(shapeToWorld0.p, shapeSphere.radius, shapeBox.halfExtents, boxTransform, point, normal, separation, contactDistance)) 
	{
		contactBuffer.contact(point, normal, separation);
		return true;
	}
	return false;
}
}


//// This is a somewhat revised version of Novodex contactSphereBox() routine. 
//// This one works in box space. But I am not too sure about this box space thingy. 
//// I am generating a contact in box space and transforming it.
//// This may cause the ray from the contact point along the contact normal to miss the sphere origin.
//static bool ContactSphereAABB(const PxVec3& sphereOrigin, PxReal sphereRadius, const PxVec3& aabbExtent, PxVec3& point, PxVec3& normal, PxReal& separation)
//{
//	//TODO: skip dRot, I don't see a need for it
//
//	//returns true on contact
//	PxVec3 dRot = sphereOrigin;	
//
//	//check if delta is outside ABB - and clip the vector to the ABB.
//	bool outside = false;
//	int ax;
//	for (ax = 0; ax < 3; ax++)
//	{
//		if (dRot[ax] < -aabbExtent[ax])
//		{ 
//			outside = true; 
//			dRot[ax] = -aabbExtent[ax];
//		}
//		else if (dRot[ax] > aabbExtent[ax])
//		{ 
//			outside = true; 
//			dRot[ax] = aabbExtent[ax];
//		}
//	}
//
//	if (outside)	//if clipping was done, sphere center is outside of box.
//	{
//		point = dRot;
//		normal = sphereOrigin - point;	//what we clipped away.	
//		PxReal lenSquared = normal.magnitudeSquared();
//		if (lenSquared > sphereRadius * sphereRadius) 
//			return false;	//disjoint
//
//		//normalize to make it into the normal:
//		separation = PxRecipSqrt(lenSquared);
//		normal *= separation;	
//		separation *= lenSquared;
//		//any plane that touches the sphere is tangential, so a vector from contact point to sphere center defines normal.
//		//we could also use point here, which has same direction.
//		//this is either a faceFace or a vertexFace contact depending on whether the box's face or vertex collides, but we did not distinguish. 
//		//We'll just use vertex face for now, this info isn't really being used anyway.
//		//contact point is point on surface of cube closest to sphere center.
//		separation -= sphereRadius;
//		return true;
//	}
//	else
//	{
//		//center is in box, we definitely have a contact.
//		PxVec3 locNorm;	//local coords contact normal
//
//		const PxVec3 absdRot(PxAbs(dRot.x), PxAbs(dRot.y), PxAbs(dRot.z));
//		const PxVec3 distToSurface = aabbExtent - absdRot;	//dist from embedded center to box surface along 3 dimensions.
//
//		//find smallest element of distToSurface
//		if (distToSurface.y < distToSurface.x)
//		{
//			if (distToSurface.y < distToSurface.z)
//			{
//				//y
//				locNorm.set(0.0f, dRot.y > 0.0f ? 1.0f : -1.0f, 0.0f);
//				separation = -distToSurface.y;
//			}
//			else
//			{
//				//z
//				locNorm.set(0.0f,0.0f, dRot.z > 0.0f ? 1.0f : -1.0f);
//				separation = -distToSurface.z;
//			}
//		}
//		else
//		{
//			if (distToSurface.x < distToSurface.z)
//			{
//				//x
//				locNorm.set(dRot.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
//				separation = -distToSurface.x;
//			}
//			else
//			{
//				//z
//				locNorm.set(0.0f,0.0f, dRot.z > 0.0f ? 1.0f : -1.0f);
//				separation = -distToSurface.z;
//			}
//		}
//		//separation so far is just the embedding of the center point; we still have to push out all of the radius.
//		point = sphereOrigin;
//		normal = locNorm;
//		separation -= sphereRadius;
//		return true;
//	}
//}
