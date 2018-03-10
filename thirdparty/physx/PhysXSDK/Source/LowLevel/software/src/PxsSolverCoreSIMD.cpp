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

//DE2387: workaround for failing unit tests on ARM Android devices when SIMD version of solve1D is used. (NDK bug, neon needs fixed gcc as well)
#if !defined(PX_ARM) || defined(PX_ARM_NEON)
#define PX_SIMD_SOLVE1D
#endif

namespace physx
{

#if !defined (PX_SIMD_SOLVE1D)
//Original scalar implementation of Solve1D
void solve1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	PxcSolverBody& b0 = *desc.bodyA;
	PxcSolverBody& b1 = *desc.bodyB;

	{
		PxU8* PX_RESTRICT bPtr = desc.constraint;
		//PxU32 length = desc.constraintLength;

		const PxcSolverConstraint1DHeader* PX_RESTRICT  header = reinterpret_cast<const PxcSolverConstraint1DHeader* PX_RESTRICT >(bPtr);
		PxcSolverConstraint1D* PX_RESTRICT const base = reinterpret_cast<PxcSolverConstraint1D* PX_RESTRICT >(bPtr + sizeof(PxcSolverConstraint1DHeader));

		for(PxU32 i=0; i<header->count;++i)
		{
			PxcSolverConstraint1D& c = base[i];

			const PxReal normalVel =  b0.projectVelocity(c.lin0, c.ang0) 
									- b1.projectVelocity(c.lin1, c.ang1);

			PxReal unclampedForce = c.constant + c.velMultiplier * normalVel + c.impulseMultiplier * c.appliedForce;
			PxReal newForce = PxMin(c.maxImpulse, PxMax(c.minImpulse, unclampedForce));

			PxReal deltaF = newForce - c.appliedForce;
			c.appliedForce = newForce;

			const PxVec3 linTemp0 = c.lin0 * c.invMass0;
			const PxVec3 linTemp1 = c.lin1 * c.invMass1;
			
			b0.linearVelocity += linTemp0 * deltaF;
			b1.linearVelocity += linTemp1 * deltaF;
			b0.angularVelocity += c.ang0InvInertia * deltaF;
			b1.angularVelocity += c.ang1InvInertia * deltaF;

		}
	}
}
#else


//Port of scalar implementation to SIMD maths with some interleaving of instructions
void solve1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	PX_UNUSED(cache);
	PxcSolverBody& b0 = *desc.bodyA;
	PxcSolverBody& b1 = *desc.bodyB;

	PxU8* PX_RESTRICT bPtr = desc.constraint;
	//PxU32 length = desc.constraintLength;

	const PxcSolverConstraint1DHeader* PX_RESTRICT  header = reinterpret_cast<const PxcSolverConstraint1DHeader*>(bPtr);
	PxcSolverConstraint1D* PX_RESTRICT base = reinterpret_cast<PxcSolverConstraint1D*>(bPtr + sizeof(PxcSolverConstraint1DHeader));

	Vec3V linVel0 = V3LoadA(b0.linearVelocity);
	Vec3V linVel1 = V3LoadA(b1.linearVelocity);
	Vec3V angVel0 = V3LoadA(b0.angularVelocity);
	Vec3V angVel1 = V3LoadA(b1.angularVelocity);

	for(PxU32 i=0; i<header->count;++i, base++)
	{
		Ps::prefetchLine(base+1);
		PxcSolverConstraint1D& c = *base;

		const Vec3V clinVel0 = V3LoadA(c.lin0);
		const Vec3V clinVel1 = V3LoadA(c.lin1);
		const Vec3V cangVel0 = V3LoadA(c.ang0);
		const Vec3V cangVel1 = V3LoadA(c.ang1);
		
		const Vec3V ang0InvInertia = V3LoadA(c.ang0InvInertia);
		const Vec3V ang1InvInertia = V3LoadA(c.ang1InvInertia);
		const FloatV invMass0D0 = FLoad(c.invMass0);
		const FloatV invMass1D1 = FLoad(c.invMass1);

		const FloatV constant = FLoad(c.constant);
		const FloatV vMul = FLoad(c.velMultiplier);
		const FloatV iMul = FLoad(c.impulseMultiplier);
		const FloatV appliedForce = FLoad(c.appliedForce);
		
		const FloatV maxImpulse = FLoad(c.maxImpulse);
		const FloatV minImpulse = FLoad(c.minImpulse);

		const FloatV linProj0 = V3Dot(linVel0, clinVel0);
		const FloatV angProj0 = V3Dot(angVel0, cangVel0);
		const FloatV linProj1 = V3Dot(linVel1, clinVel1);
		const FloatV angProj1 = V3Dot(angVel1, cangVel1);

		const FloatV projectVel0 = FAdd(linProj0, angProj0);
		const FloatV projectVel1 = FAdd(linProj1, angProj1);

		const FloatV normalVel = FSub(projectVel0, projectVel1);
		const FloatV unclampedForce = FMulAdd(iMul, appliedForce, FMulAdd(vMul, normalVel, constant));
		const FloatV clampedForce = FMin(maxImpulse, (FMax(minImpulse, unclampedForce)));
		const FloatV deltaF = FSub(clampedForce, appliedForce);
		
		const Vec3V linTemp0 = V3Scale(clinVel0, invMass0D0);
		const Vec3V linTemp1 = V3Scale(clinVel1, invMass1D1);
		
		c.appliedForce = FStore(clampedForce);
		linVel0 = V3ScaleAdd(linTemp0, deltaF, linVel0);			
		linVel1 = V3ScaleAdd(linTemp1, deltaF, linVel1);
		angVel0 = V3ScaleAdd(ang0InvInertia, deltaF, angVel0);
		angVel1 = V3ScaleAdd(ang1InvInertia, deltaF, angVel1);

	}

	V3StoreA(linVel0, b0.linearVelocity);
	V3StoreA(angVel0, b0.angularVelocity);
	V3StoreA(linVel1, b1.linearVelocity);
	V3StoreA(angVel1, b1.angularVelocity);
	
	PX_ASSERT(b0.linearVelocity.isFinite());
	PX_ASSERT(b0.angularVelocity.isFinite());
	PX_ASSERT(b1.linearVelocity.isFinite());
	PX_ASSERT(b1.angularVelocity.isFinite());
}
#endif //PX_SIMD_SOLVE1D

void conclude1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	PxcSolverConstraint1DHeader* header = reinterpret_cast<PxcSolverConstraint1DHeader*>(desc.constraint);
	PxU8* base = desc.constraint + sizeof(PxcSolverConstraint1DHeader);
	PxU32 stride = header->type == PXS_SC_TYPE_EXT_1D ? sizeof(PxcSolverConstraint1DExt) : sizeof(PxcSolverConstraint1D);

	for(PxU32 i=0; i<header->count; i++)
	{
		PxcSolverConstraint1D& c = *reinterpret_cast<PxcSolverConstraint1D*>(base);

		c.constant = c.unbiasedConstant;

		base += stride;
	}
	PX_ASSERT(desc.constraint + getConstraintLength(desc) == base);
}

// ==============================================================


void solveContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	PxcSolverBody& b0 = *desc.bodyA;
	PxcSolverBody& b1 = *desc.bodyB;

	//We'll need this.
	const FloatV zero	= FZero();
	const FloatV one	= FOne();

	Vec3V linVel0 = V3LoadA(b0.linearVelocity);
	Vec3V linVel1 = V3LoadA(b1.linearVelocity);
	Vec3V angVel0 = V3LoadA(b0.angularVelocity);
	Vec3V angVel1 = V3LoadA(b1.angularVelocity);

	const PxU8* PX_RESTRICT last = desc.constraint + getConstraintLength(desc);

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc.constraint;

	while(currPtr < last)
	{
		const PxcSolverContactHeader* PX_RESTRICT hdr = (PxcSolverContactHeader*)currPtr;
		currPtr += sizeof(PxcSolverContactHeader);

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		PxcSolverContactPoint* PX_RESTRICT contacts = (PxcSolverContactPoint*)currPtr;
		Ps::prefetchLine(contacts);
		currPtr += numNormalConstr * sizeof(PxcSolverContactPoint);

		PxcSolverContactFriction* PX_RESTRICT frictions = (PxcSolverContactFriction*)currPtr;
		currPtr += numFrictionConstr * sizeof(PxcSolverContactFriction);

		/*const FloatV dom0fV = hdr->getDominance0();
		const FloatV nDom1fV = FNeg(hdr->getDominance1());*/

		FloatV accumulatedNormalImpulse = zero;
		

		const FloatV invMassADom0 = hdr->getDominance0();
		const FloatV invMassBDom1 = hdr->getDominance1();

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContactPoint& c = contacts[i];
			Ps::prefetchLine(&contacts[i], 128);

			const Vec4V normalXYZ_velMultiplierW = c.normalXYZ_velMultiplierW;
			const Vec4V raXnXYZ_appliedForceW = c.raXnXYZ_appliedForceW;
			const Vec4V rbXnXYZ_scaledBiasW = c.rbXnXYZ_scaledBiasW;

			const Vec3V normal = Vec3V_From_Vec4V(normalXYZ_velMultiplierW);
			const Vec3V raXn = Vec3V_From_Vec4V(raXnXYZ_appliedForceW);
			const Vec3V rbXn = Vec3V_From_Vec4V(rbXnXYZ_scaledBiasW);

			const FloatV appliedForce = V4GetW(raXnXYZ_appliedForceW);
			const FloatV velMultiplier = V4GetW(normalXYZ_velMultiplierW);
			
			const FloatV targetVel = c.getTargetVelocity();
			const FloatV nScaledBias = FNeg(c.getScaledBias());
			const FloatV maxImpulse = c.getMaxImpulse();

			//Compute the normal velocity of the constraint.

			const FloatV normalVel1 = V3Dot(normal, linVel0);
			const FloatV normalVel2 = V3Dot(raXn, angVel0);
			const FloatV normalVel3 = V3Dot(normal, linVel1);
			const FloatV normalVel4 = V3Dot(rbXn, angVel1);

			//const FloatV unbiasedErr = FMul(targetVel, velMultiplier);
			const FloatV biasedErr = FMulAdd(targetVel, velMultiplier, nScaledBias);

			const FloatV normalVel = FSub(FAdd(normalVel1, normalVel2), FAdd(normalVel3, normalVel4));

			const Vec3V delAngVel0 = Vec3V_From_Vec4V(c.delAngVel0_targetVelocityW);

			const Vec3V delAngVel1 = Vec3V_From_Vec4V(c.delAngVel1_maxImpulseW);

			const Vec3V delLinVel0 = V3Scale(normal, invMassADom0);
			const Vec3V delLinVel1 = V3Scale(normal, invMassBDom1);

			// still lots to do here: using loop pipelining we can interweave this code with the
			// above - the code here has a lot of stalls that we would thereby eliminate

			//KS - clamp the maximum force
			const FloatV _deltaF = FMax(FNegScaleSub(normalVel, velMultiplier, biasedErr), FNeg(appliedForce));
			const FloatV _newForce = FAdd(appliedForce, _deltaF);
			const FloatV newForce = FMin(_newForce, maxImpulse);
			const FloatV deltaF = FSub(newForce, appliedForce);

			linVel0 = V3ScaleAdd(delLinVel0, deltaF, linVel0);
			linVel1 = V3ScaleAdd(delLinVel1, deltaF, linVel1);
			angVel0 = V3ScaleAdd(delAngVel0, deltaF, angVel0);
			angVel1 = V3ScaleAdd(delAngVel1, deltaF, angVel1);
			
			c.setAppliedForce(newForce);

			accumulatedNormalImpulse = FAdd(accumulatedNormalImpulse, newForce);
		}

		if(cache.doFriction && numFrictionConstr)
		{
			const FloatV staticFrictionCof = hdr->getStaticFriction();
			//const FloatV dynamicFrictionCof = FMul(staticFrictionCof, FLoad(0.9f));//hdr->getDynamicFriction();
			const FloatV dynamicFrictionCof = hdr->getDynamicFriction();
			const FloatV maxFrictionImpulse = FMul(staticFrictionCof, accumulatedNormalImpulse);
			const FloatV maxDynFrictionImpulse = FMul(dynamicFrictionCof, accumulatedNormalImpulse);
			const FloatV negMaxDynFrictionImpulse = FNeg(maxDynFrictionImpulse);
			//const FloatV negMaxFrictionImpulse = FNeg(maxFrictionImpulse);


			for(PxU32 i=0;i<numFrictionConstr;i++)
			{
				PxcSolverContactFriction& f = frictions[i];
				Ps::prefetchLine(&frictions[i],128);
				if(cache.writeBackIteration)
					Ps::prefetchLine(f.frictionBrokenWritebackByte);

				const Vec4V normalXYZ_appliedForceW = f.normalXYZ_appliedForceW;
				const Vec4V raXnXYZ_velMultiplierW = f.raXnXYZ_velMultiplierW;
				const Vec4V rbXnXYZ_biasW = f.rbXnXYZ_biasW;

				const Vec3V normal = Vec3V_From_Vec4V(normalXYZ_appliedForceW);
				const Vec3V raXn = Vec3V_From_Vec4V(raXnXYZ_velMultiplierW);
				const Vec3V rbXn = Vec3V_From_Vec4V(rbXnXYZ_biasW);

				const FloatV appliedForce = V4GetW(normalXYZ_appliedForceW);
				const FloatV bias = V4GetW(rbXnXYZ_biasW);
				const FloatV velMultiplier = V4GetW(raXnXYZ_velMultiplierW);

				const FloatV oldBroken = V4GetW(f.delAngVel0_BrokenW);

				const FloatV targetVel = V4GetW(f.delAngVel1_TargetVelW);
	
				const FloatV normalVel1 = V3Dot(normal, linVel0);
				const FloatV normalVel2 = V3Dot(raXn, angVel0);
				const FloatV normalVel3 = V3Dot(normal, linVel1);
				const FloatV normalVel4 = V3Dot(rbXn, angVel1);
				const FloatV normalVel = FSub(FAdd(normalVel1, normalVel2), FAdd(normalVel3, normalVel4));

				// appliedForce -bias * velMultiplier - a hoisted part of the total impulse computation
				const FloatV tmp1 = FNegScaleSub(FSub(bias, targetVel),velMultiplier,appliedForce);

				const Vec3V delAngVel0 = Vec3V_From_Vec4V(f.delAngVel0_BrokenW);

				const Vec3V delAngVel1 = Vec3V_From_Vec4V(f.delAngVel1_TargetVelW);

				const Vec3V delLinVel0 = V3Scale(normal, invMassADom0);
				const Vec3V delLinVel1 = V3Scale(normal, invMassBDom1);

				// Algorithm:
				// if abs(appliedForce + deltaF) > maxFrictionImpulse
				//    clamp newAppliedForce + deltaF to [-maxDynFrictionImpulse, maxDynFrictionImpulse]
				//      (i.e. clamp deltaF to [-maxDynFrictionImpulse-appliedForce, maxDynFrictionImpulse-appliedForce]
				//    set broken flag to true || broken flag

				// FloatV deltaF = FMul(FAdd(bias, normalVel), minusVelMultiplier);
				// FloatV potentialSumF = FAdd(appliedForce, deltaF);

				const FloatV totalImpulse = FNegScaleSub(normalVel, velMultiplier, tmp1);

				// On XBox this clamping code uses the vector simple pipe rather than vector float,
				// which eliminates a lot of stall cycles

				/*const BoolV clampLow = FIsGrtr(negMaxFrictionImpulse, totalImpulse);
				const BoolV clampHigh = FIsGrtr(totalImpulse, maxFrictionImpulse);

				const FloatV totalClampedLow = FMax(negMaxDynFrictionImpulse, totalImpulse);
				const FloatV totalClampedHigh = FMin(maxDynFrictionImpulse, totalImpulse);*/

				const BoolV clamp = FIsGrtr(FAbs(totalImpulse), maxFrictionImpulse);
				//const BoolV clampHigh = FIsGrtr(totalImpulse, maxFrictionImpulse);

				const FloatV totalClamped = FMin(maxDynFrictionImpulse, FMax(negMaxDynFrictionImpulse, totalImpulse));
				//const FloatV totalClampedHigh = FMin(maxDynFrictionImpulse, totalImpulse);

				/*const FloatV newAppliedForce = FSel(clampLow, totalClampedLow,
															  FSel(clampHigh, totalClampedHigh, totalImpulse));*/
				const FloatV newAppliedForce = FSel(clamp, totalClamped,totalImpulse);

				FloatV deltaF = FSub(newAppliedForce, appliedForce);


				//const FloatV broken = FSel(clampLow, one, FSel(clampHigh, one, oldBroken));
				const FloatV broken = FSel(clamp, one, oldBroken);

				// we could get rid of the stall here by calculating and clamping delta separately, but
				// the complexity isn't really worth it.

				linVel0 = V3ScaleAdd(delLinVel0, deltaF, linVel0);
				linVel1 = V3ScaleAdd(delLinVel1, deltaF, linVel1);
				angVel0 = V3ScaleAdd(delAngVel0, deltaF, angVel0);
				angVel1 = V3ScaleAdd(delAngVel1, deltaF, angVel1);

				f.setAppliedForce(newAppliedForce);

				f.delAngVel0_BrokenW = V4SetW(f.delAngVel0_BrokenW, broken);
			}
		}

	}

	// Write back
	V3StoreU(linVel0, b0.linearVelocity);
	V3StoreU(linVel1, b1.linearVelocity);
	V3StoreU(angVel0, b0.angularVelocity);
	V3StoreU(angVel1, b1.angularVelocity);

	PX_ASSERT(currPtr == last);
}

void solveContact_BStatic(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	PxcSolverBody& b0 = *desc.bodyA;
	//PxcSolverBody& b1 = *desc.bodyB;

	//We'll need this.
	const FloatV zero	= FZero();
	const FloatV one	= FOne();

	Vec3V linVel0 = V3LoadA(b0.linearVelocity);
	Vec3V angVel0 = V3LoadA(b0.angularVelocity);

	const PxU8* PX_RESTRICT last = desc.constraint + getConstraintLength(desc);

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc.constraint;

	while(currPtr < last)
	{
		const PxcSolverContactHeader* PX_RESTRICT hdr = (PxcSolverContactHeader*)currPtr;
		currPtr += sizeof(PxcSolverContactHeader);

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		PxcSolverContactPoint* PX_RESTRICT contacts = (PxcSolverContactPoint*)currPtr;
		//Ps::prefetchLine(contacts);
		currPtr += numNormalConstr * sizeof(PxcSolverContactPoint);

		PxcSolverContactFriction* PX_RESTRICT frictions = (PxcSolverContactFriction*)currPtr;
		currPtr += numFrictionConstr * sizeof(PxcSolverContactFriction);

		FloatV accumulatedNormalImpulse = zero;

		const FloatV invMassADom0 = hdr->getDominance0();

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContactPoint& c = contacts[i];
			Ps::prefetchLine(&contacts[i],128);

			const Vec4V normalXYZ_velMultiplierW = c.normalXYZ_velMultiplierW;
			const Vec4V raXnXYZ_appliedForceW = c.raXnXYZ_appliedForceW;
			//const Vec4V rbXnXYZ = c.rbXnXYZ;

			const Vec3V normal = Vec3V_From_Vec4V(normalXYZ_velMultiplierW);
			const Vec3V raXn = Vec3V_From_Vec4V(raXnXYZ_appliedForceW);
			//const Vec3V rbXn = Vec3V_From_Vec4V(rbXnXYZ);

			const FloatV appliedForce = V4GetW(raXnXYZ_appliedForceW);
			const FloatV velMultiplier = V4GetW(normalXYZ_velMultiplierW);

			const FloatV targetVel = c.getTargetVelocity();
			const FloatV nScaledBias = FNeg(c.getScaledBias());
			const FloatV maxImpulse = c.getMaxImpulse();

			//Compute the normal velocity of the constraint.

			const FloatV normalVel1 = V3Dot(normal, linVel0);
			const FloatV normalVel2 = V3Dot(raXn, angVel0);

			//const FloatV unbiasedErr = FMul(targetVel, velMultiplier);
			const FloatV biasedErr = FMulAdd(targetVel, velMultiplier, nScaledBias);

			const FloatV normalVel = FAdd(normalVel1, normalVel2);

			const Vec3V delAngVel0 = Vec3V_From_Vec4V(c.delAngVel0_targetVelocityW);

		//	const Vec3V delAngVel1 = Vec3V_From_Vec4V(c.delAngVel1_InvMassBDom);

			const Vec3V delLinVel0 = V3Scale(normal, invMassADom0);

			// still lots to do here: using loop pipelining we can interweave this code with the
			// above - the code here has a lot of stalls that we would thereby eliminate
			const FloatV _deltaF = FMax(FNegScaleSub(normalVel, velMultiplier, biasedErr), FNeg(appliedForce));
			const FloatV _newForce = FAdd(appliedForce, _deltaF);
			const FloatV newForce = FMin(_newForce, maxImpulse);
			const FloatV deltaF = FSub(newForce, appliedForce);

			linVel0 = V3ScaleAdd(delLinVel0, deltaF, linVel0);
			angVel0 = V3ScaleAdd(delAngVel0, deltaF, angVel0);

			c.setAppliedForce(newForce);

			accumulatedNormalImpulse = FAdd(accumulatedNormalImpulse, newForce);
		}

		if(cache.doFriction && numFrictionConstr)
		{
//#ifdef REMOVE_LAST_LHS
//			FloatV prevBroken;
//#endif
			//Ps::prefetchLine(frictions);
			const FloatV maxFrictionImpulse = FMul(hdr->getStaticFriction(), accumulatedNormalImpulse);
			const FloatV maxDynFrictionImpulse = FMul(hdr->getDynamicFriction(), accumulatedNormalImpulse);

			for(PxU32 i=0;i<numFrictionConstr;i++)
			{
				PxcSolverContactFriction& f = frictions[i];
				Ps::prefetchLine(&frictions[i],128);
				if(cache.writeBackIteration)
					Ps::prefetchLine(f.frictionBrokenWritebackByte);

				const Vec4V normalXYZ_appliedForceW = f.normalXYZ_appliedForceW;
				const Vec4V raXnXYZ_velMultiplierW = f.raXnXYZ_velMultiplierW;
				const Vec4V rbXnXYZ_biasW = f.rbXnXYZ_biasW;

				const Vec3V normal = Vec3V_From_Vec4V(normalXYZ_appliedForceW);
				const Vec3V raXn = Vec3V_From_Vec4V(raXnXYZ_velMultiplierW);

				const FloatV appliedForce = V4GetW(normalXYZ_appliedForceW);
				const FloatV bias = V4GetW(rbXnXYZ_biasW);
				const FloatV velMultiplier = V4GetW(raXnXYZ_velMultiplierW);

				const FloatV targetVel = V4GetW(f.delAngVel1_TargetVelW);
	
				const FloatV negMaxDynFrictionImpulse = FNeg(maxDynFrictionImpulse);
				//const FloatV negMaxFrictionImpulse = FNeg(maxFrictionImpulse);

				const FloatV normalVel1 = V3Dot(normal, linVel0);
				const FloatV normalVel2 = V3Dot(raXn, angVel0);
				const FloatV normalVel = FAdd(normalVel1, normalVel2);

				// appliedForce -bias * velMultiplier - a hoisted part of the total impulse computation
				const FloatV tmp1 = FNegMulSub(FSub(bias, targetVel),velMultiplier,appliedForce); 

				const Vec3V delAngVel0 = Vec3V_From_Vec4V(f.delAngVel0_BrokenW);
				const Vec3V delLinVel0 = V3Scale(normal, invMassADom0);

				const FloatV oldBroken = V4GetW(f.delAngVel0_BrokenW);

				// Algorithm:
				// if abs(appliedForce + deltaF) > maxFrictionImpulse
				//    clamp newAppliedForce + deltaF to [-maxDynFrictionImpulse, maxDynFrictionImpulse]
				//      (i.e. clamp deltaF to [-maxDynFrictionImpulse-appliedForce, maxDynFrictionImpulse-appliedForce]
				//    set broken flag to true || broken flag

				// FloatV deltaF = FMul(FAdd(bias, normalVel), minusVelMultiplier);
				// FloatV potentialSumF = FAdd(appliedForce, deltaF);

				const FloatV totalImpulse = FNegMulSub(normalVel, velMultiplier, tmp1);

				// On XBox this clamping code uses the vector simple pipe rather than vector float,
				// which eliminates a lot of stall cycles

				/*const BoolV clampLow = FIsGrtr(negMaxFrictionImpulse, totalImpulse);
				const BoolV clampHigh = FIsGrtr(totalImpulse, maxFrictionImpulse);

				const FloatV totalClampedLow = FMax(negMaxDynFrictionImpulse, totalImpulse);
				const FloatV totalClampedHigh = FMin(maxDynFrictionImpulse, totalImpulse);

				const FloatV newAppliedForce = FSel(clampLow, totalClampedLow,
															  FSel(clampHigh, totalClampedHigh, totalImpulse));

				const FloatV broken = FSel(clampLow, one, FSel(clampHigh, one, oldBroken));

				FloatV deltaF = FSub(newAppliedForce, appliedForce);*/

				const BoolV clamp = FIsGrtr(FAbs(totalImpulse), maxFrictionImpulse);
				//const BoolV clampHigh = FIsGrtr(totalImpulse, maxFrictionImpulse);

				const FloatV totalClamped = FMin(maxDynFrictionImpulse, FMax(negMaxDynFrictionImpulse, totalImpulse));
				//const FloatV totalClampedHigh = FMin(maxDynFrictionImpulse, totalImpulse);

				/*const FloatV newAppliedForce = FSel(clampLow, totalClampedLow,
															  FSel(clampHigh, totalClampedHigh, totalImpulse));*/
				const FloatV newAppliedForce = FSel(clamp, totalClamped,totalImpulse);

				FloatV deltaF = FSub(newAppliedForce, appliedForce);

				const FloatV broken = FSel(clamp, one, oldBroken);


				// we could get rid of the stall here by calculating and clamping delta separately, but
				// the complexity isn't really worth it.

				linVel0 = V3ScaleAdd(delLinVel0, deltaF, linVel0);
				angVel0 = V3ScaleAdd(delAngVel0, deltaF, angVel0);

				f.setAppliedForce(newAppliedForce);

				f.delAngVel0_BrokenW = V4SetW(f.delAngVel0_BrokenW, broken);
			}
		}

	}

	// Write back
	V3StoreU(linVel0, b0.linearVelocity);
	V3StoreU(angVel0, b0.angularVelocity);

	PX_ASSERT(currPtr == last);
}


void concludeContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	PxU8* PX_RESTRICT cPtr = desc.constraint;

	const FloatV zero = FZero();

	PxU8* PX_RESTRICT last = desc.constraint + getConstraintLength(desc);
	while(cPtr < last)
	{
		const PxcSolverContactHeader* PX_RESTRICT hdr = (const PxcSolverContactHeader*)cPtr;
		cPtr += sizeof(PxcSolverContactHeader);

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		//if(cPtr < last)
		//Ps::prefetchLine(cPtr, 512);
		Ps::prefetchLine(cPtr,128);
		Ps::prefetchLine(cPtr,256);
		Ps::prefetchLine(cPtr,384);

		const PxU32 pointStride = hdr->type == PXS_SC_TYPE_EXT_CONTACT ? sizeof(PxcSolverContactPointExt)
																	   : sizeof(PxcSolverContactPoint);
		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContactPoint *c = reinterpret_cast<PxcSolverContactPoint*>(cPtr);
			cPtr += pointStride;
			c->setScaledBias(FMax(c->getScaledBias(), zero));
		}

		const PxU32 frictionStride = hdr->type == PXS_SC_TYPE_EXT_CONTACT ? sizeof(PxcSolverContactFrictionExt)
																		  : sizeof(PxcSolverContactFriction);
		for(PxU32 i=0;i<numFrictionConstr;i++)
		{
			PxcSolverContactFriction *f = reinterpret_cast<PxcSolverContactFriction*>(cPtr);
			cPtr += frictionStride;
			f->setBias(zero);
		}
	}
	PX_ASSERT(cPtr == last);
}

void writeBackContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache,
					  PxcSolverBodyData& bd0, PxcSolverBodyData& bd1)
{
	//PxcSolverBodyData& bd0 = cache.solverBodyArray[desc.bodyADataIndex];
	//PxcSolverBodyData& bd1 = cache.solverBodyArray[desc.bodyBDataIndex];

	PxReal normalForce = 0;

	PxU8* PX_RESTRICT cPtr = desc.constraint;
	PxReal* PX_RESTRICT vForceWriteback = reinterpret_cast<PxReal*>(desc.writeBack);
	PxU8* PX_RESTRICT last = desc.constraint + getConstraintLength(desc);

	bool forceThreshold = false;

	while(cPtr < last)
	{
		const PxcSolverContactHeader* PX_RESTRICT hdr = (const PxcSolverContactHeader*)cPtr;
		cPtr += sizeof(PxcSolverContactHeader);

		forceThreshold = hdr->flags & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;
		const PxU32 numNormalConstr = hdr->numNormalConstr;
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		//if(cPtr < last)
		Ps::prefetchLine(cPtr, 256);
		Ps::prefetchLine(cPtr, 384);

		const PxU32 pointStride = hdr->type == PXS_SC_TYPE_EXT_CONTACT ? sizeof(PxcSolverContactPointExt)
																	   : sizeof(PxcSolverContactPoint);

		if(vForceWriteback!=NULL)
		{
			for(PxU32 i=0; i<numNormalConstr; i++)
			{
				PxcSolverContactPoint* c = reinterpret_cast<PxcSolverContactPoint*>(cPtr);
				cPtr += pointStride;

				const PxReal appliedForce = FStore(c->getAppliedForce());
				*vForceWriteback++ = appliedForce;
				normalForce += appliedForce;
			}
		}
		else
			cPtr += numNormalConstr * pointStride;

		const PxU32 frictionStride = hdr->type == PXS_SC_TYPE_EXT_CONTACT ? sizeof(PxcSolverContactFrictionExt)
																		  : sizeof(PxcSolverContactFriction);

		for(PxU32 i=0; i<numFrictionConstr; i++)
		{
			PxcSolverContactFriction* f = reinterpret_cast<PxcSolverContactFriction*>(cPtr);
			cPtr += frictionStride;
			if(f->frictionBrokenWritebackByte != NULL && PX_IR(((PxVec4*)&f->delAngVel0_BrokenW)->w))
				*f->frictionBrokenWritebackByte = 1;	// PT: bad L2 miss here
		}
	}
	PX_ASSERT(cPtr == last);

	if(forceThreshold && desc.linkIndexA == PxcSolverConstraintDesc::NO_LINK && desc.linkIndexB == PxcSolverConstraintDesc::NO_LINK &&
		normalForce !=0 && (bd0.reportThreshold < PX_MAX_REAL  || bd1.reportThreshold < PX_MAX_REAL))
	{
		PxcThresholdStreamElement elt;
		elt.normalForce = normalForce;
		elt.threshold = PxMin<float>(bd0.reportThreshold, bd1.reportThreshold);
		elt.body0 = bd0.originalBody;
		elt.body1 = bd1.originalBody;
		Ps::order(elt.body0,elt.body1);
		PX_ASSERT(elt.body0 < elt.body1);
		PX_ASSERT(cache.mThresholdStreamIndex<cache.mThresholdStreamLength);
		cache.mThresholdStream[cache.mThresholdStreamIndex++] = elt;
	}
}

// adjust from CoM to joint

void writeBack1D(const PxcSolverConstraintDesc& desc, PxcSolverContext&, PxcSolverBodyData&, PxcSolverBodyData&)
{
	PxsConstraintWriteback* writeback = reinterpret_cast<PxsConstraintWriteback*>(desc.writeBack);
	if(writeback)
	{
		PxcSolverConstraint1DHeader* header = reinterpret_cast<PxcSolverConstraint1DHeader*>(desc.constraint);
		PxU8* base = desc.constraint + sizeof(PxcSolverConstraint1DHeader);
		PxU32 stride = header->type == PXS_SC_TYPE_EXT_1D ? sizeof(PxcSolverConstraint1DExt) : sizeof(PxcSolverConstraint1D);

		PxVec3 lin(0), ang(0);
		for(PxU32 i=0; i<header->count; i++)
		{
			const PxcSolverConstraint1D* c = reinterpret_cast<PxcSolverConstraint1D*>(base);
			if(c->flags & PXS_SC_FLAG_OUTPUT_FORCE)
			{
				lin += c->lin0 * c->appliedForce;
				ang += c->ang0 * c->appliedForce;
			}
			base += stride;
		}

		ang -= header->body0WorldOffset.cross(lin);
		writeback->linearImpulse = lin;
		writeback->angularImpulse = ang;
		writeback->broken = PxU32(lin.magnitude()>header->linBreakImpulse || ang.magnitude()>header->angBreakImpulse);

		PX_ASSERT(desc.constraint + getConstraintLength(desc) == base);
	}
}


void solve1DBlock (const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		solve1D(desc[a-1], cache);
	}
	solve1D(desc[constraintCount-1], cache);
}

void solve1DConcludeBlock (const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		solve1D(desc[a-1], cache);
		conclude1D(desc[a-1], cache);
	}
	solve1D(desc[constraintCount-1], cache);
	conclude1D(desc[constraintCount-1], cache);
}

void solve1DBlockWriteBack (const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
							PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a-1].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a-1].bodyBDataIndex];
		solve1D(desc[a-1], cache);
		writeBack1D(desc[a-1], cache, bd0, bd1);
	}
	PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[constraintCount-1].bodyADataIndex];
	PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[constraintCount-1].bodyBDataIndex];
	solve1D(desc[constraintCount-1], cache);
	writeBack1D(desc[constraintCount-1], cache, bd0, bd1);
}

void writeBack1DBlock (const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
							PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a-1].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a-1].bodyBDataIndex];
		writeBack1D(desc[a-1], cache, bd0, bd1);
	}
	PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[constraintCount-1].bodyADataIndex];
	PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[constraintCount-1].bodyBDataIndex];
	writeBack1D(desc[constraintCount-1], cache, bd0, bd1);
}

void solveContactBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		solveContact(desc[a-1], cache);
	}
	solveContact(desc[constraintCount-1], cache);
}

void solveContactConcludeBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		solveContact(desc[a-1], cache);
		concludeContact(desc[a-1], cache);
	}
	solveContact(desc[constraintCount-1], cache);
	concludeContact(desc[constraintCount-1], cache);
}

void solveContactBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
								PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a-1].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a-1].bodyBDataIndex];
		solveContact(desc[a-1], cache);
		writeBackContact(desc[a-1], cache, bd0, bd1);
	}
	PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[constraintCount-1].bodyADataIndex];
	PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[constraintCount-1].bodyBDataIndex];
	solveContact(desc[constraintCount-1], cache);
	writeBackContact(desc[constraintCount-1], cache, bd0, bd1);

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

void contactBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
								PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a].bodyBDataIndex];
		writeBackContact(desc[a], cache, bd0, bd1);
	}

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

void solveContact_BStaticBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		solveContact_BStatic(desc[a-1], cache);
	}
	solveContact_BStatic(desc[constraintCount-1], cache);
}

void solveContact_BStaticConcludeBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		solveContact_BStatic(desc[a-1], cache);
		concludeContact(desc[a-1], cache);
	}
	solveContact_BStatic(desc[constraintCount-1], cache);
	concludeContact(desc[constraintCount-1], cache);
}

void solveContact_BStaticBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	for(PxU32 a = 1; a < constraintCount; ++a)
	{
		Ps::prefetchLine(desc[a].constraint);
		Ps::prefetchLine(desc[a].constraint, 128);
		Ps::prefetchLine(desc[a].constraint, 256);
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a-1].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a-1].bodyBDataIndex];
		solveContact_BStatic(desc[a-1], cache);
		writeBackContact(desc[a-1], cache, bd0, bd1);
	}
	PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[constraintCount-1].bodyADataIndex];
	PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[constraintCount-1].bodyBDataIndex];
	solveContact_BStatic(desc[constraintCount-1], cache);
	writeBackContact(desc[constraintCount-1], cache, bd0, bd1);

	if(cache.mThresholdStreamIndex > (cache.mThresholdStreamLength - 4))
	{
		//Not enough space to write 4 more thresholds back!
		//Write back to global buffer
		PxI32 threshIndex = physx::shdfnd::atomicAdd(outThresholdPairs, (PxI32)cache.mThresholdStreamIndex) - (PxI32)cache.mThresholdStreamIndex;
		for(PxU32 a = 0; a < cache.mThresholdStreamIndex; ++a)
		{
			thresholdStream[a + threshIndex] = cache.mThresholdStream[a];
		}
		cache.mThresholdStreamIndex = 0;
	}
}

void solveConclude1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solve1D(desc, cache);
	conclude1D(desc, cache);
}

void solveConcludeContact			(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solveContact(desc, cache);
	concludeContact(desc, cache);
}

void solveConcludeExtContact		(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solveExtContact(desc, cache);
	concludeContact(desc, cache);
}

void solveConcludeExt1D				(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solveExt1D(desc, cache);
	conclude1D(desc, cache);
}

void solveConcludeContact_BStatic	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solveContact_BStatic(desc, cache);
	concludeContact(desc, cache);
}


}

#endif
