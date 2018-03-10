/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_MATH_H
#define APEX_MATH_H

#include "foundation/PxMat44.h"
#include "PsVecMath.h"
#include "PsMathUtils.h"

namespace physx
{

#define APEX_ALIGN_UP(offset, alignment) (((offset) + (alignment)-1) & ~((alignment)-1))

PX_INLINE bool operator != (const physx::PxMat44& a, const physx::PxMat44& b)
{
	PX_ASSERT((((size_t)&a) & 0xf) == 0); // verify 16 byte alignment
	PX_ASSERT((((size_t)&b) & 0xf) == 0); // verify 16 byte alignment

	using namespace physx::shdfnd::aos;

	PxU32 allEq = 0xffffffff;
	const Vec4V ca1 = V4LoadU(&a.column0.x);
	const Vec4V cb1 = V4LoadU(&b.column0.x);
	allEq &= V4AllEq(ca1, cb1);
	const Vec4V ca2 = V4LoadU(&a.column1.x);
	const Vec4V cb2 = V4LoadU(&b.column1.x);
	allEq &= V4AllEq(ca2, cb2);
	const Vec4V ca3 = V4LoadU(&a.column2.x);
	const Vec4V cb3 = V4LoadU(&b.column2.x);
	allEq &= V4AllEq(ca3, cb3);
	const Vec4V ca4 = V4LoadU(&a.column3.x);
	const Vec4V cb4 = V4LoadU(&b.column3.x);
	allEq &= V4AllEq(ca4, cb4);

	return allEq == 0;
}


/**
 * computes weight * _origin + (1.0f - weight) * _target
 */
PX_INLINE PxMat44 interpolateMatrix(float weight, const PxMat44& _origin, const PxMat44& _target)
{
	// target: normalize, save scale, transform to quat
	PxMat34Legacy target = _target;
	PxVec3 axis0 = target.M.getColumn(0);
	PxVec3 axis1 = target.M.getColumn(1);
	PxVec3 axis2 = target.M.getColumn(2);
	const PxVec4 targetScale(axis0.normalize(), axis1.normalize(), axis2.normalize(), 1.0f);
	target.M.setColumn(0, axis0);
	target.M.setColumn(1, axis1);
	target.M.setColumn(2, axis2);
	const PxQuat targetQ = target.M.toQuat();

	// origin: normalize, save scale, transform to quat
	PxMat34Legacy origin = _origin;
	axis0 = origin.M.getColumn(0);
	axis1 = origin.M.getColumn(1);
	axis2 = origin.M.getColumn(2);
	const PxVec4 originScale(axis0.normalize(), axis1.normalize(), axis2.normalize(), 1.0f);
	origin.M.setColumn(0, axis0);
	origin.M.setColumn(1, axis1);
	origin.M.setColumn(2, axis2);
	const PxQuat originQ = origin.M.toQuat();

	// interpolate
	PxQuat relativeQ = slerp(1.0f - weight, originQ, targetQ);
	PxMat34Legacy relative;
	relative.t = weight * origin.t + (1.0f - weight) * target.t;
	relative.M.fromQuat(relativeQ);

	PxMat44 _relative = relative;
	const PxVec4 scale = weight * originScale + (1.0f - weight) * targetScale;
	_relative.scale(scale);

	return _relative;
}

}

#endif // APEX_MATH_H
