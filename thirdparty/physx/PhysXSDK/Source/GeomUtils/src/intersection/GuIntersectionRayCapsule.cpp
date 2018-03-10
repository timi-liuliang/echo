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

#include "GuIntersectionRayCapsule.h"
#include "GuCapsule.h"
#include "GuVecCapsule.h"
#include "GuIntersectionRaySphere.h"

using namespace physx;

PxU32 Gu::intersectRayCapsule(const PxVec3& origin, const PxVec3& dir, const Gu::Capsule& capsule, PxReal s[2])
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	PxVec3 capsDir;
	capsule.computeDirection(capsDir);

	PxVec3 kW = capsDir;
	const PxReal fWLength = kW.normalize();

	// PT: if the capsule is in fact a sphere, switch back to dedicated sphere code.
	// This is not just an optimization, the rest of the code fails otherwise.
	if(fWLength<=1e-6f)
	{
		const float d0 = (origin - capsule.p0).magnitudeSquared();
		const float d1 = (origin - capsule.p1).magnitudeSquared();
		const float approxLength = (PxMax(d0, d1) + capsule.radius)*2.0f;
		return (PxU32)Gu::intersectRaySphere(origin, dir, approxLength, capsule.p0, capsule.radius, s[0]);
	}

	// generate orthonormal basis
	PxVec3 kU(0.0f);

	if (fWLength > 0.0f)
	{
		PxReal fInvLength;
		if ( PxAbs(kW.x) >= PxAbs(kW.y) )
		{
			// W.x or W.z is the largest magnitude component, swap them
			fInvLength = PxRecipSqrt(kW.x*kW.x + kW.z*kW.z);
			kU.x = -kW.z*fInvLength;
			kU.y = 0.0f;
			kU.z = kW.x*fInvLength;
		}
		else
		{
			// W.y or W.z is the largest magnitude component, swap them
			fInvLength = PxRecipSqrt(kW.y*kW.y + kW.z*kW.z);
			kU.x = 0.0f;
			kU.y = kW.z*fInvLength;
			kU.z = -kW.y*fInvLength;
		}
	}

	PxVec3 kV = kW.cross(kU);
	kV.normalize();	// PT: fixed november, 24, 2004. This is a bug in Magic.

	// compute intersection

	PxVec3 kD(kU.dot(dir), kV.dot(dir), kW.dot(dir));
	const PxReal fDLength = kD.normalize();

	const PxReal fInvDLength = 1.0f/fDLength;
	const PxVec3 kDiff = origin - capsule.p0;
	const PxVec3 kP(kU.dot(kDiff), kV.dot(kDiff), kW.dot(kDiff));
	const PxReal fRadiusSqr = capsule.radius*capsule.radius;

	// Is the velocity parallel to the capsule direction? (or zero)
	if ( PxAbs(kD.z) >= 1.0f - PX_EPS_REAL || fDLength < PX_EPS_REAL )
	{
		const PxReal fAxisDir = dir.dot(capsDir);

		const PxReal fDiscr = fRadiusSqr - kP.x*kP.x - kP.y*kP.y;
		if ( fAxisDir < 0 && fDiscr >= 0.0f )
		{
			// Velocity anti-parallel to the capsule direction
			const PxReal fRoot = PxSqrt(fDiscr);
			s[0] = (kP.z + fRoot)*fInvDLength;
			s[1] = -(fWLength - kP.z + fRoot)*fInvDLength;
			return 2;
		}
		else if ( fAxisDir > 0  && fDiscr >= 0.0f )
		{
			// Velocity parallel to the capsule direction
			const PxReal fRoot = PxSqrt(fDiscr);
			s[0] = -(kP.z + fRoot)*fInvDLength;
			s[1] = (fWLength - kP.z + fRoot)*fInvDLength;
			return 2;
		}
		else
		{
			// sphere heading wrong direction, or no velocity at all
			return 0;
		}   
	}

	// test intersection with infinite cylinder
	PxReal fA = kD.x*kD.x + kD.y*kD.y;
	PxReal fB = kP.x*kD.x + kP.y*kD.y;
	PxReal fC = kP.x*kP.x + kP.y*kP.y - fRadiusSqr;
	PxReal fDiscr = fB*fB - fA*fC;
	if ( fDiscr < 0.0f )
	{
		// line does not intersect infinite cylinder
		return 0;
	}

	int iQuantity = 0;

	if ( fDiscr > 0.0f )
	{
		// line intersects infinite cylinder in two places
		const PxReal fRoot = PxSqrt(fDiscr);
		const PxReal fInv = 1.0f/fA;
		PxReal fT = (-fB - fRoot)*fInv;
		PxReal fTmp = kP.z + fT*kD.z;
		if ( 0.0f <= fTmp && fTmp <= fWLength )
			s[iQuantity++] = fT*fInvDLength;

		fT = (-fB + fRoot)*fInv;
		fTmp = kP.z + fT*kD.z;
		if ( 0.0f <= fTmp && fTmp <= fWLength )
			s[iQuantity++] = fT*fInvDLength;

		if ( iQuantity == 2 )
		{
			// line intersects capsule wall in two places
			return 2;
		}
	}
	else
	{
		// line is tangent to infinite cylinder
		const PxReal fT = -fB/fA;
		const PxReal fTmp = kP.z + fT*kD.z;
		if ( 0.0f <= fTmp && fTmp <= fWLength )
		{
			s[0] = fT*fInvDLength;
			return 1;
		}
	}

	// test intersection with bottom hemisphere
	// fA = 1
	fB += kP.z*kD.z;
	fC += kP.z*kP.z;
	fDiscr = fB*fB - fC;
	if ( fDiscr > 0.0f )
	{
		const PxReal fRoot = PxSqrt(fDiscr);
		PxReal fT = -fB - fRoot;
		PxReal fTmp = kP.z + fT*kD.z;
		if ( fTmp <= 0.0f )
		{
			s[iQuantity++] = fT*fInvDLength;
			if ( iQuantity == 2 )
				return 2;
		}

		fT = -fB + fRoot;
		fTmp = kP.z + fT*kD.z;
		if ( fTmp <= 0.0f )
		{
			s[iQuantity++] = fT*fInvDLength;
			if ( iQuantity == 2 )
				return 2;
		}
	}
	else if ( fDiscr == 0.0f )
	{
		const PxReal fT = -fB;
		const PxReal fTmp = kP.z + fT*kD.z;
		if ( fTmp <= 0.0f )
		{
			s[iQuantity++] = fT*fInvDLength;
			if ( iQuantity == 2 )
				return 2;
		}
	}

	// test intersection with top hemisphere
	// fA = 1
	fB -= kD.z*fWLength;
	fC += fWLength*(fWLength - 2.0f*kP.z);

	fDiscr = fB*fB - fC;
	if ( fDiscr > 0.0f )
	{
		const PxReal fRoot = PxSqrt(fDiscr);
		PxReal fT = -fB - fRoot;
		PxReal fTmp = kP.z + fT*kD.z;
		if ( fTmp >= fWLength )
		{
			s[iQuantity++] = fT*fInvDLength;
			if ( iQuantity == 2 )
				return 2;
		}

		fT = -fB + fRoot;
		fTmp = kP.z + fT*kD.z;
		if ( fTmp >= fWLength )
		{
			s[iQuantity++] = fT*fInvDLength;
			if ( iQuantity == 2 )
				return 2;
		}
	}
	else if ( fDiscr == 0.0f )
	{
		const PxReal fT = -fB;
		const PxReal fTmp = kP.z + fT*kD.z;
		if ( fTmp >= fWLength )
		{
			s[iQuantity++] = fT*fInvDLength;
			if ( iQuantity == 2 )
				return 2;
		}
	}
	return (PxU32)iQuantity;
}
