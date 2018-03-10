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

#ifndef GU_BARYCENTRIC_COORDINATES_H
#define GU_BARYCENTRIC_COORDINATES_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "PsVecMath.h"

namespace physx
{
namespace Gu
{
	//calculate the barycentric coorinates for a point in a segment
	void barycentricCoordinates(const Ps::aos::Vec3VArg p, 
		const Ps::aos::Vec3VArg a, 
		const Ps::aos::Vec3VArg b, 
		Ps::aos::FloatV& v);

	//calculate the barycentric coorinates for a point in a triangle
	void barycentricCoordinates(const Ps::aos::Vec3VArg p, 
		const Ps::aos::Vec3VArg a, 
		const Ps::aos::Vec3VArg b, 
		const Ps::aos::Vec3VArg c, 
		Ps::aos::FloatV& v, 
		Ps::aos::FloatV& w);

	void barycentricCoordinates(const Ps::aos::Vec3VArg v0, 
		const Ps::aos::Vec3VArg v1, 
		const Ps::aos::Vec3VArg v2,  
		Ps::aos::FloatV& v, 
		Ps::aos::FloatV& w);

	PX_INLINE Ps::aos::BoolV isValidTriangleBarycentricCoord(const Ps::aos::FloatVArg v, const Ps::aos::FloatVArg w)
	{
		using namespace Ps::aos;
		const FloatV zero = FZero();
		const FloatV one = FOne();

		const BoolV con0 = BAnd(FIsGrtrOrEq(v, zero), FIsGrtrOrEq(one, v));
		const BoolV con1 = BAnd(FIsGrtrOrEq(w, zero), FIsGrtrOrEq(one, w));
		const BoolV con2 = FIsGrtr(one, FAdd(v, w));
		return BAnd(con0, BAnd(con1, con2));
	}

	PX_INLINE Ps::aos::BoolV isValidTriangleBarycentricCoord2(const Ps::aos::Vec4VArg vwvw)
	{
		using namespace Ps::aos;
		const Vec4V zero = V4Zero();
		const Vec4V one = V4One();

		const Vec4V v0v1v0v1 = V4Perm_XZXZ(vwvw);
		const Vec4V w0w1w0w1 = V4Perm_YWYW(vwvw);

		const BoolV con0 = BAnd(V4IsGrtrOrEq(v0v1v0v1, zero), V4IsGrtrOrEq(one, v0v1v0v1));
		const BoolV con1 = BAnd(V4IsGrtrOrEq(w0w1w0w1, zero), V4IsGrtrOrEq(one, w0w1w0w1));
		const BoolV con2 = V4IsGrtr(one, V4Add(v0v1v0v1, w0w1w0w1));
		return BAnd(con0, BAnd(con1, con2));
	}

} // namespace Gu

}

#endif
