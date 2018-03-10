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

#include "foundation/PxPreprocessor.h"
#include <assert.h>
#include <stdio.h>

#include "CmPhysXCommon.h"
#include "PxTransform.h"
#include "PxMat33.h"
#include "PsMathUtils.h"
#include "PxVec4.h"

namespace physx
{

PX_FOUNDATION_API PxQuat PxShortestRotation(const PxVec3& v0, const PxVec3& v1)
{
	const PxReal d = v0.dot(v1);
	const PxVec3 cross = v0.cross(v1);

	PxQuat q = d>-1 ? PxQuat(cross.x, cross.y, cross.z, 1+d) 
					: PxAbs(v0.x)<0.1f ? PxQuat(0.0f, v0.z, -v0.y, 0.0f) : PxQuat(v0.y, -v0.x, 0.0f, 0.0f);

	return q.getNormalized();
}


PX_FOUNDATION_API PxTransform PxTransformFromPlaneEquation(const PxPlane& plane)
{
	PxPlane p = plane; 
	p.normalize();

	// special case handling for axis aligned planes
	const PxReal halfsqrt2 = 0.707106781;
	PxQuat q;
	if(2 == (p.n.x == 0.0f) + (p.n.y == 0.0f) + (p.n.z == 0.0f)) // special handling for axis aligned planes
	{
		if(p.n.x > 0)		q = PxQuat(PxIdentity);
		else if(p.n.x < 0)	q = PxQuat(0, 0, 1.0f, 0);
		else				q = PxQuat(0.0f, -p.n.z, p.n.y, 1.0f) * halfsqrt2;
	}
	else q = PxShortestRotation(PxVec3(1.f,0,0), p.n);

	return PxTransform(-p.n * p.d, q);

}

PX_FOUNDATION_API PxTransform PxTransformFromSegment(const PxVec3& p0, const PxVec3& p1, PxReal* halfHeight)
{
	const PxVec3 axis = p1-p0;
	const PxReal height = axis.magnitude();
	if(halfHeight)
		*halfHeight = height/2;

	return PxTransform((p1+p0) * 0.5f, 
						height<1e-6f ? PxQuat(PxIdentity) : PxShortestRotation(PxVec3(1.f,0,0), axis/height));		
}



namespace
{
	// indexed rotation around axis, with sine and cosine of half-angle
	PxQuat indexedRotation(PxU32 axis, PxReal s, PxReal c)
	{
		PxVec4 q(0,0,0,c);
		q[axis] = s;
		return reinterpret_cast<PxQuat&>(q);
	}
}

PX_FOUNDATION_API PxVec3 PxDiagonalize(const PxMat33& m, PxQuat &massFrame)
{
	// jacobi rotation using quaternions (from an idea of Stan Melax, with fix for precision issues)

	const PxU32 MAX_ITERS = 24;

	PxQuat q = PxQuat(PxIdentity);

	PxMat33 d;
	for(PxU32 i=0; i < MAX_ITERS;i++)
	{
		PxMat33 axes(q);
		d = axes.getTranspose() * m * axes;

		PxReal d0 = PxAbs(d[1][2]), d1 = PxAbs(d[0][2]), d2 = PxAbs(d[0][1]);
		PxU32 a = PxU32(d0 > d1 && d0 > d2 ? 0 : d1 > d2 ? 1 : 2);						// rotation axis index, from largest off-diagonal element

		PxU32 a1 = Ps::getNextIndex3(a), a2 = Ps::getNextIndex3(a1);											
		if(d[a1][a2] == 0.0f || PxAbs(d[a1][a1]-d[a2][a2]) > 2e6*PxAbs(2.0f*d[a1][a2]))
			break;

		PxReal w = (d[a1][a1]-d[a2][a2]) / (2.0f*d[a1][a2]);					// cot(2 * phi), where phi is the rotation angle
		PxReal absw = PxAbs(w);

		PxQuat r;
		if(absw>1000)
			r = indexedRotation(a, 1/(4*w), 1.f);									// h will be very close to 1, so use small angle approx instead
		else
		{
  			PxReal t = 1 / (absw + PxSqrt(w*w+1));								// absolute value of tan phi
			PxReal h = 1 / PxSqrt(t*t+1);										// absolute value of cos phi

			PX_ASSERT(h!=1);													// |w|<1000 guarantees this with typical IEEE754 machine eps (approx 6e-8)
			r = indexedRotation(a, PxSqrt((1-h)/2) * PxSign(w), PxSqrt((1+h)/2));
		}
	
		q = (q*r).getNormalized();
	}

	massFrame = q;
	return PxVec3(d.column0.x, d.column1.y, d.column2.z);
}

}
