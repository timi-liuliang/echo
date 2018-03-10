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


#ifndef PX_PHYSICS_TRANSFORM_H
#define PX_PHYSICS_TRANSFORM_H

#include "PsVecMath.h"
#include "foundation/PxTransform.h"

namespace physx
{
namespace shdfnd
{
namespace aos
{

class PsTransformV
{
public:
	QuatV q;
	Vec3V p;

	PX_FORCE_INLINE PsTransformV(const PxTransform& orientation)
	{
		//const PxQuat oq = orientation.q;
		//const PxF32 f[4] = {oq.x, oq.y, oq.z, oq.w};
		q = QuatVLoadXYZW(orientation.q.x, orientation.q.y, orientation.q.z, orientation.q.w);
		//q = QuatV_From_F32Array(&oq.x);
		p = V3LoadU(orientation.p);
	}


	PX_FORCE_INLINE PsTransformV(const Vec3VArg p0 = V3Zero(), const QuatVArg q0 = QuatIdentity()): q(q0), p(p0) 
	{
		PX_ASSERT(isSaneQuatV(q0));
	}

	PX_FORCE_INLINE PsTransformV operator*(const PsTransformV& x) const
	{
		PX_ASSERT(x.isSane());
		return transform(x);
	}

	PX_FORCE_INLINE PsTransformV getInverse() const
	{
		PX_ASSERT(isFinite());
		//return PxTransform(q.rotateInv(-p),q.getConjugate());
		return PsTransformV(QuatRotateInv(q, V3Neg(p)),QuatConjugate(q));
	}

	PX_FORCE_INLINE void normalize()
	{
		p = V3Zero();
		q = QuatIdentity();
	}

	PX_FORCE_INLINE void Invalidate()
	{
		p = V3Splat(FMax());
		q = QuatIdentity();  
	}

	PX_FORCE_INLINE Vec3V transform(const Vec3VArg input) const
	{
		PX_ASSERT(isFinite());
		//return q.rotate(input) + p;
		return QuatTransform(q, p, input);
	}

	PX_FORCE_INLINE Vec3V transformInv(const Vec3VArg input) const
	{
		PX_ASSERT(isFinite());
		//return q.rotateInv(input-p);
		return QuatRotateInv(q, V3Sub(input, p));
	}

	PX_FORCE_INLINE Vec3V rotate(const Vec3VArg input) const
	{
		PX_ASSERT(isFinite());
		//return q.rotate(input);
		return QuatRotate(q, input);
	}

	PX_FORCE_INLINE Vec3V rotateInv(const Vec3VArg input) const
	{
		PX_ASSERT(isFinite());
		//return q.rotateInv(input);
		return QuatRotateInv(q, input);
	}

	//! Transform transform to parent (returns compound transform: first src, then *this)
	PX_FORCE_INLINE PsTransformV transform(const PsTransformV& src) const
	{
		PX_ASSERT(src.isSane());
		PX_ASSERT(isSane());
		// src = [srct, srcr] -> [r*srct + t, r*srcr]
		//return PxTransform(q.rotate(src.p) + p, q*src.q);
		return PsTransformV(V3Add(QuatRotate(q, src.p), p), QuatMul(q, src.q));
	}

	/**
	\brief returns true if finite and q is a unit quaternion
	*/

	PX_FORCE_INLINE bool isValid() const
	{
		//return p.isFinite() && q.isFinite() && q.isValid();
		return isFiniteVec3V(p) & isFiniteQuatV(q) & isValidQuatV(q);
	}
       
	/**
	\brief returns true if finite and quat magnitude is reasonably close to unit to allow for some accumulation of error vs isValid
	*/

	PX_FORCE_INLINE bool isSane() const
	{
		//return isFinite() && q.isSane();
		return isFinite() & isSaneQuatV(q);
	}


	/**
	\brief returns true if all elems are finite (not NAN or INF, etc.)
	*/
	PX_FORCE_INLINE bool isFinite() const 
	{ 
		//return p.isFinite() && q.isFinite();
		return isFiniteVec3V(p) & isFiniteQuatV(q);
	}

	//! Transform transform from parent (returns compound transform: first src, then this->inverse)
	PX_FORCE_INLINE PsTransformV transformInv(const PsTransformV& src) const
	{
		PX_ASSERT(src.isSane());
		PX_ASSERT(isFinite());
		// src = [srct, srcr] -> [r^-1*(srct-t), r^-1*srcr]
		/*PxQuat qinv = q.getConjugate();
		return PxTransform(qinv.rotate(src.p - p), qinv*src.q);*/
		const QuatV qinv = QuatConjugate(q);
		const Vec3V v = QuatRotate(qinv, V3Sub(src.p, p));
		const QuatV rot = QuatMul(qinv, src.q);
		return PsTransformV(v, rot);
	}

	static PX_FORCE_INLINE PsTransformV createIdentity() 
	{ 
		return PsTransformV(V3Zero()); 
	}
};

PX_FORCE_INLINE PsTransformV loadTransformA(const PxTransform& transform)
{
	const QuatV q0 = QuatVLoadA(&transform.q.x);
	const Vec3V p0 = V3LoadA(&transform.p.x);

	return PsTransformV(p0, q0);
}

PX_FORCE_INLINE PsTransformV loadTransformU(const PxTransform& transform)
{
	const QuatV q0 = QuatVLoadU(&transform.q.x);
	const Vec3V p0 = V3LoadU(&transform.p.x);

	return PsTransformV(p0, q0);
}


class PsMatTransformV
{
public:
	Mat33V rot;
	Vec3V p;


	PX_FORCE_INLINE PsMatTransformV()
	{
		p = V3Zero();
		rot = M33Identity();
	}
	PX_FORCE_INLINE PsMatTransformV(const Vec3VArg _p, const Mat33V& _rot)
	{
		p = _p;
		rot = _rot;
	}

	PX_FORCE_INLINE PsMatTransformV(const PsTransformV& other)
	{
		p = other.p;
		rot = QuatGetMat33V(other.q);
	}

	PX_FORCE_INLINE PsMatTransformV(const Vec3VArg _p, const QuatV& quat)
	{
		p = _p;
		rot = QuatGetMat33V(quat);
	}

	PX_FORCE_INLINE Vec3V getCol0()const
	{
		return rot.col0;
	}

	PX_FORCE_INLINE Vec3V getCol1()const
	{
		return rot.col1;
	}

	PX_FORCE_INLINE Vec3V getCol2()const
	{
		return rot.col2;
	}

	PX_FORCE_INLINE void setCol0(const Vec3VArg col0)
	{
		rot.col0 = col0;
	}

	PX_FORCE_INLINE void setCol1(const Vec3VArg col1)
	{
		rot.col1 = col1;
	}

	PX_FORCE_INLINE void setCol2(const Vec3VArg col2)
	{
		rot.col2 = col2;
	}

	

	PX_FORCE_INLINE Vec3V transform(const Vec3VArg input) const
	{
		return V3Add(p, M33MulV3(rot, input));
	}

	PX_FORCE_INLINE Vec3V transformInv(const Vec3VArg input) const
	{
		return M33TrnspsMulV3(rot, V3Sub(input, p));//QuatRotateInv(q, V3Sub(input, p));
	}

	PX_FORCE_INLINE Vec3V rotate(const Vec3VArg input) const
	{
		return M33MulV3(rot, input);
	}

	PX_FORCE_INLINE Vec3V rotateInv(const Vec3VArg input) const
	{
		return M33TrnspsMulV3(rot, input);
	}

	PX_FORCE_INLINE PsMatTransformV transformInv(const PsMatTransformV& src) const
	{

		const Vec3V v = M33TrnspsMulV3(rot, V3Sub(src.p, p));
		const Mat33V mat = M33MulM33(M33Trnsps(rot), src.rot);
		return PsMatTransformV(v, mat);
	}


};
}
}
}

#endif
