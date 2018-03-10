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


#include "PsMathUtils.h"
#include "PsUtilities.h"
#include "foundation/PxMat33.h"

using namespace physx;
using namespace physx::shdfnd;
using namespace physx::intrinsics;

PxQuat physx::shdfnd::computeQuatFromNormal(const PxVec3& n)
{
	//parallel or anti-parallel
	if(n.x > 0.9999f)
	{
		//parallel
		return PxQuat(PxIdentity);
	}
	else if (n.x < -0.9999f)
	{
		//anti-parallel
		//contactQuaternion.fromAngleAxisFast(PXD_PI, Vector3(0.0f, 1.0f, 0.0f));
		return PxQuat(0.0f, 1.0f, 0.0f, 0.0f);
	}
	else
	{
		PxVec3 rotVec(0.0f, -n.z, n.y);

		//Convert to quat
		PxReal angle = rotVec.magnitude();
		rotVec *= 1.0f/angle;
//		if(angle > 1.0f) angle = 1.0f;
		angle = selectMin(angle, 1.0f);

		// djs: injudiciously imbecilic use of trig functions, good thing Adam is going to trample this path like a
		// frustrated rhinoceros in mating season

		angle = PxAsin(angle);

//		if(n.x < 0)
//			angle = PxPi - angle;
		angle = fsel(n.x, angle, PxPi - angle);

		return PxQuat(angle, rotVec);
	}
}

/**
\brief computes a oriented bounding box around the scaled basis.
\param basis Input = skewed basis, Output = (normalized) orthogonal basis.
\return Bounding box extent.
*/
PxVec3 physx::shdfnd::optimizeBoundingBox(PxMat33& basis)
{
	PxVec3* PX_RESTRICT vec = &basis[0];	// PT: don't copy vectors if not needed...

	// PT: since we store the magnitudes to memory, we can avoid the FCMPs afterwards
	PxVec3 magnitude( vec[0].magnitudeSquared(), vec[1].magnitudeSquared(), vec[2].magnitudeSquared() );

	// find indices sorted by magnitude
#ifdef PX_X360
	unsigned int i = (PxU32&)(magnitude[1]) > (PxU32&)(magnitude[ 0 ]) ? 1 :  0u;
	unsigned int j = (PxU32&)(magnitude[2]) > (PxU32&)(magnitude[1-i]) ? 2 : 1-i;
#else
	unsigned int i = magnitude[1] > magnitude[0] ? 1 :  0u;
	unsigned int j = magnitude[2] > magnitude[1-i] ? 2 : 1-i;
#endif
	const unsigned int k = 3 - i - j;
#ifdef PX_X360
	if((PxU32&)(magnitude[i]) < (PxU32&)(magnitude[j]))
#else
	if(magnitude[i] < magnitude[j])
#endif
		swap(i, j);

	// ortho-normalize basis

	PxReal invSqrt = PxRecipSqrt(magnitude[i]); 
	magnitude[i] *= invSqrt; vec[i] *= invSqrt; // normalize the first axis
	PxReal dotij = vec[i].dot(vec[j]);
	PxReal dotik = vec[i].dot(vec[k]);
	magnitude[i] += PxAbs(dotij) + PxAbs(dotik); // elongate the axis by projection of the other two
	vec[j] -= vec[i] * dotij; // orthogonize the two remaining axii relative to vec[i]
	vec[k] -= vec[i] * dotik;

	magnitude[j] = vec[j].normalize();
	PxReal dotjk = vec[j].dot(vec[k]);
	magnitude[j] += PxAbs(dotjk); // elongate the axis by projection of the other one
	vec[k] -= vec[j] * dotjk; // orthogonize vec[k] relative to vec[j]

	magnitude[k] = vec[k].normalize();

	return magnitude;
}

PxQuat physx::shdfnd::slerp(const PxReal t, const PxQuat& left, const PxQuat& right) 
{
	const PxReal quatEpsilon = (PxReal(1.0e-8f));

	PxReal cosine = left.dot(right);
	PxReal sign = PxReal(1);
	if (cosine < 0)
	{
		cosine = -cosine;
		sign = PxReal(-1);
	}

	PxReal sine = PxReal(1) - cosine*cosine;

	if(sine>=quatEpsilon*quatEpsilon)	
	{
		sine = PxSqrt(sine);
		const PxReal angle = PxAtan2(sine, cosine);
		const PxReal i_sin_angle = PxReal(1) / sine;

		const PxReal leftw = PxSin(angle*(PxReal(1)-t)) * i_sin_angle;
		const PxReal rightw = PxSin(angle * t) * i_sin_angle * sign;

		return left * leftw + right * rightw;
	}

	return left;
}

void physx::shdfnd::integrateTransform(const PxTransform& curTrans, const PxVec3& linvel, const PxVec3& angvel, PxReal timeStep, PxTransform& result)
{
	result.p = curTrans.p + linvel * timeStep;

	//from void PxsDynamicsContext::integrateAtomPose(PxsRigidBody* atom, Cm::BitMap &shapeChangedMap) const:
	// Integrate the rotation using closed form quaternion integrator
	PxReal w = angvel.magnitudeSquared();

	if(w != 0.0f)
	{
		w = PxSqrt(w);
		if (w != 0.0f)
		{
			const PxReal v = timeStep * w * 0.5f;
			const PxReal q = PxCos(v);
			const PxReal s = PxSin(v) / w;

			const PxVec3 pqr = angvel * s;
			const PxQuat quatVel(pqr.x, pqr.y, pqr.z, 0);
			PxQuat out;		//need to have temporary, otherwise we may overwrite input if &curTrans == &result.
			out = quatVel * curTrans.q;
			out.x += curTrans.q.x * q;
			out.y += curTrans.q.y * q;
			out.z += curTrans.q.z * q;
			out.w += curTrans.q.w * q;
			result.q = out;
			return;
		}
	}
	//orientation stays the same - convert from quat to matrix:
	result.q = curTrans.q;
}
