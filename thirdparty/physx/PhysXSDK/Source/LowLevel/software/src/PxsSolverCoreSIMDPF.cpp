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
#include "PxcSolverContactPF.h"
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

PX_FORCE_INLINE PxU32 SelectMask(const bool selectMask, const PxU32 ifTrue, const PxU32 ifFalse)
{
	const PxU32 mask = (PxU32)selectMask;
	const PxU32 maskFalse = (mask - 1);
	const PxU32 maskTrue = ~maskFalse;
	return (maskTrue & ifTrue) | (maskFalse & ifFalse);
}

// ==============================================================

void solveContactCoulomb(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	PxcSolverBody& b0 = *desc.bodyA;
	PxcSolverBody& b1 = *desc.bodyB;

	Vec3V linVel0 = V3LoadA(b0.linearVelocity);
	Vec3V linVel1 = V3LoadA(b1.linearVelocity);
	Vec3V angVel0 = V3LoadA(b0.angularVelocity);
	Vec3V angVel1 = V3LoadA(b1.angularVelocity);

	const PxcSolverContactCoulombHeader* PX_RESTRICT firstHeader = (PxcSolverContactCoulombHeader*)desc.constraint;
	const PxU8* PX_RESTRICT last = desc.constraint + firstHeader->frictionOffset;//getConstraintLength(desc);

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc.constraint;

	
	const FloatV zero = FZero();

	while(currPtr < last)
	{
		PxcSolverContactCoulombHeader* PX_RESTRICT hdr = (PxcSolverContactCoulombHeader*)currPtr;
		currPtr += sizeof(PxcSolverContactCoulombHeader);

		const PxU32 numNormalConstr = hdr->numNormalConstr;

		const Vec3V normal = hdr->getNormal();
		const FloatV invMassDom0 = FLoad(hdr->dominance0);
		const FloatV invMassDom1 = FLoad(hdr->dominance1);
		const Vec3V delLinVel0 = V3Scale(normal, invMassDom0);
		const Vec3V delLinVel1 = V3Scale(normal, invMassDom1);

		//const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		PxcSolverContact* PX_RESTRICT contacts = (PxcSolverContact*)currPtr;
		currPtr += numNormalConstr * sizeof(PxcSolverContact);

		PxF32* appliedImpulse = (PxF32*) (((PxU8*)hdr) + hdr->frictionOffset + sizeof(PxcSolverFrictionHeader));
		Ps::prefetchLine(appliedImpulse);

		FloatV normalVel1 = V3Dot(normal, linVel0);
		FloatV normalVel3 = V3Dot(normal, linVel1);

		FloatV accumDeltaF =zero;

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContact& c = contacts[i];
			Ps::prefetchLine(&contacts[i], 128);

			//const Vec4V normalXYZ_velMultiplierW = c.normalXYZ_velMultiplierW;
			const Vec4V raXnXYZ_appliedForceW = c.raXnXYZ_appliedForceW;
			const Vec4V rbXnXYZ_velMultiplierW = c.rbXnXYZ_velMultiplierW;

			//const Vec3V normal = c.normal;
			//const Vec3V normal = Vec3V_From_Vec4V(normalXYZ_velMultiplierW);
			const Vec3V raXn = Vec3V_From_Vec4V(raXnXYZ_appliedForceW);
			const Vec3V rbXn = Vec3V_From_Vec4V(rbXnXYZ_velMultiplierW);
			//const Vec3V rbXn =  Vec3V_From_Vec4V(c.rbXnXYZ);

			const FloatV appliedForce = V4GetW(raXnXYZ_appliedForceW);
			const FloatV velMultiplier = V4GetW(rbXnXYZ_velMultiplierW);
			//const FloatV velMultiplier = V4GetW(normalXYZ_velMultiplierW);

			const FloatV targetVel = c.getTargetVelocity();
			const FloatV nScaledBias = FNeg(c.getScaledBias());
			const FloatV maxImpulse = c.getMaxImpulse();

			//Compute the normal velocity of the constraint.

			//const FloatV normalVel1 = V3Dot(normal, linVel0);
			const FloatV normalVel2 = V3Dot(raXn, angVel0);
			//const FloatV normalVel3 = V3Dot(normal, linVel1);
			const FloatV normalVel4 = V3Dot(rbXn, angVel1);

			//const FloatV unbiasedErr = FMul(targetVel, velMultiplier);
			const FloatV biasedErr = FMulAdd(targetVel, velMultiplier, nScaledBias);

			const FloatV normalVel = FSub(FAdd(normalVel1, normalVel2), FAdd(normalVel3, normalVel4));
			//const FloatV normalNNVel = FSub(FAdd(normalNVel1, normalVel2), FAdd(normalNVel3, normalVel4));

			const Vec3V delAngVel0 = Vec3V_From_Vec4V(c.delAngVel0_InvMassADom);

			const Vec3V delAngVel1 = Vec3V_From_Vec4V(c.delAngVel1_InvMassBDom);

			//const Vec3V delLinVel0 = V3Scale(normal, V4GetW(c.delAngVel0_InvMassADom));
			//const Vec3V delLinVel1 = V3Scale(normal, V4GetW(c.delAngVel1_InvMassBDom));

			// still lots to do here: using loop pipelining we can interweave this code with the
			// above - the code here has a lot of stalls that we would thereby eliminate

			const FloatV unclampedDeltaF = FNegScaleSub(normalVel, velMultiplier, biasedErr);
			const FloatV _deltaF = FMax(unclampedDeltaF, FNeg(appliedForce));
			const FloatV _newForce = FAdd(appliedForce, _deltaF);
			const FloatV newForce = FMin(_newForce, maxImpulse);
			const FloatV deltaF = FSub(newForce, appliedForce);

			//linVel0 = V3ScaleAdd(delLinVel0, deltaF, linVel0);
			//linVel1 = V3ScaleAdd(delLinVel1, deltaF, linVel1);

			angVel0 = V3ScaleAdd(delAngVel0, deltaF, angVel0);
			angVel1 = V3ScaleAdd(delAngVel1, deltaF, angVel1);

			normalVel1 = FScaleAdd(invMassDom0, deltaF, normalVel1);
			normalVel3 = FScaleAdd(invMassDom1, deltaF, normalVel3);

			accumDeltaF = FAdd(deltaF, accumDeltaF);

			c.setAppliedForce(newForce);
			FStore(newForce, &appliedImpulse[i]);
			Ps::prefetchLine(&appliedImpulse[i], 128);

		}
		linVel0 = V3ScaleAdd(delLinVel0, accumDeltaF, linVel0);
		linVel1 = V3ScaleAdd(delLinVel1, accumDeltaF, linVel1);

	}

	// Write back
	V3StoreU(linVel0, b0.linearVelocity);
	V3StoreU(linVel1, b1.linearVelocity);
	V3StoreU(angVel0, b0.angularVelocity);
	V3StoreU(angVel1, b1.angularVelocity);

	PX_ASSERT(currPtr == last);
}

void solveFriction(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	PxcSolverBody& b0 = *desc.bodyA;
	PxcSolverBody& b1 = *desc.bodyB;

	Vec3V linVel0 = V3LoadA(b0.linearVelocity);
	Vec3V linVel1 = V3LoadA(b1.linearVelocity);
	Vec3V angVel0 = V3LoadA(b0.angularVelocity);
	Vec3V angVel1 = V3LoadA(b1.angularVelocity);

	//PxVec3 l0, l1, a0, a1;
	//PxVec3_From_Vec3V(linVel0, l0);
	//PxVec3_From_Vec3V(linVel1, l1);
	//PxVec3_From_Vec3V(angVel0, a0);
	//PxVec3_From_Vec3V(angVel0, a1);

	//PX_ASSERT(l0.isFinite());
	//PX_ASSERT(l1.isFinite());
	//PX_ASSERT(a0.isFinite());
	//PX_ASSERT(a1.isFinite());

	const PxU8* PX_RESTRICT ptr = desc.constraint;
	const PxU8* PX_RESTRICT currPtr = ptr;

	const PxU8* PX_RESTRICT last = ptr + getConstraintLength(desc);


	while(currPtr < last)
	{
		const PxcSolverFrictionHeader* PX_RESTRICT frictionHeader = (PxcSolverFrictionHeader*)currPtr;
		currPtr += sizeof(PxcSolverFrictionHeader);
		PxF32* appliedImpulse = (PxF32*)currPtr;
		currPtr += frictionHeader->getAppliedForcePaddingSize();

		PxcSolverFriction* PX_RESTRICT frictions = (PxcSolverFriction*)currPtr;
		const PxU32 numFrictionConstr = frictionHeader->numFrictionConstr;

		currPtr += numFrictionConstr * sizeof(PxcSolverFriction);
		const FloatV staticFriction = frictionHeader->getStaticFriction();

		for(PxU32 i=0;i<numFrictionConstr;i++)
		{
		
			PxcSolverFriction& f = frictions[i];
			Ps::prefetchLine(&frictions[i], 128);

			const Vec3V t0 = Vec3V_From_Vec4V(f.normalXYZ_appliedForceW);
			const Vec3V raXt0 = Vec3V_From_Vec4V(f.raXnXYZ_velMultiplierW);
			const Vec3V rbXt0 = Vec3V_From_Vec4V(f.rbXnXYZ_targetVelocityW);

			const FloatV appliedForce = V4GetW(f.normalXYZ_appliedForceW);
			const FloatV velMultiplier = V4GetW(f.raXnXYZ_velMultiplierW);

			const FloatV targetVel = V4GetW(f.rbXnXYZ_targetVelocityW);

			//const FloatV normalImpulse = contacts[f.contactIndex].getAppliedForce();
			const FloatV normalImpulse = FLoad(appliedImpulse[f.contactIndex]);
			const FloatV maxFriction = FMul(staticFriction, normalImpulse);
			const FloatV nMaxFriction = FNeg(maxFriction);

			//Compute the normal velocity of the constraint.

			//const FloatV t0Vel13 = V3Dot(t0, V3Sub(linVel1, linVel0));
			const FloatV t0Vel1 = V3Dot(t0, linVel0);
			const FloatV t0Vel2 = V3Dot(raXt0, angVel0);
			const FloatV t0Vel3 = V3Dot(t0, linVel1);
			const FloatV t0Vel4 = V3Dot(rbXt0, angVel1);

			//const FloatV unbiasedErr = FMul(targetVel, velMultiplier);
			//const FloatV biasedErr = FMulAdd(targetVel, velMultiplier, nScaledBias);

			const FloatV t0Vel = FSub(FAdd(t0Vel1, t0Vel2), FAdd(t0Vel3, t0Vel4));
			//const FloatV t0Vel = FAdd(t0Vel13, FSub(t0Vel2, t0Vel4));

			const Vec3V delAngVel0 = Vec3V_From_Vec4V(f.delAngVel0_InvMassADom);
			const Vec3V delAngVel1 = Vec3V_From_Vec4V(f.delAngVel1_InvMassBDom);
			const Vec3V delLinVel0 = V3Scale(t0, V4GetW(f.delAngVel0_InvMassADom));
			const Vec3V delLinVel1 = V3Scale(t0, V4GetW(f.delAngVel1_InvMassBDom));

			// still lots to do here: using loop pipelining we can interweave this code with the
			// above - the code here has a lot of stalls that we would thereby eliminate

			
			//FloatV deltaF = FNeg(FMul(t0Vel, velMultiplier));
			//FloatV newForce = FAdd(appliedForce, deltaF);
			const FloatV tmp = FNegMulSub(targetVel,velMultiplier,appliedForce);
			//FloatV newForce = FMulAdd(t0Vel, velMultiplier, appliedForce);
			FloatV newForce = FMulAdd(t0Vel, velMultiplier, tmp);
			newForce = FClamp(newForce, nMaxFriction, maxFriction);
			FloatV deltaF = FSub(newForce, appliedForce);

			linVel0 = V3ScaleAdd(delLinVel0, deltaF, linVel0);
			linVel1 = V3ScaleAdd(delLinVel1, deltaF, linVel1);
			angVel0 = V3ScaleAdd(delAngVel0, deltaF, angVel0);
			angVel1 = V3ScaleAdd(delAngVel1, deltaF, angVel1);

			f.setAppliedForce(newForce);
		}
	}


	//PxVec3_From_Vec3V(linVel0, l0);
	//PxVec3_From_Vec3V(linVel1, l1);
	//PxVec3_From_Vec3V(angVel0, a0);
	//PxVec3_From_Vec3V(angVel0, a1);

	//PX_ASSERT(l0.isFinite());
	//PX_ASSERT(l1.isFinite());
	//PX_ASSERT(a0.isFinite());
	//PX_ASSERT(a1.isFinite());

	// Write back
	V3StoreU(linVel0, b0.linearVelocity);
	V3StoreU(linVel1, b1.linearVelocity);
	V3StoreU(angVel0, b0.angularVelocity);
	V3StoreU(angVel1, b1.angularVelocity);


	PX_ASSERT(currPtr == last);
}

void solveContactCoulomb_BStatic(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	PxcSolverBody& b0 = *desc.bodyA;


	Vec3V linVel0 = V3LoadA(b0.linearVelocity);
	Vec3V angVel0 = V3LoadA(b0.angularVelocity);

	PxcSolverContactCoulombHeader* firstHeader = (PxcSolverContactCoulombHeader*)desc.constraint;
	const PxU8* PX_RESTRICT last = desc.constraint + firstHeader->frictionOffset;//getConstraintLength(desc);

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc.constraint;

	const FloatV zero = FZero();

	while(currPtr < last)
	{
		PxcSolverContactCoulombHeader* PX_RESTRICT hdr = (PxcSolverContactCoulombHeader*)currPtr;
		currPtr += sizeof(PxcSolverContactCoulombHeader);

		const PxU32 numNormalConstr = hdr->numNormalConstr;

		PxcSolverContact* PX_RESTRICT contacts = (PxcSolverContact*)currPtr;
		Ps::prefetchLine(contacts);
		currPtr += numNormalConstr * sizeof(PxcSolverContact);

		PxF32* appliedImpulse = (PxF32*) (((PxU8*)hdr) + hdr->frictionOffset + sizeof(PxcSolverFrictionHeader));
		Ps::prefetchLine(appliedImpulse);

		const Vec3V normal = hdr->getNormal();

		const FloatV invMassDom0 = FLoad(hdr->dominance0);

		FloatV normalVel1 = V3Dot(normal, linVel0);

		const Vec3V delLinVel0 = V3Scale(normal, invMassDom0);
		FloatV accumDeltaF = zero;
		//FloatV accumImpulse = zero;

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContact& c = contacts[i];
			Ps::prefetchLine(&contacts[i+1]);

			//const Vec4V normalXYZ_velMultiplierW = c.normalXYZ_velMultiplierW;
			const Vec4V raXnXYZ_appliedForceW = c.raXnXYZ_appliedForceW;
			const Vec4V rbXnXYZ_velMultiplierW = c.rbXnXYZ_velMultiplierW;

			//const Vec3V normal = c.normal;
			//const Vec3V normal = Vec3V_From_Vec4V(normalXYZ_velMultiplierW);
			const Vec3V raXn = Vec3V_From_Vec4V(raXnXYZ_appliedForceW);

			const FloatV appliedForce = V4GetW(raXnXYZ_appliedForceW);
			const FloatV velMultiplier = V4GetW(rbXnXYZ_velMultiplierW);
			//const FloatV velMultiplier = V4GetW(normalXYZ_velMultiplierW);

			const Vec3V delAngVel0 = Vec3V_From_Vec4V(c.delAngVel0_InvMassADom);

			const FloatV targetVel = c.getTargetVelocity();
			const FloatV nScaledBias = FNeg(c.getScaledBias());
			const FloatV maxImpulse = c.getMaxImpulse();

			//Compute the normal velocity of the constraint.

			//const FloatV normalVel1 = V3Dot(normal, linVel0);
			const FloatV normalVel2 = V3Dot(raXn, angVel0);
			const FloatV normalVel =  FAdd(normalVel1, normalVel2);

			//const FloatV unbiasedErr = FMul(targetVel, velMultiplier);
			const FloatV biasedErr = FMulAdd(targetVel, velMultiplier, nScaledBias);

			// still lots to do here: using loop pipelining we can interweave this code with the
			// above - the code here has a lot of stalls that we would thereby eliminate

			const FloatV _deltaF = FMax(FNegMulSub(normalVel, velMultiplier, biasedErr), FNeg(appliedForce));
			const FloatV _newForce = FAdd(appliedForce, _deltaF);
			const FloatV newForce = FMin(_newForce, maxImpulse);
			const FloatV deltaF = FSub(newForce, appliedForce);

			//linVel0 = V3MulAdd(delLinVel0, deltaF, linVel0);
			normalVel1 = FScaleAdd(invMassDom0, deltaF, normalVel1);
			angVel0 = V3ScaleAdd(delAngVel0, deltaF, angVel0);

			accumDeltaF = FAdd(accumDeltaF, deltaF);

			c.setAppliedForce(newForce);
			Ps::aos::FStore(newForce, &appliedImpulse[i]);
			Ps::prefetchLine(&appliedImpulse[i], 128);

			//accumImpulse = FAdd(accumImpulse, newAppliedForce);
		}
		linVel0 = V3ScaleAdd(delLinVel0, accumDeltaF, linVel0);
		//hdr->setAccumlatedForce(accumImpulse);
	}

	// Write back
	V3StoreU(linVel0, b0.linearVelocity);
	V3StoreU(angVel0, b0.angularVelocity);

	PX_ASSERT(currPtr == last);
}

void solveFriction_BStatic(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	PxcSolverBody& b0 = *desc.bodyA;

	Vec3V linVel0 = V3LoadA(b0.linearVelocity);
	Vec3V angVel0 = V3LoadA(b0.angularVelocity);

	const PxU8* PX_RESTRICT currPtr = desc.constraint;

	const PxU8* PX_RESTRICT last = currPtr + getConstraintLength(desc);

	//hopefully pointer aliasing doesn't bite.

	//PxVec3 l0, a0;
	//PxVec3_From_Vec3V(linVel0, l0);
	//PxVec3_From_Vec3V(angVel0, a0);

	//PX_ASSERT(l0.isFinite());
	//PX_ASSERT(a0.isFinite());
	

	while(currPtr < last)
	{

		const PxcSolverFrictionHeader* PX_RESTRICT frictionHeader = (PxcSolverFrictionHeader*)currPtr;
		const PxU32 numFrictionConstr = frictionHeader->numFrictionConstr;
		currPtr +=sizeof(PxcSolverFrictionHeader);
		PxF32* appliedImpulse = (PxF32*)currPtr;
		currPtr +=frictionHeader->getAppliedForcePaddingSize();

		PxcSolverFriction* PX_RESTRICT frictions = (PxcSolverFriction*)currPtr;
		currPtr += numFrictionConstr * sizeof(PxcSolverFriction);


		const FloatV staticFriction = frictionHeader->getStaticFriction();

		for(PxU32 i=0;i<numFrictionConstr;i++)   
		{
			PxcSolverFriction& f = frictions[i];
			Ps::prefetchLine(&frictions[i+1]);

			const Vec3V t0 = Vec3V_From_Vec4V(f.normalXYZ_appliedForceW);
			const Vec3V raXt0 = Vec3V_From_Vec4V(f.raXnXYZ_velMultiplierW);

			const FloatV appliedForce = V4GetW(f.normalXYZ_appliedForceW);
			const FloatV velMultiplier = V4GetW(f.raXnXYZ_velMultiplierW);

			const FloatV targetVel = V4GetW(f.rbXnXYZ_targetVelocityW);
			
			//const FloatV normalImpulse = contacts[f.contactIndex].getAppliedForce();
			const FloatV normalImpulse = FLoad(appliedImpulse[f.contactIndex]);
			const FloatV maxFriction = FMul(staticFriction, normalImpulse);
			const FloatV nMaxFriction = FNeg(maxFriction);

			//Compute the normal velocity of the constraint.

			const FloatV t0Vel1 = V3Dot(t0, linVel0);
			const FloatV t0Vel2 = V3Dot(raXt0, angVel0);

			//const FloatV unbiasedErr = FMul(targetVel, velMultiplier);
			//const FloatV biasedErr = FMulAdd(targetVel, velMultiplier, nScaledBias);

			const FloatV t0Vel = FAdd(t0Vel1, t0Vel2);

			const Vec3V delAngVel0 = Vec3V_From_Vec4V(f.delAngVel0_InvMassADom);
			const Vec3V delLinVel0 = V3Scale(t0, V4GetW(f.delAngVel0_InvMassADom));

			// still lots to do here: using loop pipelining we can interweave this code with the
			// above - the code here has a lot of stalls that we would thereby eliminate

				//FloatV deltaF = FSub(scaledBias, FMul(t0Vel, velMultiplier));//FNeg(FMul(t0Vel, velMultiplier));
			//FloatV deltaF = FMul(t0Vel, velMultiplier);
			//FloatV newForce = FMulAdd(t0Vel, velMultiplier, appliedForce);

			const FloatV tmp = FNegMulSub(targetVel,velMultiplier,appliedForce);
			FloatV newForce = FMulAdd(t0Vel, velMultiplier, tmp);
			newForce = FClamp(newForce, nMaxFriction, maxFriction);
			const FloatV deltaF = FSub(newForce, appliedForce);

			linVel0 = V3ScaleAdd(delLinVel0, deltaF, linVel0);
			angVel0 = V3ScaleAdd(delAngVel0, deltaF, angVel0);

			f.setAppliedForce(newForce);
		}
	}

	//PxVec3_From_Vec3V(linVel0, l0);
	//PxVec3_From_Vec3V(angVel0, a0);

	//PX_ASSERT(l0.isFinite());
	//PX_ASSERT(a0.isFinite());

	// Write back
	V3StoreU(linVel0, b0.linearVelocity);
	V3StoreU(angVel0, b0.angularVelocity);

	PX_ASSERT(currPtr == last);
}


void concludeContactCoulomb(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	PxU8* PX_RESTRICT cPtr = desc.constraint;

	const FloatV zero = FZero();

	const PxcSolverContactCoulombHeader* PX_RESTRICT firstHeader = (const PxcSolverContactCoulombHeader*)cPtr;
	PxU8* PX_RESTRICT last = desc.constraint + firstHeader->frictionOffset;//getConstraintLength(desc);
	while(cPtr < last)
	{
		const PxcSolverContactCoulombHeader* PX_RESTRICT hdr = (const PxcSolverContactCoulombHeader*)cPtr;
		cPtr += sizeof(PxcSolverContactCoulombHeader);

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		
		//if(cPtr < last)
		//Ps::prefetchLine(cPtr, 512);
		Ps::prefetchLine(cPtr,128);
		Ps::prefetchLine(cPtr,256);
		Ps::prefetchLine(cPtr,384);

		const PxU32 pointStride = hdr->type == PXS_SC_TYPE_EXT_CONTACT ? sizeof(PxcSolverContactExt)
																	   : sizeof(PxcSolverContact);
		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContact *c = reinterpret_cast<PxcSolverContact*>(cPtr);
			cPtr += pointStride;
			c->setScaledBias(FMax(c->getScaledBias(), zero));
		}
	}
	PX_ASSERT(cPtr == last);
}

void  writeBackContactCoulomb(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache,
					  PxcSolverBodyData& bd0, PxcSolverBodyData& bd1)
{

	PxReal normalForce = 0;
	FloatV normalForceV = FZero();

	PxU8* PX_RESTRICT cPtr = desc.constraint;
	PxReal* PX_RESTRICT vForceWriteback = reinterpret_cast<PxReal*>(desc.writeBack);
	const PxcSolverContactCoulombHeader* PX_RESTRICT firstHeader = (const PxcSolverContactCoulombHeader*)cPtr;
	PxU8* PX_RESTRICT last = desc.constraint + firstHeader->frictionOffset;

	const PxU32 pointStride = firstHeader->type == PXS_SC_TYPE_EXT_CONTACT ? sizeof(PxcSolverContactExt)
																	   : sizeof(PxcSolverContact);

	bool hasForceThresholds = false;
	while(cPtr < last)
	{
		const PxcSolverContactCoulombHeader* PX_RESTRICT hdr = (const PxcSolverContactCoulombHeader*)cPtr;
		cPtr += sizeof(PxcSolverContactCoulombHeader);

		hasForceThresholds = hdr->flags & PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS;

		const PxU32 numNormalConstr = hdr->numNormalConstr;

		Ps::prefetchLine(cPtr, 256);
		Ps::prefetchLine(cPtr, 384);

		if(vForceWriteback!=NULL)
		{
			for(PxU32 i=0; i<numNormalConstr; i++)
			{
				PxcSolverContact* c = reinterpret_cast<PxcSolverContact*>(cPtr);
				cPtr += pointStride;
				const FloatV appliedForce = c->getAppliedForce();
				FStore(appliedForce, vForceWriteback);
				vForceWriteback++;
				normalForceV = FAdd(normalForceV, appliedForce);
			}
		}
		else
			cPtr += numNormalConstr * pointStride;
	}
	PX_ASSERT(cPtr == last);

	if(hasForceThresholds && desc.linkIndexA == PxcSolverConstraintDesc::NO_LINK && desc.linkIndexB == PxcSolverConstraintDesc::NO_LINK &&
		normalForce !=0 && (bd0.reportThreshold < PX_MAX_REAL  || bd1.reportThreshold < PX_MAX_REAL))
	{
		PxcThresholdStreamElement elt;
		FStore(normalForceV, &elt.normalForce);
		elt.threshold = PxMin<float>(bd0.reportThreshold, bd1.reportThreshold);
		elt.body0 = bd0.originalBody;
		elt.body1 = bd1.originalBody;
		Ps::order(elt.body0,elt.body1);
		PX_ASSERT(elt.body0 < elt.body1);
		PX_ASSERT(cache.mThresholdStreamIndex<cache.mThresholdStreamLength);
		cache.mThresholdStream[cache.mThresholdStreamIndex++] = elt;
	}

}


void solveFrictionBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveFriction(desc[a], cache);
	}
}


void solveFrictionBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
								PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveFriction(desc[a], cache);
	}
}

void solveFriction_BStaticBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveFriction_BStatic(desc[a], cache);
	}
}


void solveFriction_BStaticConcludeBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveFriction_BStatic(desc[a], cache);
	}
}

void solveFriction_BStaticBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveFriction_BStatic(desc[a], cache);
	}
}


void solveContactCoulombBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveContactCoulomb(desc[a], cache);
	}
}

void solveContactCoulombConcludeBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveContactCoulomb(desc[a], cache);
		concludeContactCoulomb(desc[a], cache);
	}
}

void solveContactCoulombBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
								PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a].bodyBDataIndex];
		solveContactCoulomb(desc[a], cache);
		writeBackContactCoulomb(desc[a], cache, bd0, bd1);
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

void solveContactCoulomb_BStaticBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveContactCoulomb_BStatic(desc[a], cache);
	}
}

void solveContactCoulomb_BStaticConcludeBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveContactCoulomb_BStatic(desc[a], cache);
		concludeContactCoulomb(desc[a], cache);
	}
}

void solveContactCoulomb_BStaticBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a].bodyBDataIndex];
		solveContactCoulomb_BStatic(desc[a], cache);
		writeBackContactCoulomb(desc[a], cache, bd0, bd1);
	}

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


void solveConcludeContactCoulomb			(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solveContactCoulomb(desc, cache);
	concludeContactCoulomb(desc, cache);
}


void solveConcludeContactCoulomb_BStatic	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solveContactCoulomb_BStatic(desc, cache);
	concludeContactCoulomb(desc, cache);
}



}

#endif //PX_SUPPORT_SIMD
