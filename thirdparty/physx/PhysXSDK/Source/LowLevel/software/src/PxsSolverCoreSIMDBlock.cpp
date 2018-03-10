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

#ifdef PX_SUPPORT_SIMD

#include "CmPhysXCommon.h"
#include "PxcSolverBody.h"
#include "PxcSolverContact.h"
#include "PxcSolverConstraint1D.h"
#include "PxcSolverConstraintDesc.h"
#include "PxcThresholdStreamElement.h"
#include "PxsSolverContext.h"
#include "PsUtilities.h"
#include "PxvShader.h"
#include "PsAtomic.h"
#include "PxcThresholdStreamElement.h"
#include "PxsSolverCoreGeneral.h"
#include "PxcSolverContact4.h"
#include "PxcSolverConstraint1D4.h"

namespace physx
{

static void solveContact4_Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& cache)
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


	Vec4V linVel0T0, linVel0T1, linVel0T2, linVel0T3;
	Vec4V linVel1T0, linVel1T1, linVel1T2, linVel1T3;
	Vec4V angVel0T0, angVel0T1, angVel0T2, angVel0T3;
	Vec4V angVel1T0, angVel1T1, angVel1T2, angVel1T3;


	PX_TRANSPOSE_44(linVel00, linVel10, linVel20, linVel30, linVel0T0, linVel0T1, linVel0T2, linVel0T3);
	PX_TRANSPOSE_44(linVel01, linVel11, linVel21, linVel31, linVel1T0, linVel1T1, linVel1T2, linVel1T3);
	PX_TRANSPOSE_44(angVel00, angVel10, angVel20, angVel30, angVel0T0, angVel0T1, angVel0T2, angVel0T3);
	PX_TRANSPOSE_44(angVel01, angVel11, angVel21, angVel31, angVel1T0, angVel1T1, angVel1T2, angVel1T3);


	const PxU8* PX_RESTRICT last = desc[0].constraint + getConstraintLength(desc[0]);

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc[0].constraint;

	Vec4V vMax = V4Splat(FMax());


	//TODO - can I avoid this many tests???
#if 1//DO_PREFETCHES
	const PxU8* PX_RESTRICT prefetchAddress = currPtr + sizeof(PxcSolverContactHeader4) + sizeof(PxcSolverContactBatchPointDynamic4);
#endif

	while(currPtr < last)
	{

		PxcSolverContactHeader4* PX_RESTRICT hdr = (PxcSolverContactHeader4*)currPtr;

		currPtr = (PxU8*)(hdr + 1);

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		bool hasMaxImpulse = (hdr->flag & PxcSolverContactHeader4::eHAS_MAX_IMPULSE) != 0;
		bool hasTargetVel = (hdr->flag & PxcSolverContactHeader4::eHAS_TARGET_VELOCITY) != 0;

		Vec4V* appliedForces = (Vec4V*)currPtr;
		currPtr += sizeof(Vec4V)*numNormalConstr;

		PxcSolverContactBatchPointDynamic4* PX_RESTRICT contacts = (PxcSolverContactBatchPointDynamic4*)currPtr;
		//const Vec4V dominance1 = V4Neg(__dominance1);

		Vec4V* maxImpulses;
		currPtr = (PxU8*)(contacts + numNormalConstr);
		PxU32 maxImpulseMask = 0;
		if(hasMaxImpulse)
		{
			maxImpulseMask = 0xFFFFFFFF;
			maxImpulses = (Vec4V*)currPtr;
			currPtr += sizeof(Vec4V) * numNormalConstr;
		}
		else
		{
			maxImpulses = &vMax;
		}

				
		PxcSolverFrictionSharedData4* PX_RESTRICT fd = (PxcSolverFrictionSharedData4*)currPtr;
		if(numFrictionConstr)
			currPtr += sizeof(PxcSolverFrictionSharedData4);

		Vec4V* frictionAppliedForce = (Vec4V*)currPtr;
		currPtr += sizeof(Vec4V)*numFrictionConstr;

		PxcSolverContactFrictionDynamic4* PX_RESTRICT frictions = (PxcSolverContactFrictionDynamic4*)currPtr;
		currPtr += numFrictionConstr * sizeof(PxcSolverContactFrictionDynamic4);

		if(hasTargetVel)
			currPtr += sizeof(Vec4V) * numFrictionConstr;
		
		Vec4V accumulatedNormalImpulse = vZero;

		const Vec4V invMass0D0 = hdr->invMassADom0;
		const Vec4V invMass1D1 = hdr->invMassBDom1;

		const Vec4V _normalT0 = hdr->normalX;
		const Vec4V _normalT1 = hdr->normalY;
		const Vec4V _normalT2 = hdr->normalZ;

		Vec4V _normalVel1 = V4Mul(linVel0T0, _normalT0);
		Vec4V _normalVel3 = V4Mul(linVel1T0, _normalT0);
		_normalVel1 = V4MulAdd(linVel0T1, _normalT1, _normalVel1);
		_normalVel3 = V4MulAdd(linVel1T1, _normalT1, _normalVel3);

		_normalVel1 = V4MulAdd(linVel0T2, _normalT2, _normalVel1);
		_normalVel3 = V4MulAdd(linVel1T2, _normalT2, _normalVel3);

		Vec4V accumDeltaF = vZero;

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContactBatchPointDynamic4& c = contacts[i];

#if 1//DO_PREFETCHES
			PxU32 offset = 0;
			Ps::prefetchLine(prefetchAddress, offset += 64);
			Ps::prefetchLine(prefetchAddress, offset += 64);
			Ps::prefetchLine(prefetchAddress, offset += 64);
			Ps::prefetchLine(prefetchAddress, offset += 64);
			prefetchAddress += offset;
#endif
			//Ps::prefetchLine((&contacts[i+2]), 384);

			const Vec4V appliedForce = appliedForces[i];
			const Vec4V maxImpulse = maxImpulses[i & maxImpulseMask];
			
			
			Vec4V normalVel2 = V4Mul(c.raXnX, angVel0T0);
			Vec4V normalVel4 = V4Mul(c.rbXnX, angVel1T0);

			normalVel2 = V4MulAdd(c.raXnY, angVel0T1, normalVel2);
			normalVel4 = V4MulAdd(c.rbXnY, angVel1T1, normalVel4);

			normalVel2 = V4MulAdd(c.raXnZ, angVel0T2, normalVel2);
			normalVel4 = V4MulAdd(c.rbXnZ, angVel1T2, normalVel4);

			//Linear component - normal * invMass_dom

			const Vec4V _normalVel(V4Add(_normalVel1, normalVel2));
			const Vec4V __normalVel(V4Add(_normalVel3, normalVel4));
		
			const Vec4V normalVel = V4Sub(_normalVel, __normalVel );

			Vec4V deltaF = V4NegMulSub(normalVel, c.velMultiplier, c.biasedErr);

			deltaF = V4Max(deltaF,  V4Neg(appliedForce));
			const Vec4V newAppliedForce = V4Min(V4Add(appliedForce, deltaF), maxImpulse);
			deltaF = V4Sub(newAppliedForce, appliedForce);

			accumDeltaF = V4Add(accumDeltaF, deltaF);

			_normalVel1 = V4MulAdd(invMass0D0, deltaF, _normalVel1);
			_normalVel3 = V4MulAdd(invMass1D1, deltaF, _normalVel3);
			
			angVel0T0 = V4MulAdd(c.delAngVel0X, deltaF, angVel0T0);
			angVel1T0 = V4MulAdd(c.delAngVel1X, deltaF, angVel1T0);
			
			angVel0T1 = V4MulAdd(c.delAngVel0Y, deltaF, angVel0T1);
			angVel1T1 = V4MulAdd(c.delAngVel1Y, deltaF, angVel1T1);

			angVel0T2 = V4MulAdd(c.delAngVel0Z, deltaF, angVel0T2);
			angVel1T2 = V4MulAdd(c.delAngVel1Z, deltaF, angVel1T2);

#if 1
			appliedForces[i] = newAppliedForce;
#endif

			accumulatedNormalImpulse = V4Add(accumulatedNormalImpulse, newAppliedForce);
		}

		const Vec4V accumDeltaF_IM0 = V4Mul(accumDeltaF, invMass0D0);
		const Vec4V accumDeltaF_IM1 = V4Mul(accumDeltaF, invMass1D1);

		linVel0T0 = V4MulAdd(_normalT0, accumDeltaF_IM0, linVel0T0);
		linVel1T0 = V4MulAdd(_normalT0, accumDeltaF_IM1, linVel1T0);
		linVel0T1 = V4MulAdd(_normalT1, accumDeltaF_IM0, linVel0T1);
		linVel1T1 = V4MulAdd(_normalT1, accumDeltaF_IM1, linVel1T1);
		linVel0T2 = V4MulAdd(_normalT2, accumDeltaF_IM0, linVel0T2);
		linVel1T2 = V4MulAdd(_normalT2, accumDeltaF_IM1, linVel1T2);


		if(cache.doFriction && numFrictionConstr)
		{
			const Vec4V staticFric = hdr->staticFriction;
			const Vec4V dynamicFric = hdr->dynamicFriction;

			const Vec4V maxFrictionImpulse = V4Mul(staticFric, accumulatedNormalImpulse);
			const Vec4V maxDynFrictionImpulse = V4Mul(dynamicFric, accumulatedNormalImpulse);
			const Vec4V negMaxDynFrictionImpulse = V4Neg(maxDynFrictionImpulse);
			//const Vec4V negMaxFrictionImpulse = V4Neg(maxFrictionImpulse);
			BoolV broken = fd->broken;

			if(cache.writeBackIteration)
			{
				Ps::prefetchLine(fd->frictionBrokenWritebackByte[0]);
				Ps::prefetchLine(fd->frictionBrokenWritebackByte[1]);
				Ps::prefetchLine(fd->frictionBrokenWritebackByte[2]);
				Ps::prefetchLine(fd->frictionBrokenWritebackByte[3]);
			}


			for(PxU32 i=0;i<numFrictionConstr;i++)
			{
				PxcSolverContactFrictionDynamic4& f = frictions[i];
#if 1//DO_PREFETCHES
				PxU32 offset = 0;
				Ps::prefetchLine(prefetchAddress, offset += 64);
				Ps::prefetchLine(prefetchAddress, offset += 64);
				Ps::prefetchLine(prefetchAddress, offset += 64);
				Ps::prefetchLine(prefetchAddress, offset += 64);
				prefetchAddress += offset;
#endif

				const Vec4V appliedForce = frictionAppliedForce[i];

				const Vec4V normalT0 = fd->normalX[i&1];
				const Vec4V normalT1 = fd->normalY[i&1];
				const Vec4V normalT2 = fd->normalZ[i&1];

				Vec4V normalVel1 = V4Mul(linVel0T0, normalT0);
				Vec4V normalVel2 = V4Mul(f.raXnX, angVel0T0);
				Vec4V normalVel3 = V4Mul(linVel1T0, normalT0);
				Vec4V normalVel4 = V4Mul(f.rbXnX, angVel1T0);

				normalVel1 = V4MulAdd(linVel0T1, normalT1, normalVel1);
				normalVel2 = V4MulAdd(f.raXnY, angVel0T1, normalVel2);
				normalVel3 = V4MulAdd(linVel1T1, normalT1, normalVel3);
				normalVel4 = V4MulAdd(f.rbXnY, angVel1T1, normalVel4);

				normalVel1 = V4MulAdd(linVel0T2, normalT2, normalVel1);
				normalVel2 = V4MulAdd(f.raXnZ, angVel0T2, normalVel2);
				normalVel3 = V4MulAdd(linVel1T2, normalT2, normalVel3);
				normalVel4 = V4MulAdd(f.rbXnZ, angVel1T2, normalVel4);

				const Vec4V _normalVel = V4Add(normalVel1, normalVel2);
				const Vec4V __normalVel = V4Add(normalVel3, normalVel4);

				// appliedForce -bias * velMultiplier - a hoisted part of the total impulse computation
			
				const Vec4V normalVel = V4Sub(_normalVel, __normalVel );

				const Vec4V tmp1 = V4Sub(appliedForce, f.scaledBias); 

				const Vec4V totalImpulse = V4NegMulSub(normalVel, f.velMultiplier, tmp1);
				
				broken = BOr(broken, V4IsGrtr(V4Abs(totalImpulse), maxFrictionImpulse));

				const Vec4V newAppliedForce = V4Min(maxDynFrictionImpulse, V4Max(negMaxDynFrictionImpulse, totalImpulse));

				const Vec4V deltaF =V4Sub(newAppliedForce, appliedForce);

#if 1
				frictionAppliedForce[i] = newAppliedForce;
#endif

				const Vec4V deltaFIM0 = V4Mul(deltaF, invMass0D0);
				const Vec4V deltaFIM1 = V4Mul(deltaF, invMass1D1);

				linVel0T0 = V4MulAdd(normalT0, deltaFIM0, linVel0T0);
				linVel1T0 = V4MulAdd(normalT0, deltaFIM1, linVel1T0);
				angVel0T0 = V4MulAdd(f.delAngVel0X, deltaF, angVel0T0);
				angVel1T0 = V4MulAdd(f.delAngVel1X, deltaF, angVel1T0);

				linVel0T1 = V4MulAdd(normalT1, deltaFIM0, linVel0T1);
				linVel1T1 = V4MulAdd(normalT1, deltaFIM1, linVel1T1);
				angVel0T1 = V4MulAdd(f.delAngVel0Y, deltaF, angVel0T1);
				angVel1T1 = V4MulAdd(f.delAngVel1Y, deltaF, angVel1T1);

				linVel0T2 = V4MulAdd(normalT2, deltaFIM0, linVel0T2);
				linVel1T2 = V4MulAdd(normalT2, deltaFIM1, linVel1T2);
				angVel0T2 = V4MulAdd(f.delAngVel0Z, deltaF, angVel0T2);
				angVel1T2 = V4MulAdd(f.delAngVel1Z, deltaF, angVel1T2);
			}
#if 1
			fd->broken = broken;
#endif
		}
	}

	PX_TRANSPOSE_44(linVel0T0, linVel0T1, linVel0T2, linVel0T3, linVel00, linVel10, linVel20, linVel30);
	PX_TRANSPOSE_44(linVel1T0, linVel1T1, linVel1T2, linVel1T3, linVel01, linVel11, linVel21, linVel31);
	PX_TRANSPOSE_44(angVel0T0, angVel0T1, angVel0T2, angVel0T3, angVel00, angVel10, angVel20, angVel30);
	PX_TRANSPOSE_44(angVel1T0, angVel1T1, angVel1T2, angVel1T3, angVel01, angVel11, angVel21, angVel31);

	// Write back
	V4StoreA(linVel00, &b00.linearVelocity.x);
	V4StoreA(angVel00, &b00.angularVelocity.x);
	V4StoreA(linVel10, &b10.linearVelocity.x);
	V4StoreA(angVel10, &b10.angularVelocity.x);
	V4StoreA(linVel20, &b20.linearVelocity.x);
	V4StoreA(angVel20, &b20.angularVelocity.x);
	V4StoreA(linVel30, &b30.linearVelocity.x);
	V4StoreA(angVel30, &b30.angularVelocity.x);

	if(desc[0].bodyBDataIndex != 0)
	{
		V4StoreA(linVel01, &b01.linearVelocity.x);
		V4StoreA(angVel01, &b01.angularVelocity.x);
	}
	if(desc[1].bodyBDataIndex != 0)
	{
		V4StoreA(linVel11, &b11.linearVelocity.x);
		V4StoreA(angVel11, &b11.angularVelocity.x);
	}
	if(desc[2].bodyBDataIndex != 0)
	{
		V4StoreA(linVel21, &b21.linearVelocity.x);
		V4StoreA(angVel21, &b21.angularVelocity.x);
	}
	if(desc[3].bodyBDataIndex != 0)
	{
		V4StoreA(linVel31, &b31.linearVelocity.x);
		V4StoreA(angVel31, &b31.angularVelocity.x);
	}
}

static void solveContact4_StaticBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& cache)
{
	PxcSolverBody& b00 = *desc[0].bodyA;
	PxcSolverBody& b10 = *desc[1].bodyA;
	PxcSolverBody& b20 = *desc[2].bodyA;
	PxcSolverBody& b30 = *desc[3].bodyA;

	const PxU8* PX_RESTRICT last = desc[0].constraint + getConstraintLength(desc[0]);

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc[0].constraint;


	//We'll need this.
	const Vec4V vZero	= V4Zero();
	Vec4V vMax	= V4Splat(FMax());
	
	Vec4V linVel00 = V4LoadA(&b00.linearVelocity.x);
	Vec4V angVel00 = V4LoadA(&b00.angularVelocity.x);

	Vec4V linVel10 = V4LoadA(&b10.linearVelocity.x);
	Vec4V angVel10 = V4LoadA(&b10.angularVelocity.x);

	Vec4V linVel20 = V4LoadA(&b20.linearVelocity.x);
	Vec4V angVel20 = V4LoadA(&b20.angularVelocity.x);

	Vec4V linVel30 = V4LoadA(&b30.linearVelocity.x);
	Vec4V angVel30 = V4LoadA(&b30.angularVelocity.x);

	Vec4V linVel0T0, linVel0T1, linVel0T2, linVel0T3;
	Vec4V angVel0T0, angVel0T1, angVel0T2, angVel0T3;


	PX_TRANSPOSE_44(linVel00, linVel10, linVel20, linVel30, linVel0T0, linVel0T1, linVel0T2, linVel0T3);
	PX_TRANSPOSE_44(angVel00, angVel10, angVel20, angVel30, angVel0T0, angVel0T1, angVel0T2, angVel0T3);

#if 1//DO_PREFETCHES
	const PxU8* PX_RESTRICT prefetchAddress = currPtr + sizeof(PxcSolverContactHeader4) + sizeof(PxcSolverContactBatchPointBase4);
#endif
	while((currPtr < last))
	{
		PxcSolverContactHeader4* PX_RESTRICT hdr = (PxcSolverContactHeader4*)currPtr;
		
		currPtr = (PxU8*)(hdr + 1);		

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;
		bool hasMaxImpulse = (hdr->flag & PxcSolverContactHeader4::eHAS_MAX_IMPULSE) != 0;
		bool hasTargetVel = (hdr->flag & PxcSolverContactHeader4::eHAS_TARGET_VELOCITY) != 0;

		Vec4V* appliedForces = (Vec4V*)currPtr;
		currPtr += sizeof(Vec4V)*numNormalConstr;

		PxcSolverContactBatchPointBase4* PX_RESTRICT contacts = (PxcSolverContactBatchPointBase4*)currPtr;

		currPtr = (PxU8*)(contacts + numNormalConstr);

		Vec4V* maxImpulses;
		PxU32 maxImpulseMask;
		if(hasMaxImpulse)
		{
			maxImpulseMask = 0xFFFFFFFF;
			maxImpulses = (Vec4V*)currPtr;
			currPtr += sizeof(Vec4V) * numNormalConstr;
		}
		else
		{
			maxImpulseMask = 0;
			maxImpulses = &vMax;
		}

		PxcSolverFrictionSharedData4* PX_RESTRICT fd = (PxcSolverFrictionSharedData4*)currPtr;
		if(numFrictionConstr)
			currPtr += sizeof(PxcSolverFrictionSharedData4);

		Vec4V* frictionAppliedForces = (Vec4V*)currPtr;
		currPtr += sizeof(Vec4V)*numFrictionConstr;

		PxcSolverContactFrictionBase4* PX_RESTRICT frictions = (PxcSolverContactFrictionBase4*)currPtr;
		currPtr += numFrictionConstr * sizeof(PxcSolverContactFrictionBase4);

		if(hasTargetVel)
			currPtr += sizeof(Vec4V) * numFrictionConstr;

		
		Vec4V accumulatedNormalImpulse = vZero;

		const Vec4V invMass0 = hdr->invMassADom0;

		const Vec4V _normalT0 = hdr->normalX;
		const Vec4V _normalT1 = hdr->normalY;
		const Vec4V _normalT2 = hdr->normalZ;

		const Vec4V __normalVel1 = V4Mul(linVel0T0, _normalT0);
		const Vec4V _normalVel1 = V4MulAdd(linVel0T1, _normalT1, __normalVel1);

		Vec4V nVel1 = V4MulAdd(linVel0T2, _normalT2, _normalVel1);

		Vec4V accumDeltaF = vZero;


		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContactBatchPointBase4& c = contacts[i];
#if 1//DO_PREFETCHES
			PxU32 offset = 0;
			Ps::prefetchLine(prefetchAddress, offset += 64);
			Ps::prefetchLine(prefetchAddress, offset += 64);
			Ps::prefetchLine(prefetchAddress, offset += 64);
			prefetchAddress += offset;
#endif		
			const Vec4V appliedForce = appliedForces[i];
			const Vec4V maxImpulse = maxImpulses[i&maxImpulseMask];
			
			Vec4V normalVel2 = V4Mul(c.raXnX, angVel0T0);
			normalVel2 = V4MulAdd(c.raXnY, angVel0T1, normalVel2);
			normalVel2 = V4MulAdd(c.raXnZ, angVel0T2, normalVel2);

			//NormalVel stores relative normal velocity for all 4 constraints

			//Linear component - normal * invMass_dom

			const Vec4V normalVel = V4Add(nVel1, normalVel2);

			const Vec4V _deltaF = V4Max(V4NegMulSub(normalVel, c.velMultiplier, c.biasedErr), V4Neg(appliedForce));

			Vec4V newAppliedForce(V4Add(appliedForce, _deltaF));
			newAppliedForce = V4Min(newAppliedForce, maxImpulse);
			const Vec4V deltaF = V4Sub(newAppliedForce, appliedForce);

			accumDeltaF = V4Add(accumDeltaF, deltaF);

			nVel1 = V4MulAdd(invMass0, deltaF, nVel1);
			angVel0T0 = V4MulAdd(c.delAngVel0X, deltaF, angVel0T0);
			angVel0T1 = V4MulAdd(c.delAngVel0Y, deltaF, angVel0T1);
			angVel0T2 = V4MulAdd(c.delAngVel0Z, deltaF, angVel0T2);
			
#if 1
			appliedForces[i] = newAppliedForce;
#endif
			
			accumulatedNormalImpulse = V4Add(accumulatedNormalImpulse, newAppliedForce);
		}	

		const Vec4V deltaFInvMass0 = V4Mul(accumDeltaF, invMass0);

		linVel0T0 = V4MulAdd(_normalT0, deltaFInvMass0, linVel0T0);
		linVel0T1 = V4MulAdd(_normalT1, deltaFInvMass0, linVel0T1);
		linVel0T2 = V4MulAdd(_normalT2, deltaFInvMass0, linVel0T2);

		if(cache.doFriction && numFrictionConstr)
		{
			const Vec4V staticFric = hdr->staticFriction;

			const Vec4V dynamicFric = hdr->dynamicFriction;

			const Vec4V maxFrictionImpulse = V4Mul(staticFric, accumulatedNormalImpulse);
			const Vec4V maxDynFrictionImpulse = V4Mul(dynamicFric, accumulatedNormalImpulse);
			const Vec4V negMaxDynFrictionImpulse = V4Neg(maxDynFrictionImpulse);
		//	const Vec4V negMaxFrictionImpulse = V4Neg(maxFrictionImpulse);

			BoolV broken = fd->broken;

			if(cache.writeBackIteration)
			{
				Ps::prefetchLine(fd->frictionBrokenWritebackByte[0]);
				Ps::prefetchLine(fd->frictionBrokenWritebackByte[1]);
				Ps::prefetchLine(fd->frictionBrokenWritebackByte[2]);
				Ps::prefetchLine(fd->frictionBrokenWritebackByte[3]);
			}

			for(PxU32 i=0;i<numFrictionConstr;i++)
			{
				PxcSolverContactFrictionBase4& f = frictions[i];
#if 1//DO_PREFETCHES
				PxU32 offset = 0;
				Ps::prefetchLine(prefetchAddress, offset += 64);
				Ps::prefetchLine(prefetchAddress, offset += 64);
				Ps::prefetchLine(prefetchAddress, offset += 64);
				prefetchAddress += offset;
#endif	
				const Vec4V appliedForce = frictionAppliedForces[i];

				const Vec4V normalT0 = fd->normalX[i&1];
				const Vec4V normalT1 = fd->normalY[i&1];
				const Vec4V normalT2 = fd->normalZ[i&1];

				Vec4V normalVel1 = V4Mul(linVel0T0, normalT0);
				Vec4V normalVel2 = V4Mul(f.raXnX, angVel0T0);

				normalVel1 = V4MulAdd(linVel0T1, normalT1, normalVel1);
				normalVel2 = V4MulAdd(f.raXnY, angVel0T1, normalVel2);

				normalVel1 = V4MulAdd(linVel0T2, normalT2, normalVel1);
				normalVel2 = V4MulAdd(f.raXnZ, angVel0T2, normalVel2);

				//relative normal velocity for all 4 constraints
				const Vec4V normalVel = V4Add(normalVel1, normalVel2);

				// appliedForce -bias * velMultiplier - a hoisted part of the total impulse computation
				const Vec4V tmp1 = V4Sub(appliedForce, f.scaledBias); 

				const Vec4V totalImpulse = V4NegMulSub(normalVel, f.velMultiplier, tmp1);

				broken = BOr(broken, V4IsGrtr(V4Abs(totalImpulse), maxFrictionImpulse));

				const Vec4V newAppliedForce = V4Min(maxDynFrictionImpulse, V4Max(negMaxDynFrictionImpulse, totalImpulse));

				const Vec4V deltaF =V4Sub(newAppliedForce, appliedForce);

				const Vec4V deltaFInvMass = V4Mul(invMass0, deltaF);

				linVel0T0 = V4MulAdd(normalT0, deltaFInvMass, linVel0T0);
				angVel0T0 = V4MulAdd(f.delAngVel0X, deltaF, angVel0T0);

				linVel0T1 = V4MulAdd(normalT1, deltaFInvMass, linVel0T1);
				angVel0T1 = V4MulAdd(f.delAngVel0Y, deltaF, angVel0T1);

				linVel0T2 = V4MulAdd(normalT2, deltaFInvMass, linVel0T2);
				angVel0T2 = V4MulAdd(f.delAngVel0Z, deltaF, angVel0T2);

#if 1
				frictionAppliedForces[i] = newAppliedForce;
#endif

			}
#if 1
			fd->broken = broken;
#endif
		}
	}

	PX_TRANSPOSE_44(linVel0T0, linVel0T1, linVel0T2, linVel0T3, linVel00, linVel10, linVel20, linVel30);
	PX_TRANSPOSE_44(angVel0T0, angVel0T1, angVel0T2, angVel0T3, angVel00, angVel10, angVel20, angVel30);

	// Write back
	V4StoreA(linVel00, &b00.linearVelocity.x);
	V4StoreA(linVel10, &b10.linearVelocity.x);
	V4StoreA(linVel20, &b20.linearVelocity.x);
	V4StoreA(linVel30, &b30.linearVelocity.x);

	V4StoreA(angVel00, &b00.angularVelocity.x);
	V4StoreA(angVel10, &b10.angularVelocity.x);
	V4StoreA(angVel20, &b20.angularVelocity.x);
	V4StoreA(angVel30, &b30.angularVelocity.x);
}

static void concludeContact4_Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& /*cache*/, PxU32 contactSize, PxU32 frictionSize)
{
	const PxU8* PX_RESTRICT last = desc[0].constraint + getConstraintLength(desc[0]);

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc[0].constraint;

	const Vec4V zeroV = V4Zero();

	while((currPtr < last))
	{
		PxcSolverContactHeader4* PX_RESTRICT hdr = (PxcSolverContactHeader4*)currPtr;
		
		currPtr = (PxU8*)(hdr + 1);		

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		currPtr += sizeof(Vec4V)*numNormalConstr;

		PxcSolverContactBatchPointBase4* PX_RESTRICT contacts = (PxcSolverContactBatchPointBase4*)currPtr;
		currPtr += (numNormalConstr * contactSize);
		bool hasMaxImpulse = (hdr->flag & PxcSolverContactHeader4::eHAS_MAX_IMPULSE) != 0;
		bool hasTargetVel = (hdr->flag & PxcSolverContactHeader4::eHAS_TARGET_VELOCITY) != 0;

		if(hasMaxImpulse)
			currPtr += sizeof(Vec4V) * numNormalConstr;

		currPtr += sizeof(Vec4V)*numFrictionConstr;

		PxcSolverFrictionSharedData4* PX_RESTRICT fd = (PxcSolverFrictionSharedData4*)currPtr;
		if(numFrictionConstr)
			currPtr += sizeof(PxcSolverFrictionSharedData4);
		PX_UNUSED(fd);

		PxcSolverContactFrictionBase4* PX_RESTRICT frictions = (PxcSolverContactFrictionBase4*)currPtr;
		currPtr += (numFrictionConstr * frictionSize);

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContactBatchPointBase4& c = *contacts;
			contacts = (PxcSolverContactBatchPointBase4*)(((PxU8*)contacts) + contactSize);
			//c.biasedErr = V4Min(c.biasedErr, vZero);
			c.biasedErr = V4Sub(c.biasedErr, c.scaledBias);
		}	

		if(hasTargetVel)
		{
			Vec4V* targetVel = (Vec4V*)currPtr;
			currPtr += sizeof(Vec4V) * numFrictionConstr;
			for(PxU32 i=0;i<numFrictionConstr;i++)
			{
				PxcSolverContactFrictionBase4& f = *frictions;
				frictions = (PxcSolverContactFrictionBase4*)(((PxU8*)frictions) + frictionSize);
				f.scaledBias = *targetVel++;
			}
		}
		else
		{
			for(PxU32 i=0;i<numFrictionConstr;i++)
			{
				PxcSolverContactFrictionBase4& f = *frictions;
				frictions = (PxcSolverContactFrictionBase4*)(((PxU8*)frictions) + frictionSize);
				f.scaledBias = zeroV;
			}
		}
	}
}

void writeBackContact4_Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& cache,
							 const PxcSolverBodyData** PX_RESTRICT bd0, const PxcSolverBodyData** PX_RESTRICT bd1)
{
	const PxU8* PX_RESTRICT last = desc[0].constraint + getConstraintLength(desc[0]);

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc[0].constraint;
	PxReal* PX_RESTRICT vForceWriteback0 = reinterpret_cast<PxReal*>(desc[0].writeBack);
	PxReal* PX_RESTRICT vForceWriteback1 = reinterpret_cast<PxReal*>(desc[1].writeBack);
	PxReal* PX_RESTRICT vForceWriteback2 = reinterpret_cast<PxReal*>(desc[2].writeBack);
	PxReal* PX_RESTRICT vForceWriteback3 = reinterpret_cast<PxReal*>(desc[3].writeBack);

	const PxU8 type = *desc[0].constraint;
	const PxU32 contactSize = type == PXS_SC_TYPE_BLOCK_RB_CONTACT ? sizeof(PxcSolverContactBatchPointDynamic4) : sizeof(PxcSolverContactBatchPointBase4);
	const PxU32 frictionSize = type == PXS_SC_TYPE_BLOCK_RB_CONTACT ? sizeof(PxcSolverContactFrictionDynamic4) : sizeof(PxcSolverContactFrictionBase4);


	Vec4V normalForce = V4Zero();


	//We'll need this.
	//const Vec4V vZero	= V4Zero();

	bool writeBackThresholds[4] = {false, false, false, false};

	while((currPtr < last))
	{
		PxcSolverContactHeader4* PX_RESTRICT hdr = (PxcSolverContactHeader4*)currPtr;
		
		currPtr = (PxU8*)(hdr + 1);		

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		Vec4V* PX_RESTRICT appliedForces = (Vec4V*)currPtr;
		currPtr += sizeof(Vec4V)*numNormalConstr;

		//PxcSolverContactBatchPointBase4* PX_RESTRICT contacts = (PxcSolverContactBatchPointBase4*)currPtr;
		currPtr += (numNormalConstr * contactSize);

		bool hasMaxImpulse = (hdr->flag & PxcSolverContactHeader4::eHAS_MAX_IMPULSE) != 0;
		bool hasTargetVel = (hdr->flag & PxcSolverContactHeader4::eHAS_TARGET_VELOCITY) != 0;

		if(hasMaxImpulse)
			currPtr += sizeof(Vec4V) * numNormalConstr;

		PxcSolverFrictionSharedData4* PX_RESTRICT fd = (PxcSolverFrictionSharedData4*)currPtr;
		if(numFrictionConstr)
			currPtr += sizeof(PxcSolverFrictionSharedData4);

		currPtr += sizeof(Vec4V)*numFrictionConstr;

		//PxcSolverContactFrictionBase4* PX_RESTRICT frictions = (PxcSolverContactFrictionBase4*)currPtr;
		currPtr += (numFrictionConstr * frictionSize);
		if(hasTargetVel)
			currPtr += sizeof(Vec4V) * numFrictionConstr;

		writeBackThresholds[0] = hdr->flags[0] & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;
		writeBackThresholds[1] = hdr->flags[1] & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;
		writeBackThresholds[2] = hdr->flags[2] & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;
		writeBackThresholds[3] = hdr->flags[3] & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;


		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			//contacts = (PxcSolverContactBatchPointBase4*)(((PxU8*)contacts) + contactSize);
			const FloatV appliedForce0 = V4GetX(appliedForces[i]);
			const FloatV appliedForce1 = V4GetY(appliedForces[i]);
			const FloatV appliedForce2 = V4GetZ(appliedForces[i]);
			const FloatV appliedForce3 = V4GetW(appliedForces[i]);

			normalForce = V4Add(normalForce, appliedForces[i]);

			if(vForceWriteback0 && i < hdr->numNormalConstr0)
				FStore(appliedForce0, vForceWriteback0++);
			if(vForceWriteback1 && i < hdr->numNormalConstr1)
				FStore(appliedForce1, vForceWriteback1++);
			if(vForceWriteback2 && i < hdr->numNormalConstr2)
				FStore(appliedForce2, vForceWriteback2++);
			if(vForceWriteback3 && i < hdr->numNormalConstr3)
				FStore(appliedForce3, vForceWriteback3++);
		}	

		if(numFrictionConstr)
		{
			PX_ALIGN(16, PxU32 broken[4]);
			BStoreA(fd->broken, broken);

			PxU8* frictionCounts = &hdr->numFrictionConstr0;

			for(PxU32 a = 0; a < 4; ++a)
			{
				if(frictionCounts[a] && broken[a])
					*fd->frictionBrokenWritebackByte[a] = 1;	// PT: bad L2 miss here
			}
		}
	}

	PX_ALIGN(16, PxReal nf[4]);
	V4StoreA(normalForce, nf);

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


static void solve1D4_Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& /*cache*/)
{

	PxcSolverBody& b00 = *desc[0].bodyA;
	PxcSolverBody& b01 = *desc[0].bodyB;

	PxcSolverBody& b10 = *desc[1].bodyA;
	PxcSolverBody& b11 = *desc[1].bodyB;

	PxcSolverBody& b20 = *desc[2].bodyA;
	PxcSolverBody& b21 = *desc[2].bodyB;

	PxcSolverBody& b30 = *desc[3].bodyA;
	PxcSolverBody& b31 = *desc[3].bodyB;

	PxU8* PX_RESTRICT bPtr = desc[0].constraint;
	//PxU32 length = desc.constraintLength;

	const PxcSolverConstraint1DHeader4* PX_RESTRICT  header = (const PxcSolverConstraint1DHeader4*)(bPtr);
	PxcSolverConstraint1DDynamic4* PX_RESTRICT base = (PxcSolverConstraint1DDynamic4*)(header+1);

	//const FloatV fZero = FZero();
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


	Vec4V linVel0T0, linVel0T1, linVel0T2, linVel0T3;
	Vec4V linVel1T0, linVel1T1, linVel1T2, linVel1T3;
	Vec4V angVel0T0, angVel0T1, angVel0T2, angVel0T3;
	Vec4V angVel1T0, angVel1T1, angVel1T2, angVel1T3;


	PX_TRANSPOSE_44(linVel00, linVel10, linVel20, linVel30, linVel0T0, linVel0T1, linVel0T2, linVel0T3);
	PX_TRANSPOSE_44(linVel01, linVel11, linVel21, linVel31, linVel1T0, linVel1T1, linVel1T2, linVel1T3);
	PX_TRANSPOSE_44(angVel00, angVel10, angVel20, angVel30, angVel0T0, angVel0T1, angVel0T2, angVel0T3);
	PX_TRANSPOSE_44(angVel01, angVel11, angVel21, angVel31, angVel1T0, angVel1T1, angVel1T2, angVel1T3);

	const Vec4V	invMass0D0 = header->invMass0Dom0;
	const Vec4V	invMass1D1 = header->invMass1Dom1;

	PxU32 maxConstraints = header->count;

	for(PxU32 a = 0; a < maxConstraints; ++a)
	{
		PxcSolverConstraint1DDynamic4& c = *base;
		base++;
#if DO_PREFETCHES
		Ps::prefetchLine(base);
		Ps::prefetchLine(base,128);
		Ps::prefetchLine(base,256);
		Ps::prefetchLine(base,384);
#endif
		
		const Vec4V appliedForce = c.appliedForce;

		//Everything above this point is just standard SOA loading code!

		//transpose clinVel/cangVel to enable us to use Multiply-adds

		Vec4V linProj0(V4Mul(c.lin0X, linVel0T0));
		Vec4V linProj1(V4Mul(c.lin1X, linVel1T0));
		Vec4V angProj0(V4Mul(c.ang0X, angVel0T0));
		Vec4V angProj1(V4Mul(c.ang1X, angVel1T0));

		linProj0 = (V4MulAdd(c.lin0Y, linVel0T1, linProj0));
		linProj1 = (V4MulAdd(c.lin1Y, linVel1T1, linProj1));
		angProj0 = (V4MulAdd(c.ang0Y, angVel0T1, angProj0));
		angProj1 = (V4MulAdd(c.ang1Y, angVel1T1, angProj1));
		
		linProj0 = (V4MulAdd(c.lin0Z, linVel0T2, linProj0));
		linProj1 = (V4MulAdd(c.lin1Z, linVel1T2, linProj1));
		angProj0 = (V4MulAdd(c.ang0Z, angVel0T2, angProj0));
		angProj1 = (V4MulAdd(c.ang1Z, angVel1T2, angProj1));

		const Vec4V projectVel0(V4Add(linProj0, angProj0));
		const Vec4V projectVel1(V4Add(linProj1, angProj1));
		
		const Vec4V normalVel(V4Sub(projectVel0, projectVel1));

		const Vec4V unclampedForce = V4MulAdd(appliedForce, c.impulseMultiplier, V4MulAdd(normalVel, c.velMultiplier, c.constant));
		const Vec4V clampedForce = V4Max(c.minImpulse, V4Min(c.maxImpulse, unclampedForce));
		const Vec4V deltaF = V4Sub(clampedForce, appliedForce);
		c.appliedForce = clampedForce;

		const Vec4V deltaFInvMass0 = V4Mul(deltaF, invMass0D0);
		const Vec4V deltaFInvMass1 = V4Mul(deltaF, invMass1D1);

		linVel0T0 = V4MulAdd(c.lin0X, deltaFInvMass0, linVel0T0);
		linVel1T0 = V4MulAdd(c.lin1X, deltaFInvMass1, linVel1T0);
		angVel0T0 = V4MulAdd(c.ang0InvInertiaX, deltaF, angVel0T0);
		angVel1T0 = V4MulAdd(c.ang1InvInertia1X, deltaF, angVel1T0);

		linVel0T1 = V4MulAdd(c.lin0Y, deltaFInvMass0, linVel0T1);
		linVel1T1 = V4MulAdd(c.lin1Y, deltaFInvMass1, linVel1T1);
		angVel0T1 = V4MulAdd(c.ang0InvInertiaY, deltaF, angVel0T1);
		angVel1T1 = V4MulAdd(c.ang1InvInertia1Y, deltaF, angVel1T1);

		linVel0T2 = V4MulAdd(c.lin0Z, deltaFInvMass0, linVel0T2);
		linVel1T2 = V4MulAdd(c.lin1Z, deltaFInvMass1, linVel1T2);
		angVel0T2 = V4MulAdd(c.ang0InvInertiaZ, deltaF, angVel0T2);
		angVel1T2 = V4MulAdd(c.ang1InvInertia1Z, deltaF, angVel1T2);
	}

	PX_TRANSPOSE_44(linVel0T0, linVel0T1, linVel0T2, linVel0T3, linVel00, linVel10, linVel20, linVel30);
	PX_TRANSPOSE_44(linVel1T0, linVel1T1, linVel1T2, linVel1T3, linVel01, linVel11, linVel21, linVel31);
	PX_TRANSPOSE_44(angVel0T0, angVel0T1, angVel0T2, angVel0T3, angVel00, angVel10, angVel20, angVel30);
	PX_TRANSPOSE_44(angVel1T0, angVel1T1, angVel1T2, angVel1T3, angVel01, angVel11, angVel21, angVel31);


	// Write back
	V4StoreA((linVel00), &b00.linearVelocity.x);
	V4StoreA((linVel10), &b10.linearVelocity.x);
	V4StoreA((linVel20), &b20.linearVelocity.x);
	V4StoreA((linVel30), &b30.linearVelocity.x);

	V4StoreA((linVel01), &b01.linearVelocity.x);
	V4StoreA((linVel11), &b11.linearVelocity.x);
	V4StoreA((linVel21), &b21.linearVelocity.x);
	V4StoreA((linVel31), &b31.linearVelocity.x);

	V4StoreA((angVel00), &b00.angularVelocity.x);
	V4StoreA((angVel10), &b10.angularVelocity.x);
	V4StoreA((angVel20), &b20.angularVelocity.x);
	V4StoreA((angVel30), &b30.angularVelocity.x);

	V4StoreA((angVel01), &b01.angularVelocity.x);
	V4StoreA((angVel11), &b11.angularVelocity.x);
	V4StoreA((angVel21), &b21.angularVelocity.x);
	V4StoreA((angVel31), &b31.angularVelocity.x);
	
}

static void conclude1D4_Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& /*cache*/)
{
	PxcSolverConstraint1DHeader4* header = reinterpret_cast<PxcSolverConstraint1DHeader4*>(desc[0].constraint);
	PxU8* base = desc[0].constraint + sizeof(PxcSolverConstraint1DHeader4);
	PxU32 stride = header->type == PXS_SC_TYPE_BLOCK_1D ? sizeof(PxcSolverConstraint1DDynamic4) : sizeof(PxcSolverConstraint1DBase4);

	for(PxU32 i=0; i<header->count; i++)
	{
		PxcSolverConstraint1DBase4& c = *reinterpret_cast<PxcSolverConstraint1DBase4*>(base);
		c.constant = c.unbiasedConstant;
		base += stride;
	}
	PX_ASSERT(desc[0].constraint + getConstraintLength(desc[0]) == base);
}

void writeBack1D4(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& /*cache*/,
							 const PxcSolverBodyData** PX_RESTRICT /*bd0*/, const PxcSolverBodyData** PX_RESTRICT /*bd1*/)
{
	PxsConstraintWriteback* writeback0 = reinterpret_cast<PxsConstraintWriteback*>(desc[0].writeBack);
	PxsConstraintWriteback* writeback1 = reinterpret_cast<PxsConstraintWriteback*>(desc[1].writeBack);
	PxsConstraintWriteback* writeback2 = reinterpret_cast<PxsConstraintWriteback*>(desc[2].writeBack);
	PxsConstraintWriteback* writeback3 = reinterpret_cast<PxsConstraintWriteback*>(desc[3].writeBack);

	if(writeback0 || writeback1 || writeback2 || writeback3)
	{
		PxcSolverConstraint1DHeader4* header = reinterpret_cast<PxcSolverConstraint1DHeader4*>(desc[0].constraint);
		PxU8* base = desc[0].constraint + sizeof(PxcSolverConstraint1DHeader4);
		PxU32 stride = header->type == PXS_SC_TYPE_BLOCK_1D ? sizeof(PxcSolverConstraint1DDynamic4) : sizeof(PxcSolverConstraint1DBase4);

		const Vec4V zero = V4Zero();
		Vec4V linX(zero), linY(zero), linZ(zero); 
		Vec4V angX(zero), angY(zero), angZ(zero); 

		for(PxU32 i=0; i<header->count; i++)
		{
			const PxcSolverConstraint1DBase4* c = reinterpret_cast<PxcSolverConstraint1DBase4*>(base);

			//Load in flags
			const VecI32V flags = I4LoadU((PxI32*)&c->flags[0]);
			//Work out masks
			const VecI32V mask = I4Load(PXS_SC_FLAG_OUTPUT_FORCE);

			const VecI32V masked = VecI32V_And(flags, mask);
			const BoolV isEq = VecI32V_IsEq(masked, mask);

			const Vec4V appliedForce = V4Sel(isEq, c->appliedForce, zero);

			linX = V4MulAdd(c->lin0X, appliedForce, linX);
			linY = V4MulAdd(c->lin0Y, appliedForce, linY);
			linZ = V4MulAdd(c->lin0Z, appliedForce, linZ);

			angX = V4MulAdd(c->ang0X, appliedForce, angX);
			angY = V4MulAdd(c->ang0Y, appliedForce, angY);
			angZ = V4MulAdd(c->ang0Z, appliedForce, angZ);

			base += stride;
		}

		//We need to do the cross product now

		angX = V4Sub(angX, V4NegMulSub(header->body0WorkOffsetZ, linY, V4Mul(header->body0WorkOffsetY, linZ)));
		angY = V4Sub(angY, V4NegMulSub(header->body0WorkOffsetX, linZ, V4Mul(header->body0WorkOffsetZ, linX)));
		angZ = V4Sub(angZ, V4NegMulSub(header->body0WorkOffsetY, linX, V4Mul(header->body0WorkOffsetX, linY)));

		const Vec4V linLenSq = V4MulAdd(linZ, linZ, V4MulAdd(linY, linY, V4Mul(linX, linX)));
		const Vec4V angLenSq = V4MulAdd(angZ, angZ, V4MulAdd(angY, angY, V4Mul(angX, angX)));

		const Vec4V linLen = V4Sqrt(linLenSq);
		const Vec4V angLen = V4Sqrt(angLenSq);

		const BoolV broken = BOr(V4IsGrtr(linLen, header->linBreakImpulse), V4IsGrtr(angLen, header->angBreakImpulse));

		PX_ALIGN(16, PxU32 iBroken[4]);
		BStoreA(broken, iBroken);


		Vec4V lin0, lin1, lin2, lin3;
		Vec4V ang0, ang1, ang2, ang3;

		PX_TRANSPOSE_34_44(linX, linY, linZ, lin0, lin1, lin2, lin3);
		PX_TRANSPOSE_34_44(angX, angY, angZ, ang0, ang1, ang2, ang3);

		if(writeback0)
		{
			V3StoreU(Vec3V_From_Vec4V_WUndefined(lin0), writeback0->linearImpulse);
			V3StoreU(Vec3V_From_Vec4V_WUndefined(ang0), writeback0->angularImpulse);
			writeback0->broken = PxU32(iBroken[0] != 0);
		}
		if(writeback1)
		{
			V3StoreU(Vec3V_From_Vec4V_WUndefined(lin1), writeback1->linearImpulse);
			V3StoreU(Vec3V_From_Vec4V_WUndefined(ang1), writeback1->angularImpulse);
			writeback1->broken = PxU32(iBroken[1] != 0);
		}
		if(writeback2)
		{
			V3StoreU(Vec3V_From_Vec4V_WUndefined(lin2), writeback2->linearImpulse);
			V3StoreU(Vec3V_From_Vec4V_WUndefined(ang2), writeback2->angularImpulse);
			writeback2->broken = PxU32(iBroken[2] != 0);
		}
		if(writeback3)
		{
			V3StoreU(Vec3V_From_Vec4V_WUndefined(lin3), writeback3->linearImpulse);
			V3StoreU(Vec3V_From_Vec4V_WUndefined(ang3), writeback3->angularImpulse);
			writeback3->broken = PxU32(iBroken[3] != 0);
		}

		PX_ASSERT(desc[0].constraint + getConstraintLength(desc[0]) == base);
	}
}


void solveContactPreBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 /*constraintCount*/, PxcSolverContext& cache)
{
	solveContact4_Block(desc, cache);
}

void solveContactPreBlock_Static(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveContact4_StaticBlock(desc, cache);
}

void solveContactPreBlock_Conclude(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveContact4_Block(desc, cache);
	concludeContact4_Block(desc, cache, sizeof(PxcSolverContactBatchPointDynamic4), sizeof(PxcSolverContactFrictionDynamic4));
}

void solveContactPreBlock_ConcludeStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solveContact4_StaticBlock(desc, cache);
	concludeContact4_Block(desc, cache, sizeof(PxcSolverContactBatchPointBase4), sizeof(PxcSolverContactFrictionBase4));
}

void solveContactPreBlock_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	solveContact4_Block(desc, cache);

	const PxcSolverBodyData* bd0[4] = {	&cache.solverBodyArray[desc[0].bodyADataIndex], 
										&cache.solverBodyArray[desc[1].bodyADataIndex],
										&cache.solverBodyArray[desc[2].bodyADataIndex],
										&cache.solverBodyArray[desc[3].bodyADataIndex]};

	const PxcSolverBodyData* bd1[4] = {	&cache.solverBodyArray[desc[0].bodyBDataIndex], 
										&cache.solverBodyArray[desc[1].bodyBDataIndex],
										&cache.solverBodyArray[desc[2].bodyBDataIndex],
										&cache.solverBodyArray[desc[3].bodyBDataIndex]};

	writeBackContact4_Block(desc, cache, bd0, bd1);

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

void contactPreBlock_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	const PxcSolverBodyData* bd0[4] = {	&cache.solverBodyArray[desc[0].bodyADataIndex], 
										&cache.solverBodyArray[desc[1].bodyADataIndex],
										&cache.solverBodyArray[desc[2].bodyADataIndex],
										&cache.solverBodyArray[desc[3].bodyADataIndex]};

	const PxcSolverBodyData* bd1[4] = {	&cache.solverBodyArray[desc[0].bodyBDataIndex], 
										&cache.solverBodyArray[desc[1].bodyBDataIndex],
										&cache.solverBodyArray[desc[2].bodyBDataIndex],
										&cache.solverBodyArray[desc[3].bodyBDataIndex]};

	writeBackContact4_Block(desc, cache, bd0, bd1);

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

void solveContactPreBlock_WriteBackStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	solveContact4_StaticBlock(desc, cache);
	const PxcSolverBodyData* bd0[4] = {	&cache.solverBodyArray[desc[0].bodyADataIndex], 
										&cache.solverBodyArray[desc[1].bodyADataIndex],
										&cache.solverBodyArray[desc[2].bodyADataIndex],
										&cache.solverBodyArray[desc[3].bodyADataIndex]};

	const PxcSolverBodyData* bd1[4] = {	&cache.solverBodyArray[desc[0].bodyBDataIndex], 
										&cache.solverBodyArray[desc[1].bodyBDataIndex],
										&cache.solverBodyArray[desc[2].bodyBDataIndex],
										&cache.solverBodyArray[desc[3].bodyBDataIndex]};

	writeBackContact4_Block(desc, cache, bd0, bd1);

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

void solve1D4_Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solve1D4_Block(desc, cache);
}


void solve1D4Block_Conclude(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache)
{
	solve1D4_Block(desc, cache);
	conclude1D4_Block(desc, cache);
}


void solve1D4Block_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	solve1D4_Block(desc, cache);

	const PxcSolverBodyData* bd0[4] = {	&cache.solverBodyArray[desc[0].bodyADataIndex], 
										&cache.solverBodyArray[desc[1].bodyADataIndex],
										&cache.solverBodyArray[desc[2].bodyADataIndex],
										&cache.solverBodyArray[desc[3].bodyADataIndex]};

	const PxcSolverBodyData* bd1[4] = {	&cache.solverBodyArray[desc[0].bodyBDataIndex], 
										&cache.solverBodyArray[desc[1].bodyBDataIndex],
										&cache.solverBodyArray[desc[2].bodyBDataIndex],
										&cache.solverBodyArray[desc[3].bodyBDataIndex]};

	writeBack1D4(desc, cache, bd0, bd1);
}

void writeBack1D4Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32  /*constraintCount*/, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	const PxcSolverBodyData* bd0[4] = {	&cache.solverBodyArray[desc[0].bodyADataIndex], 
										&cache.solverBodyArray[desc[1].bodyADataIndex],
										&cache.solverBodyArray[desc[2].bodyADataIndex],
										&cache.solverBodyArray[desc[3].bodyADataIndex]};

	const PxcSolverBodyData* bd1[4] = {	&cache.solverBodyArray[desc[0].bodyBDataIndex], 
										&cache.solverBodyArray[desc[1].bodyBDataIndex],
										&cache.solverBodyArray[desc[2].bodyBDataIndex],
										&cache.solverBodyArray[desc[3].bodyBDataIndex]};

	writeBack1D4(desc, cache, bd0, bd1);
}

}

#endif
