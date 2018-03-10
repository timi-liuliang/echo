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


#include "PxPreprocessor.h"
#include "PsVecMath.h"

#include "CmPhysXCommon.h"
#include "PxcSolverBody.h"
#include "PxcSolverContactPF4.h"
#include "PxcSolverConstraint1D.h"
#include "PxcSolverConstraintDesc.h"
#include "PxcThresholdStreamElement.h"
#include "PxsSolverContext.h"
#include "PsUtilities.h"
#include "PxvShader.h"
#include "PsAtomic.h"
#include "PxcThresholdStreamElement.h"
#include "PxsSolverCoreGeneral.h"
#include "PxcSolverContact.h"

namespace physx
{

static void solveContactCoulomb4_Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& /*cache*/)
{
	PxcSolverBody& b00 = *desc[0].bodyA;
	PxcSolverBody& b01 = *desc[0].bodyB;
	PxcSolverBody& b10 = *desc[1].bodyA;
	PxcSolverBody& b11 = *desc[1].bodyB;
	PxcSolverBody& b20 = *desc[2].bodyA;
	PxcSolverBody& b21 = *desc[2].bodyB;
	PxcSolverBody& b30 = *desc[3].bodyA;
	PxcSolverBody& b31 = *desc[3].bodyB;

	//We'll need this.
	const Vec4V vZero	= V4Zero();
		
	Vec4V linVel00 = V4LoadA(&b00.linearVelocity.x);
	Vec4V linVel01 = V4LoadA(&b01.linearVelocity.x);
	Vec4V angVel00 = V4LoadA(&b00.angularVelocity.x);
	Vec4V angVel01 = V4LoadA(&b01.angularVelocity.x);

	Vec4V linVel10 = V4LoadA(&b10.linearVelocity.x);
	Vec4V linVel11 = V4LoadA(&b11.linearVelocity.x);
	Vec4V angVel10 = V4LoadA(&b10.angularVelocity.x);
	Vec4V angVel11 = V4LoadA(&b11.angularVelocity.x);

	Vec4V linVel20 = V4LoadA(&b20.linearVelocity.x);
	Vec4V linVel21 = V4LoadA(&b21.linearVelocity.x);
	Vec4V angVel20 = V4LoadA(&b20.angularVelocity.x);
	Vec4V angVel21 = V4LoadA(&b21.angularVelocity.x);

	Vec4V linVel30 = V4LoadA(&b30.linearVelocity.x);
	Vec4V linVel31 = V4LoadA(&b31.linearVelocity.x);
	Vec4V angVel30 = V4LoadA(&b30.angularVelocity.x);
	Vec4V angVel31 = V4LoadA(&b31.angularVelocity.x);


	Vec4V linVel0T0, linVel0T1, linVel0T2;
	Vec4V linVel1T0, linVel1T1, linVel1T2;
	Vec4V angVel0T0, angVel0T1, angVel0T2;
	Vec4V angVel1T0, angVel1T1, angVel1T2;


	PX_TRANSPOSE_44_34(linVel00, linVel10, linVel20, linVel30, linVel0T0, linVel0T1, linVel0T2);
	PX_TRANSPOSE_44_34(linVel01, linVel11, linVel21, linVel31, linVel1T0, linVel1T1, linVel1T2);
	PX_TRANSPOSE_44_34(angVel00, angVel10, angVel20, angVel30, angVel0T0, angVel0T1, angVel0T2);
	PX_TRANSPOSE_44_34(angVel01, angVel11, angVel21, angVel31, angVel1T0, angVel1T1, angVel1T2);


	

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc[0].constraint;

	PxcSolverContactCoulombHeader4* PX_RESTRICT firstHeader = (PxcSolverContactCoulombHeader4*)currPtr;

	const PxU8* PX_RESTRICT last = desc[0].constraint + firstHeader->frictionOffset;

	//const PxU8* PX_RESTRICT endPtr = desc[0].constraint + getConstraintLength(desc[0]);


	//TODO - can I avoid this many tests???
	while(currPtr < last)
	{

		PxcSolverContactCoulombHeader4* PX_RESTRICT hdr = (PxcSolverContactCoulombHeader4*)currPtr;

		Vec4V* appliedForceBuffer = (Vec4V*)(currPtr + hdr->frictionOffset + sizeof(PxcSolverFrictionHeader4));

		//PX_ASSERT((PxU8*)appliedForceBuffer < endPtr);

		currPtr = (PxU8*)(hdr + 1);

		const PxU32 numNormalConstr = hdr->numNormalConstr;

		PxcSolverContact4Dynamic* PX_RESTRICT contacts = (PxcSolverContact4Dynamic*)currPtr;
		//const Vec4V dominance1 = V4Neg(__dominance1);

		currPtr = (PxU8*)(contacts + numNormalConstr);

		const Vec4V invMass0D0 = hdr->invMassADom;
		const Vec4V invMass1D1 = hdr->invMassBDom;

		const Vec4V normalT0 = hdr->normalX;
		const Vec4V normalT1 = hdr->normalY;
		const Vec4V normalT2 = hdr->normalZ;

		const Vec4V __normalVel1 = V4Mul(linVel0T0, normalT0);
		const Vec4V __normalVel3 = V4Mul(linVel1T0, normalT0);
		const Vec4V _normalVel1 = V4MulAdd(linVel0T1, normalT1, __normalVel1);
		const Vec4V _normalVel3 = V4MulAdd(linVel1T1, normalT1, __normalVel3);

		const Vec4V delLinVel00 = V4Mul(normalT0, invMass0D0);
		const Vec4V delLinVel01 = V4Mul(normalT0, invMass1D1);

		const Vec4V delLinVel10 = V4Mul(normalT1, invMass0D0);
		const Vec4V delLinVel11 = V4Mul(normalT1, invMass1D1);

		const Vec4V delLinVel20 = V4Mul(normalT2, invMass0D0);
		const Vec4V delLinVel21 = V4Mul(normalT2, invMass1D1);

		Vec4V normalVel1 = V4MulAdd(linVel0T2, normalT2, _normalVel1);
		Vec4V normalVel3 = V4MulAdd(linVel1T2, normalT2, _normalVel3);

		Vec4V accumDeltaF = vZero;

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContact4Dynamic& c = contacts[i];
			Ps::prefetchLine((&contacts[i+1]));
			Ps::prefetchLine((&contacts[i+1]), 128);
			Ps::prefetchLine((&contacts[i+1]), 256);
			Ps::prefetchLine((&contacts[i+1]), 384);

			const Vec4V delAngVel0T0 = c.delAngVel0X;
			const Vec4V delAngVel0T1 = c.delAngVel0Y;
			const Vec4V delAngVel0T2 = c.delAngVel0Z;
			const Vec4V delAngVel1T0 = c.delAngVel1X;
			const Vec4V delAngVel1T1 = c.delAngVel1Y;
			const Vec4V delAngVel1T2 = c.delAngVel1Z;

			const Vec4V appliedForce = c.appliedForce;
			const Vec4V velMultiplier = c.velMultiplier;
			
			const Vec4V targetVel = c.targetVelocity;
			const Vec4V scaledBias = c.scaledBias;
			const Vec4V maxImpulse = c.maxImpulse;

			const Vec4V raXnT0 = c.raXnX;
			const Vec4V raXnT1 = c.raXnY;
			const Vec4V raXnT2 = c.raXnZ;
			const Vec4V rbXnT0 = c.rbXnX;
			const Vec4V rbXnT1 = c.rbXnY;
			const Vec4V rbXnT2 = c.rbXnZ;

			
			const Vec4V __normalVel2 = V4Mul(raXnT0, angVel0T0);
			const Vec4V __normalVel4 = V4Mul(rbXnT0, angVel1T0);

			
			const Vec4V _normalVel2 = V4MulAdd(raXnT1, angVel0T1, __normalVel2);
			const Vec4V _normalVel4 = V4MulAdd(rbXnT1, angVel1T1, __normalVel4);

			
			const Vec4V normalVel2 = V4MulAdd(raXnT2, angVel0T2, _normalVel2);
			const Vec4V normalVel4 = V4MulAdd(rbXnT2, angVel1T2, _normalVel4);

			const Vec4V biasedErr = V4MulAdd(targetVel, velMultiplier, V4Neg(scaledBias));

			//Linear component - normal * invMass_dom

			const Vec4V _normalVel(V4Add(normalVel1, normalVel2));
			const Vec4V __normalVel(V4Add(normalVel3, normalVel4));
		
			const Vec4V normalVel = V4Sub(_normalVel, __normalVel );

			const Vec4V _deltaF = V4NegMulSub(normalVel, velMultiplier, biasedErr);
			const Vec4V nAppliedForce = V4Neg(appliedForce);
			const Vec4V _deltaF2 = V4Max(_deltaF, nAppliedForce);
			const Vec4V _newAppliedForce(V4Add(appliedForce, _deltaF2));
			const Vec4V newAppliedForce = V4Min(_newAppliedForce, maxImpulse);
			const Vec4V deltaF = V4Sub(newAppliedForce, appliedForce);

			normalVel1 = V4MulAdd(invMass0D0, deltaF, normalVel1);
			normalVel3 = V4MulAdd(invMass1D1, deltaF, normalVel3);

			accumDeltaF = V4Add(deltaF, accumDeltaF);

			angVel0T0 = V4MulAdd(delAngVel0T0, deltaF, angVel0T0);
			angVel1T0 = V4MulAdd(delAngVel1T0, deltaF, angVel1T0);

			angVel0T1 = V4MulAdd(delAngVel0T1, deltaF, angVel0T1);
			angVel1T1 = V4MulAdd(delAngVel1T1, deltaF, angVel1T1);

			angVel0T2 = V4MulAdd(delAngVel0T2, deltaF, angVel0T2);
			angVel1T2 = V4MulAdd(delAngVel1T2, deltaF, angVel1T2);

			c.appliedForce = newAppliedForce;
			appliedForceBuffer[i] = newAppliedForce;
		}
		linVel0T0 = V4MulAdd(delLinVel00, accumDeltaF, linVel0T0);
		linVel1T0 = V4MulAdd(delLinVel01, accumDeltaF, linVel1T0);
		linVel0T1 = V4MulAdd(delLinVel10, accumDeltaF, linVel0T1);
		linVel1T1 = V4MulAdd(delLinVel11, accumDeltaF, linVel1T1);
		linVel0T2 = V4MulAdd(delLinVel20, accumDeltaF, linVel0T2);
		linVel1T2 = V4MulAdd(delLinVel21, accumDeltaF, linVel1T2);
	}

	PX_ASSERT(currPtr == last);
	

	PX_TRANSPOSE_34_44(linVel0T0, linVel0T1, linVel0T2, linVel00, linVel10, linVel20, linVel30);
	PX_TRANSPOSE_34_44(linVel1T0, linVel1T1, linVel1T2, linVel01, linVel11, linVel21, linVel31);
	PX_TRANSPOSE_34_44(angVel0T0, angVel0T1, angVel0T2, angVel00, angVel10, angVel20, angVel30);
	PX_TRANSPOSE_34_44(angVel1T0, angVel1T1, angVel1T2, angVel01, angVel11, angVel21, angVel31);


	// Write back
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel00), b00.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel10), b10.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel20), b20.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel30), b30.linearVelocity);

	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel01), b01.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel11), b11.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel21), b21.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel31), b31.linearVelocity);

	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel00), b00.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel10), b10.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel20), b20.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel30), b30.angularVelocity);

	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel01), b01.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel11), b11.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel21), b21.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel31), b31.angularVelocity);
}


static void solveContactCoulomb4_StaticBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& /*cache*/)
{
	PxcSolverBody& b00 = *desc[0].bodyA;
	PxcSolverBody& b10 = *desc[1].bodyA;
	PxcSolverBody& b20 = *desc[2].bodyA;
	PxcSolverBody& b30 = *desc[3].bodyA;

	//We'll need this.
	const Vec4V vZero	= V4Zero();
		
	Vec4V linVel00 = V4LoadA(&b00.linearVelocity.x);
	Vec4V angVel00 = V4LoadA(&b00.angularVelocity.x);

	Vec4V linVel10 = V4LoadA(&b10.linearVelocity.x);
	Vec4V angVel10 = V4LoadA(&b10.angularVelocity.x);

	Vec4V linVel20 = V4LoadA(&b20.linearVelocity.x);
	Vec4V angVel20 = V4LoadA(&b20.angularVelocity.x);

	Vec4V linVel30 = V4LoadA(&b30.linearVelocity.x);
	Vec4V angVel30 = V4LoadA(&b30.angularVelocity.x);


	Vec4V linVel0T0, linVel0T1, linVel0T2;
	Vec4V angVel0T0, angVel0T1, angVel0T2;


	PX_TRANSPOSE_44_34(linVel00, linVel10, linVel20, linVel30, linVel0T0, linVel0T1, linVel0T2);
	PX_TRANSPOSE_44_34(angVel00, angVel10, angVel20, angVel30, angVel0T0, angVel0T1, angVel0T2);
	

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc[0].constraint;

	PxcSolverContactCoulombHeader4* PX_RESTRICT firstHeader = (PxcSolverContactCoulombHeader4*)currPtr;

	const PxU8* PX_RESTRICT last = desc[0].constraint + firstHeader->frictionOffset;


	//TODO - can I avoid this many tests???
	while(currPtr < last)
	{

		PxcSolverContactCoulombHeader4* PX_RESTRICT hdr = (PxcSolverContactCoulombHeader4*)currPtr;

		Vec4V* appliedForceBuffer = (Vec4V*)(currPtr + hdr->frictionOffset + sizeof(PxcSolverFrictionHeader4));

		currPtr = (PxU8*)(hdr + 1);

		const PxU32 numNormalConstr = hdr->numNormalConstr;

		PxcSolverContact4Base* PX_RESTRICT contacts = (PxcSolverContact4Base*)currPtr;
	
		currPtr = (PxU8*)(contacts + numNormalConstr);

		const Vec4V invMass0D0 = hdr->invMassADom;

		const Vec4V normalT0 = hdr->normalX;
		const Vec4V normalT1 = hdr->normalY;
		const Vec4V normalT2 = hdr->normalZ;

		const Vec4V __normalVel1 = V4Mul(linVel0T0, normalT0);
		const Vec4V _normalVel1 = V4MulAdd(linVel0T1, normalT1, __normalVel1);

		const Vec4V delLinVel00 = V4Mul(normalT0, invMass0D0);

		const Vec4V delLinVel10 = V4Mul(normalT1, invMass0D0);

		const Vec4V delLinVel20 = V4Mul(normalT2, invMass0D0);

		Vec4V normalVel1 = V4MulAdd(linVel0T2, normalT2, _normalVel1);

		Vec4V accumDeltaF = vZero;

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContact4Base& c = contacts[i];
			Ps::prefetchLine((&contacts[i+1]));
			Ps::prefetchLine((&contacts[i+1]), 128);
			Ps::prefetchLine((&contacts[i+1]), 256);

			const Vec4V delAngVel0T0 = c.delAngVel0X;
			const Vec4V delAngVel0T1 = c.delAngVel0Y;
			const Vec4V delAngVel0T2 = c.delAngVel0Z;

			const Vec4V appliedForce = c.appliedForce;
			const Vec4V velMultiplier = c.velMultiplier;
			
			const Vec4V targetVel = c.targetVelocity;
			const Vec4V scaledBias = c.scaledBias;
			const Vec4V maxImpulse = c.maxImpulse;

			const Vec4V raXnT0 = c.raXnX;
			const Vec4V raXnT1 = c.raXnY;
			const Vec4V raXnT2 = c.raXnZ;

			
			const Vec4V __normalVel2 = V4Mul(raXnT0, angVel0T0);
			
			const Vec4V _normalVel2 = V4MulAdd(raXnT1, angVel0T1, __normalVel2);
			
			const Vec4V normalVel2 = V4MulAdd(raXnT2, angVel0T2, _normalVel2);

			const Vec4V biasedErr = V4MulAdd(targetVel, velMultiplier, V4Neg(scaledBias));

			//Linear component - normal * invMass_dom

			const Vec4V normalVel(V4Add(normalVel1, normalVel2));

			const Vec4V _deltaF = V4NegMulSub(normalVel, velMultiplier, biasedErr);
			const Vec4V nAppliedForce = V4Neg(appliedForce);

			const Vec4V _deltaF2 = V4Max(_deltaF, nAppliedForce);

			const Vec4V _newAppliedForce(V4Add(appliedForce, _deltaF2));
			const Vec4V newAppliedForce = V4Min(_newAppliedForce, maxImpulse);
			const Vec4V deltaF = V4Sub(newAppliedForce, appliedForce);

			normalVel1 = V4MulAdd(invMass0D0, deltaF, normalVel1);

			accumDeltaF = V4Add(deltaF, accumDeltaF);

			angVel0T0 = V4MulAdd(delAngVel0T0, deltaF, angVel0T0);

			angVel0T1 = V4MulAdd(delAngVel0T1, deltaF, angVel0T1);

			angVel0T2 = V4MulAdd(delAngVel0T2, deltaF, angVel0T2);

			c.appliedForce = newAppliedForce;
			appliedForceBuffer[i] = newAppliedForce;
		}
		linVel0T0 = V4MulAdd(delLinVel00, accumDeltaF, linVel0T0);
		linVel0T1 = V4MulAdd(delLinVel10, accumDeltaF, linVel0T1);
		linVel0T2 = V4MulAdd(delLinVel20, accumDeltaF, linVel0T2);
	}

	PX_ASSERT(currPtr == last);
	

	PX_TRANSPOSE_34_44(linVel0T0, linVel0T1, linVel0T2, linVel00, linVel10, linVel20, linVel30);
	PX_TRANSPOSE_34_44(angVel0T0, angVel0T1, angVel0T2, angVel00, angVel10, angVel20, angVel30);

	// Write back
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel00), b00.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel10), b10.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel20), b20.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel30), b30.linearVelocity);

	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel00), b00.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel10), b10.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel20), b20.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel30), b30.angularVelocity);
}

static void solveFriction4_Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& /*cache*/)
{
	PxcSolverBody& b00 = *desc[0].bodyA;
	PxcSolverBody& b01 = *desc[0].bodyB;
	PxcSolverBody& b10 = *desc[1].bodyA;
	PxcSolverBody& b11 = *desc[1].bodyB;
	PxcSolverBody& b20 = *desc[2].bodyA;
	PxcSolverBody& b21 = *desc[2].bodyB;
	PxcSolverBody& b30 = *desc[3].bodyA;
	PxcSolverBody& b31 = *desc[3].bodyB;


	Vec4V linVel00 = V4LoadA(&b00.linearVelocity.x);
	Vec4V linVel01 = V4LoadA(&b01.linearVelocity.x);
	Vec4V angVel00 = V4LoadA(&b00.angularVelocity.x);
	Vec4V angVel01 = V4LoadA(&b01.angularVelocity.x);

	Vec4V linVel10 = V4LoadA(&b10.linearVelocity.x);
	Vec4V linVel11 = V4LoadA(&b11.linearVelocity.x);
	Vec4V angVel10 = V4LoadA(&b10.angularVelocity.x);
	Vec4V angVel11 = V4LoadA(&b11.angularVelocity.x);

	Vec4V linVel20 = V4LoadA(&b20.linearVelocity.x);
	Vec4V linVel21 = V4LoadA(&b21.linearVelocity.x);
	Vec4V angVel20 = V4LoadA(&b20.angularVelocity.x);
	Vec4V angVel21 = V4LoadA(&b21.angularVelocity.x);

	Vec4V linVel30 = V4LoadA(&b30.linearVelocity.x);
	Vec4V linVel31 = V4LoadA(&b31.linearVelocity.x);
	Vec4V angVel30 = V4LoadA(&b30.angularVelocity.x);
	Vec4V angVel31 = V4LoadA(&b31.angularVelocity.x);


	Vec4V linVel0T0, linVel0T1, linVel0T2;
	Vec4V linVel1T0, linVel1T1, linVel1T2;
	Vec4V angVel0T0, angVel0T1, angVel0T2;
	Vec4V angVel1T0, angVel1T1, angVel1T2;


	PX_TRANSPOSE_44_34(linVel00, linVel10, linVel20, linVel30, linVel0T0, linVel0T1, linVel0T2);
	PX_TRANSPOSE_44_34(linVel01, linVel11, linVel21, linVel31, linVel1T0, linVel1T1, linVel1T2);
	PX_TRANSPOSE_44_34(angVel00, angVel10, angVel20, angVel30, angVel0T0, angVel0T1, angVel0T2);
	PX_TRANSPOSE_44_34(angVel01, angVel11, angVel21, angVel31, angVel1T0, angVel1T1, angVel1T2);

	PxU8* PX_RESTRICT currPtr = desc[0].constraint;
	PxU8* PX_RESTRICT endPtr = desc[0].constraint + getConstraintLength(desc[0]);
	

	while(currPtr < endPtr)
	{
		PxcSolverFrictionHeader4* PX_RESTRICT hdr = (PxcSolverFrictionHeader4*)currPtr;

		currPtr = (PxU8*)(hdr + 1);

		Vec4V* appliedImpulses = (Vec4V*)currPtr;

		currPtr += hdr->numNormalConstr * sizeof(Vec4V);

		Ps::prefetchLine(currPtr, 128);
		Ps::prefetchLine(currPtr,256);
		Ps::prefetchLine(currPtr,384);
		
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		PxcSolverFriction4Dynamic* PX_RESTRICT frictions = (PxcSolverFriction4Dynamic*)currPtr;

		currPtr = (PxU8*)(frictions + hdr->numFrictionConstr);

		const PxU32 maxFrictionConstr = numFrictionConstr;
	
		const Vec4V staticFric = hdr->staticFriction;

		const Vec4V invMass0D0 = hdr->invMassADom;
		const Vec4V invMass1D1 = hdr->invMassBDom;

		for(PxU32 i=0;i<maxFrictionConstr;i++)
		{
			PxcSolverFriction4Dynamic& f = frictions[i];
			Ps::prefetchLine((&f)+1);
			Ps::prefetchLine((&f)+1,128);
			Ps::prefetchLine((&f)+1,256);
			Ps::prefetchLine((&f)+1,384);

			const Vec4V appliedImpulse = appliedImpulses[i>>hdr->frictionPerContact];

			const Vec4V maxFriction =  V4Mul(staticFric, appliedImpulse);

			const Vec4V nMaxFriction = V4Neg(maxFriction); 

			const Vec4V delAngVel0X = f.delAngVel0X;
			const Vec4V delAngVel0Y = f.delAngVel0Y;
			const Vec4V delAngVel0Z = f.delAngVel0Z;

			const Vec4V delAngVel1X = f.delAngVel1X;
			const Vec4V delAngVel1Y = f.delAngVel1Y;
			const Vec4V delAngVel1Z = f.delAngVel1Z;

			const Vec4V normalX = f.normalX;
			const Vec4V normalY = f.normalY;
			const Vec4V normalZ = f.normalZ;

			const Vec4V raXnX = f.raXnX;
			const Vec4V raXnY = f.raXnY;
			const Vec4V raXnZ = f.raXnZ;

			const Vec4V rbXnX = f.rbXnX;
			const Vec4V rbXnY = f.rbXnY;
			const Vec4V rbXnZ = f.rbXnZ;

			const Vec4V appliedForce(f.appliedForce);
			const Vec4V velMultiplier(f.velMultiplier);
			const Vec4V targetVel(f.targetVelocity);
	
			//4 x 4 Dot3 products encoded as 8 M44 transposes, 4 MulV and 8 MulAdd ops

			const Vec4V __normalVel1 = V4Mul(linVel0T0, normalX);
			const Vec4V __normalVel2 = V4Mul(raXnX, angVel0T0);
			const Vec4V __normalVel3 = V4Mul(linVel1T0, normalX);
			const Vec4V __normalVel4 = V4Mul(rbXnX, angVel1T0);

			const Vec4V _normalVel1 = V4MulAdd(linVel0T1, normalY, __normalVel1);
			const Vec4V _normalVel2 = V4MulAdd(raXnY, angVel0T1, __normalVel2);
			const Vec4V _normalVel3 = V4MulAdd(linVel1T1, normalY, __normalVel3);
			const Vec4V _normalVel4 = V4MulAdd(rbXnY, angVel1T1, __normalVel4);

			const Vec4V normalVel1 = V4MulAdd(linVel0T2, normalZ, _normalVel1);
			const Vec4V normalVel2 = V4MulAdd(raXnZ, angVel0T2, _normalVel2);
			const Vec4V normalVel3 = V4MulAdd(linVel1T2, normalZ, _normalVel3);
			const Vec4V normalVel4 = V4MulAdd(rbXnZ, angVel1T2, _normalVel4);

			const Vec4V delLinVel00 = V4Mul(normalX, invMass0D0);
			const Vec4V delLinVel01 = V4Mul(normalX, invMass1D1);

			const Vec4V delLinVel10 = V4Mul(normalY, invMass0D0);
			const Vec4V delLinVel11 = V4Mul(normalY, invMass1D1);

			const Vec4V _normalVel = V4Add(normalVel1, normalVel2);
			const Vec4V __normalVel = V4Add(normalVel3, normalVel4);

			const Vec4V delLinVel20 = V4Mul(normalZ, invMass0D0);
			const Vec4V delLinVel21 = V4Mul(normalZ, invMass1D1);

			const Vec4V normalVel = V4Sub(_normalVel, __normalVel );

			const Vec4V tmp = V4NegMulSub(targetVel, velMultiplier, appliedForce);
			//Vec4V newAppliedForce = V4MulAdd(normalVel, velMultiplier, appliedForce);
			Vec4V newAppliedForce = V4MulAdd(normalVel, velMultiplier, tmp);
			newAppliedForce = V4Clamp(newAppliedForce,nMaxFriction,  maxFriction);
			const Vec4V deltaF = V4Sub(newAppliedForce, appliedForce);


			linVel0T0 = V4MulAdd(delLinVel00, deltaF, linVel0T0);
			linVel1T0 = V4MulAdd(delLinVel01, deltaF, linVel1T0);
			angVel0T0 = V4MulAdd(delAngVel0X, deltaF, angVel0T0);
			angVel1T0 = V4MulAdd(delAngVel1X, deltaF, angVel1T0);

			linVel0T1 = V4MulAdd(delLinVel10, deltaF, linVel0T1);
			linVel1T1 = V4MulAdd(delLinVel11, deltaF, linVel1T1);
			angVel0T1 = V4MulAdd(delAngVel0Y, deltaF, angVel0T1);
			angVel1T1 = V4MulAdd(delAngVel1Y, deltaF, angVel1T1);

			linVel0T2 = V4MulAdd(delLinVel20, deltaF, linVel0T2);
			linVel1T2 = V4MulAdd(delLinVel21, deltaF, linVel1T2);
			angVel0T2 = V4MulAdd(delAngVel0Z, deltaF, angVel0T2);
			angVel1T2 = V4MulAdd(delAngVel1Z, deltaF, angVel1T2);

			f.appliedForce = newAppliedForce;
		}
	}

	PX_ASSERT(currPtr == endPtr);

	PX_TRANSPOSE_34_44(linVel0T0, linVel0T1, linVel0T2, linVel00, linVel10, linVel20, linVel30);
	PX_TRANSPOSE_34_44(linVel1T0, linVel1T1, linVel1T2, linVel01, linVel11, linVel21, linVel31);
	PX_TRANSPOSE_34_44(angVel0T0, angVel0T1, angVel0T2, angVel00, angVel10, angVel20, angVel30);
	PX_TRANSPOSE_34_44(angVel1T0, angVel1T1, angVel1T2, angVel01, angVel11, angVel21, angVel31);


	/*PxVec3 vel;
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel00), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel10), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel20), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel30), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel00), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel10), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel20), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel30), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);

	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel01), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel11), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel21), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel31), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel01), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel11), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel21), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel31), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);*/


	// Write back
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel00), b00.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel10), b10.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel20), b20.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel30), b30.linearVelocity);

	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel01), b01.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel11), b11.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel21), b21.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel31), b31.linearVelocity);

	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel00), b00.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel10), b10.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel20), b20.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel30), b30.angularVelocity);

	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel01), b01.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel11), b11.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel21), b21.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel31), b31.angularVelocity);

}


static void solveFriction4_StaticBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& /*cache*/)
{

	PxcSolverBody& b00 = *desc[0].bodyA;
	PxcSolverBody& b10 = *desc[1].bodyA;
	PxcSolverBody& b20 = *desc[2].bodyA;
	PxcSolverBody& b30 = *desc[3].bodyA;


	Vec4V linVel00 = V4LoadA(&b00.linearVelocity.x);
	Vec4V angVel00 = V4LoadA(&b00.angularVelocity.x);

	Vec4V linVel10 = V4LoadA(&b10.linearVelocity.x);
	Vec4V angVel10 = V4LoadA(&b10.angularVelocity.x);

	Vec4V linVel20 = V4LoadA(&b20.linearVelocity.x);
	Vec4V angVel20 = V4LoadA(&b20.angularVelocity.x);

	Vec4V linVel30 = V4LoadA(&b30.linearVelocity.x);
	Vec4V angVel30 = V4LoadA(&b30.angularVelocity.x);


	Vec4V linVel0T0, linVel0T1, linVel0T2;
	Vec4V angVel0T0, angVel0T1, angVel0T2;


	PX_TRANSPOSE_44_34(linVel00, linVel10, linVel20, linVel30, linVel0T0, linVel0T1, linVel0T2);
	PX_TRANSPOSE_44_34(angVel00, angVel10, angVel20, angVel30, angVel0T0, angVel0T1, angVel0T2);

	PxU8* PX_RESTRICT currPtr = desc[0].constraint;
	PxU8* PX_RESTRICT endPtr = desc[0].constraint + getConstraintLength(desc[0]);
	

	while(currPtr < endPtr)
	{
		PxcSolverFrictionHeader4* PX_RESTRICT hdr = (PxcSolverFrictionHeader4*)currPtr;

		currPtr = (PxU8*)(hdr + 1);

		Vec4V* appliedImpulses = (Vec4V*)currPtr;

		currPtr += hdr->numNormalConstr * sizeof(Vec4V);

		Ps::prefetchLine(currPtr, 128);
		Ps::prefetchLine(currPtr,256);
		Ps::prefetchLine(currPtr,384);
		
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		PxcSolverFriction4Base* PX_RESTRICT frictions = (PxcSolverFriction4Base*)currPtr;

		currPtr = (PxU8*)(frictions + hdr->numFrictionConstr);

		const PxU32 maxFrictionConstr = numFrictionConstr;
	
		const Vec4V staticFric = hdr->staticFriction;

		const Vec4V invMass0D0 = hdr->invMassADom;

		for(PxU32 i=0;i<maxFrictionConstr;i++)
		{
			PxcSolverFriction4Base& f = frictions[i];
			Ps::prefetchLine((&f)+1);
			Ps::prefetchLine((&f)+1,128);
			Ps::prefetchLine((&f)+1,256);

			const Vec4V appliedImpulse = appliedImpulses[i>>hdr->frictionPerContact];

			const Vec4V maxFriction =  V4Mul(staticFric, appliedImpulse);

			const Vec4V nMaxFriction = V4Neg(maxFriction); 

			const Vec4V delAngVel0X = f.delAngVel0X;
			const Vec4V delAngVel0Y = f.delAngVel0Y;
			const Vec4V delAngVel0Z = f.delAngVel0Z;

			const Vec4V normalX = f.normalX;
			const Vec4V normalY = f.normalY;
			const Vec4V normalZ = f.normalZ;

			const Vec4V raXnX = f.raXnX;
			const Vec4V raXnY = f.raXnY;
			const Vec4V raXnZ = f.raXnZ;

			const Vec4V appliedForce(f.appliedForce);
			const Vec4V velMultiplier(f.velMultiplier);
			const Vec4V targetVel(f.targetVelocity);
	
			//4 x 4 Dot3 products encoded as 8 M44 transposes, 4 MulV and 8 MulAdd ops

			const Vec4V __normalVel1 = V4Mul(linVel0T0, normalX);
			const Vec4V __normalVel2 = V4Mul(raXnX, angVel0T0);

			const Vec4V _normalVel1 = V4MulAdd(linVel0T1, normalY, __normalVel1);
			const Vec4V _normalVel2 = V4MulAdd(raXnY, angVel0T1, __normalVel2);

			const Vec4V normalVel1 = V4MulAdd(linVel0T2, normalZ, _normalVel1);
			const Vec4V normalVel2 = V4MulAdd(raXnZ, angVel0T2, _normalVel2);

			const Vec4V delLinVel00 = V4Mul(normalX, invMass0D0);

			const Vec4V delLinVel10 = V4Mul(normalY, invMass0D0);

			const Vec4V normalVel = V4Add(normalVel1, normalVel2);

			const Vec4V delLinVel20 = V4Mul(normalZ, invMass0D0);

			const Vec4V tmp = V4NegMulSub(targetVel, velMultiplier, appliedForce);

			Vec4V newAppliedForce = V4MulAdd(normalVel, velMultiplier, tmp);
			newAppliedForce = V4Clamp(newAppliedForce,nMaxFriction,  maxFriction);
			const Vec4V deltaF = V4Sub(newAppliedForce, appliedForce);


			linVel0T0 = V4MulAdd(delLinVel00, deltaF, linVel0T0);
			angVel0T0 = V4MulAdd(delAngVel0X, deltaF, angVel0T0);

			linVel0T1 = V4MulAdd(delLinVel10, deltaF, linVel0T1);
			angVel0T1 = V4MulAdd(delAngVel0Y, deltaF, angVel0T1);

			linVel0T2 = V4MulAdd(delLinVel20, deltaF, linVel0T2);
			angVel0T2 = V4MulAdd(delAngVel0Z, deltaF, angVel0T2);

			f.appliedForce = newAppliedForce;
		}
	}

	PX_ASSERT(currPtr == endPtr);

	PX_TRANSPOSE_34_44(linVel0T0, linVel0T1, linVel0T2, linVel00, linVel10, linVel20, linVel30);
	PX_TRANSPOSE_34_44(angVel0T0, angVel0T1, angVel0T2, angVel00, angVel10, angVel20, angVel30);


	/*PxVec3 vel;
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel00), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel10), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel20), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(linVel30), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100000.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel00), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel10), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel20), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);
	PxVec3_From_Vec3V(Vec3V_From_Vec4V_WUndefined(angVel30), vel);
	PX_ASSERT(vel.isFinite() && vel.magnitudeSquared() < 100.f);*/


	// Write back
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel00), b00.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel10), b10.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel20), b20.linearVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(linVel30), b30.linearVelocity);

	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel00), b00.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel10), b10.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel20), b20.angularVelocity);
	V3StoreU(Vec3V_From_Vec4V_WUndefined(angVel30), b30.angularVelocity);

}

static void concludeContactCoulomb4(const PxcSolverConstraintDesc* desc, PxcSolverContext& /*cache*/)
{
	PxU8* PX_RESTRICT cPtr = desc[0].constraint;

	const Vec4V zero = V4Zero();

	const PxcSolverContactCoulombHeader4* PX_RESTRICT firstHeader = (const PxcSolverContactCoulombHeader4*)cPtr;
	PxU8* PX_RESTRICT last = desc[0].constraint + firstHeader->frictionOffset;

	PxU32 pointStride = firstHeader->type == PXS_SC_TYPE_BLOCK_RB_CONTACT ? sizeof(PxcSolverContact4Dynamic) : sizeof(PxcSolverContact4Base);

	while(cPtr < last)
	{
		const PxcSolverContactCoulombHeader4* PX_RESTRICT hdr = (const PxcSolverContactCoulombHeader4*)cPtr;
		cPtr += sizeof(PxcSolverContactCoulombHeader4);

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		
		//if(cPtr < last)
		//Ps::prefetchLine(cPtr, 512);
		Ps::prefetchLine(cPtr,128);
		Ps::prefetchLine(cPtr,256);
		Ps::prefetchLine(cPtr,384);

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContact4Base *c = reinterpret_cast<PxcSolverContact4Base*>(cPtr);
			cPtr += pointStride;
			c->scaledBias = V4Max(c->scaledBias, zero);
		}
	}
	PX_ASSERT(cPtr == last);
}

void  writeBackContactCoulomb4(const PxcSolverConstraintDesc* desc, PxcSolverContext& cache,
					  const PxcSolverBodyData** PX_RESTRICT bd0, const PxcSolverBodyData** PX_RESTRICT bd1)
{
	Vec4V normalForceV = V4Zero();
	PxU8* PX_RESTRICT cPtr = desc[0].constraint;
	PxReal* PX_RESTRICT vForceWriteback0 = reinterpret_cast<PxReal*>(desc[0].writeBack);
	PxReal* PX_RESTRICT vForceWriteback1 = reinterpret_cast<PxReal*>(desc[1].writeBack);
	PxReal* PX_RESTRICT vForceWriteback2 = reinterpret_cast<PxReal*>(desc[2].writeBack);
	PxReal* PX_RESTRICT vForceWriteback3 = reinterpret_cast<PxReal*>(desc[3].writeBack);

	const PxcSolverContactCoulombHeader4* PX_RESTRICT firstHeader = (const PxcSolverContactCoulombHeader4*)cPtr;
	PxU8* PX_RESTRICT last = desc[0].constraint + firstHeader->frictionOffset;

	const PxU32 pointStride = firstHeader->type == PXS_SC_TYPE_BLOCK_RB_CONTACT ? sizeof(PxcSolverContact4Dynamic)
																	   : sizeof(PxcSolverContact4Base);

	bool writeBackThresholds[4] = {false, false, false, false};

	while(cPtr < last)
	{
		const PxcSolverContactCoulombHeader4* PX_RESTRICT hdr = (const PxcSolverContactCoulombHeader4*)cPtr;
		cPtr += sizeof(PxcSolverContactCoulombHeader4);

		writeBackThresholds[0] = hdr->flags[0] & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;
		writeBackThresholds[1] = hdr->flags[1] & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;
		writeBackThresholds[2] = hdr->flags[2] & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;
		writeBackThresholds[3] = hdr->flags[3] & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;

		const PxU32 numNormalConstr = hdr->numNormalConstr;

		Ps::prefetchLine(cPtr, 256);
		Ps::prefetchLine(cPtr, 384);

		
		for(PxU32 i=0; i<numNormalConstr; i++)
		{
			PxcSolverContact4Base* c = reinterpret_cast<PxcSolverContact4Base*>(cPtr);
			cPtr += pointStride;

			const Vec4V appliedForce = c->appliedForce;
			if(vForceWriteback0 && i < hdr->numNormalConstr0)
				FStore(V4GetX(appliedForce), vForceWriteback0++);
			if(vForceWriteback1 && i < hdr->numNormalConstr1)
				FStore(V4GetY(appliedForce), vForceWriteback1++);
			if(vForceWriteback2 && i < hdr->numNormalConstr2)
				FStore(V4GetZ(appliedForce), vForceWriteback2++);
			if(vForceWriteback3 && i < hdr->numNormalConstr3)
				FStore(V4GetW(appliedForce), vForceWriteback3++);
			
			normalForceV = V4Add(normalForceV, appliedForce);
		}
	}
	PX_ASSERT(cPtr == last);

	PX_ALIGN(16, PxReal nf[4]);
	V4StoreA(normalForceV, nf);

	for(PxU32 a = 0; a < 4; ++a)
	{
		if(writeBackThresholds[a] && desc[a].linkIndexA == PxcSolverConstraintDesc::NO_LINK && desc[a].linkIndexB == PxcSolverConstraintDesc::NO_LINK &&
			PX_IR(nf[a]) !=0 && (bd0[a]->reportThreshold < PX_MAX_REAL  || bd1[a]->reportThreshold < PX_MAX_REAL))
		{
			PxcThresholdStreamElement elt;
			elt.normalForce = nf[a];
			elt.threshold = PxMin<float>(bd0[a]->reportThreshold, bd1[a]->reportThreshold);
			elt.body0 = bd0[a]->originalBody;
			elt.body1 = bd1[a]->originalBody;
			Ps::order(elt.body0,elt.body1);
			PX_ASSERT(elt.body0 < elt.body1);
			PX_ASSERT(cache.mThresholdStreamIndex<cache.mThresholdStreamLength);
			cache.mThresholdStream[cache.mThresholdStreamIndex++] = elt;
		}
	}
}

void solveContactCoulombPreBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 /*constraintCount*/, PxcSolverContext& cache)
{
	solveContactCoulomb4_Block(desc, cache);
}

void solveContactCoulombPreBlock_Static(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveContactCoulomb4_StaticBlock(desc, cache);
}

void solveContactCoulombPreBlock_Conclude(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveContactCoulomb4_Block(desc, cache);
	concludeContactCoulomb4(desc, cache);
}

void solveContactCoulombPreBlock_ConcludeStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveContactCoulomb4_StaticBlock(desc, cache);
	concludeContactCoulomb4(desc, cache);
}

void solveContactCoulombPreBlock_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	solveContactCoulomb4_Block(desc, cache);

	const PxcSolverBodyData* bd0[4] = {	&cache.solverBodyArray[desc[0].bodyADataIndex], 
										&cache.solverBodyArray[desc[1].bodyADataIndex],
										&cache.solverBodyArray[desc[2].bodyADataIndex],
										&cache.solverBodyArray[desc[3].bodyADataIndex]};

	const PxcSolverBodyData* bd1[4] = {	&cache.solverBodyArray[desc[0].bodyBDataIndex], 
										&cache.solverBodyArray[desc[1].bodyBDataIndex],
										&cache.solverBodyArray[desc[2].bodyBDataIndex],
										&cache.solverBodyArray[desc[3].bodyBDataIndex]};

	writeBackContactCoulomb4(desc, cache, bd0, bd1);

	if(cache.mThresholdStreamIndex > (cache.mThresholdStreamLength - 4))
	{
		//Write back to global buffer
		PxI32 threshIndex = physx::shdfnd::atomicAdd(outThresholdPairs, (PxI32)cache.mThresholdStreamIndex) - (PxI32)cache.mThresholdStreamIndex;
		for(PxU32 a = 0; a < cache.mThresholdStreamIndex; ++a)
		{
			thresholdStream[a + threshIndex] = cache.mThresholdStream[a];
		}
		cache.mThresholdStreamIndex = 0;
	}
}

void solveContactCoulombPreBlock_WriteBackStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 /*constraintCount*/, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	solveContactCoulomb4_StaticBlock(desc, cache);
	const PxcSolverBodyData* bd0[4] = {	&cache.solverBodyArray[desc[0].bodyADataIndex], 
										&cache.solverBodyArray[desc[1].bodyADataIndex],
										&cache.solverBodyArray[desc[2].bodyADataIndex],
										&cache.solverBodyArray[desc[3].bodyADataIndex]};

	const PxcSolverBodyData* bd1[4] = {	&cache.solverBodyArray[desc[0].bodyBDataIndex], 
										&cache.solverBodyArray[desc[1].bodyBDataIndex],
										&cache.solverBodyArray[desc[2].bodyBDataIndex],
										&cache.solverBodyArray[desc[3].bodyBDataIndex]};

	writeBackContactCoulomb4(desc, cache, bd0, bd1);

	if(cache.mThresholdStreamIndex > (cache.mThresholdStreamLength - 4))
	{
		//Write back to global buffer
		PxI32 threshIndex = physx::shdfnd::atomicAdd(outThresholdPairs, (PxI32)cache.mThresholdStreamIndex) - (PxI32)cache.mThresholdStreamIndex;
		for(PxU32 a = 0; a < cache.mThresholdStreamIndex; ++a)
		{
			thresholdStream[a + threshIndex] = cache.mThresholdStream[a];
		}
		cache.mThresholdStreamIndex = 0;
	}
}

void solveFrictionCoulombPreBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveFriction4_Block(desc, cache);
}

void solveFrictionCoulombPreBlock_Static(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveFriction4_StaticBlock(desc, cache);
}

void solveFrictionCoulombPreBlock_Conclude(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveFriction4_Block(desc, cache);
}

void solveFrictionCoulombPreBlock_ConcludeStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveFriction4_StaticBlock(desc, cache);
}

void solveFrictionCoulombPreBlock_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	solveFriction4_Block(desc, cache);
}

void solveFrictionCoulombPreBlock_WriteBackStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	solveFriction4_StaticBlock(desc, cache);
}


}

