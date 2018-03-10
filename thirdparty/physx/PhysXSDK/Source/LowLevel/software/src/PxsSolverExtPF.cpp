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
#include "PxsSolverExt.h"
#include "PxcSolverConstraintDesc.h"
#include "PxcSolverConstraint1D.h"
#include "PxsSolverContact.h"
#include "PxsSolverContext.h"
#include "PxcArticulation.h"
#include "PsAtomic.h"
#include "PxcThresholdStreamElement.h"

namespace physx
{  

void writeBackContactCoulomb(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache, PxcSolverBodyData& bd0, PxcSolverBodyData& bd1);
void concludeContactCoulomb(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);

void solveExtContactCoulomb(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	//We'll need this.
//	const FloatV zero	= FZero();
//	const FloatV one	= FOne();

	Vec3V linVel0, angVel0, linVel1, angVel1;

	if(desc.linkIndexA == PxcSolverConstraintDesc::NO_LINK)
	{
		linVel0 = V3LoadA(desc.bodyA->linearVelocity);
		angVel0 = V3LoadA(desc.bodyA->angularVelocity);
	}
	else
	{
		PxcSIMDSpatial v = PxcFsGetVelocity(*desc.articulationA, desc.linkIndexA);
		linVel0 = v.linear;
		angVel0 = v.angular;
	}

	if(desc.linkIndexB == PxcSolverConstraintDesc::NO_LINK)
	{
		linVel1 = V3LoadA(desc.bodyB->linearVelocity);
		angVel1 = V3LoadA(desc.bodyB->angularVelocity);
	}
	else
	{
		PxcSIMDSpatial v = PxcFsGetVelocity(*desc.articulationB, desc.linkIndexB);
		linVel1 = v.linear;
		angVel1 = v.angular;
	}

	//const PxU8* PX_RESTRICT last = desc.constraint + desc.constraintLengthOver16*16;

	const PxU8* PX_RESTRICT currPtr = desc.constraint;

	const PxcSolverContactCoulombHeader* PX_RESTRICT firstHeader = (PxcSolverContactCoulombHeader*)currPtr;

	const PxU8* PX_RESTRICT last = desc.constraint + firstHeader->frictionOffset;

	//hopefully pointer aliasing doesn't bite.
	

	Vec3V linImpulse0 = V3Zero(), linImpulse1 = V3Zero(), angImpulse0 = V3Zero(), angImpulse1 = V3Zero();

	while(currPtr < last)
	{
		const PxcSolverContactCoulombHeader* PX_RESTRICT hdr = (PxcSolverContactCoulombHeader*)currPtr;
		currPtr += sizeof(PxcSolverContactCoulombHeader);

		const PxU32 numNormalConstr = hdr->numNormalConstr;

		PxF32* appliedImpulse = (PxF32*) (((PxU8*)hdr) + hdr->frictionOffset + sizeof(PxcSolverFrictionHeader));
		Ps::prefetchLine(appliedImpulse);
		
		PxcSolverContactExt* PX_RESTRICT contacts = (PxcSolverContactExt*)currPtr;
		Ps::prefetchLine(contacts);
		currPtr += numNormalConstr * sizeof(PxcSolverContactExt);

		Vec3V li0 = V3Zero(), li1 = V3Zero(), ai0 = V3Zero(), ai1 = V3Zero();

		const Vec3V normal = hdr->getNormal();

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContactExt& c = contacts[i];
			Ps::prefetchLine(&contacts[i+1]);

			const Vec4V rbXnXYZ_velMultiplierW = c.rbXnXYZ_velMultiplierW;
			const Vec4V raXnXYZ_appliedForceW = c.raXnXYZ_appliedForceW;

			const Vec3V raXn = Vec3V_From_Vec4V(raXnXYZ_appliedForceW);
			const Vec3V rbXn = Vec3V_From_Vec4V(rbXnXYZ_velMultiplierW);

			const FloatV appliedForce = V4GetW(raXnXYZ_appliedForceW);
			const FloatV velMultiplier = V4GetW(rbXnXYZ_velMultiplierW);

			const FloatV targetVel = c.getTargetVelocity();
			const FloatV scaledBias = c.getScaledBias();

			//Compute the normal velocity of the constraint.

			const Vec3V v0 = V3MulAdd(linVel0, normal, V3Mul(angVel0, raXn));
			const Vec3V v1 = V3MulAdd(linVel1, normal, V3Mul(angVel1, rbXn));
			const FloatV normalVel = V3SumElems(V3Sub(v0, v1));

			const FloatV unbiasedErr = FMul(targetVel, velMultiplier);
			const FloatV biasedErr = FSub(unbiasedErr, scaledBias);

			// still lots to do here: using loop pipelining we can interweave this code with the
			// above - the code here has a lot of stalls that we would thereby eliminate

			const FloatV deltaF = FMax(FNegMulSub(normalVel, velMultiplier, biasedErr), FNeg(appliedForce));

			linVel0 = V3ScaleAdd(Vec3V_From_Vec4V(c.linDeltaVA), deltaF, linVel0);	
			angVel0 = V3ScaleAdd(Vec3V_From_Vec4V(c.delAngVel0_InvMassADom), deltaF, angVel0);
			linVel1 = V3ScaleAdd(Vec3V_From_Vec4V(c.linDeltaVB), deltaF, linVel1);	
			angVel1 = V3ScaleAdd(Vec3V_From_Vec4V(c.delAngVel1_InvMassBDom), deltaF, angVel1);

			li0 = V3ScaleAdd(normal, deltaF, li0);	ai0 = V3ScaleAdd(raXn, deltaF, ai0);
			li1 = V3ScaleAdd(normal, deltaF, li1);	ai1 = V3ScaleAdd(rbXn, deltaF, ai1);

			const FloatV newAppliedForce = FAdd(appliedForce, deltaF);
			c.setAppliedForce(newAppliedForce);

			FStore(newAppliedForce, &appliedImpulse[i]);
			Ps::prefetchLine(&appliedImpulse[i], 128);
		}

		linImpulse0 = V3ScaleAdd(li0, FLoad(hdr->dominance0), linImpulse0);		
		angImpulse0 = V3ScaleAdd(ai0, FLoad(hdr->angDom0), angImpulse0);
		linImpulse1 = V3NegScaleSub(li1, FLoad(hdr->dominance1), linImpulse1);	
		angImpulse1 = V3NegScaleSub(ai1, FLoad(hdr->angDom1), angImpulse1);
	}

	if(desc.linkIndexA == PxcSolverConstraintDesc::NO_LINK)
	{
		V3StoreA(linVel0, desc.bodyA->linearVelocity);
		V3StoreA(angVel0, desc.bodyA->angularVelocity);
	}
	else
		PxcFsApplyImpulse(*desc.articulationA, desc.linkIndexA, linImpulse0, angImpulse0);

	if(desc.linkIndexB == PxcSolverConstraintDesc::NO_LINK)
	{
		V3StoreA(linVel1, desc.bodyB->linearVelocity);
		V3StoreA(angVel1, desc.bodyB->angularVelocity);
	}
	else
		PxcFsApplyImpulse(*desc.articulationB, desc.linkIndexB, linImpulse1, angImpulse1);

	PX_ASSERT(currPtr == last);
}

void solveExtFriction(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
//	const FloatV zero	= FZero();
//	const FloatV one	= FOne();

	Vec3V linVel0, angVel0, linVel1, angVel1;

	if(desc.linkIndexA == PxcSolverConstraintDesc::NO_LINK)
	{
		linVel0 = V3LoadA(desc.bodyA->linearVelocity);
		angVel0 = V3LoadA(desc.bodyA->angularVelocity);
	}
	else
	{
		PxcSIMDSpatial v = PxcFsGetVelocity(*desc.articulationA, desc.linkIndexA);
		linVel0 = v.linear;
		angVel0 = v.angular;
	}

	if(desc.linkIndexB == PxcSolverConstraintDesc::NO_LINK)
	{
		linVel1 = V3LoadA(desc.bodyB->linearVelocity);
		angVel1 = V3LoadA(desc.bodyB->angularVelocity);
	}
	else
	{
		PxcSIMDSpatial v = PxcFsGetVelocity(*desc.articulationB, desc.linkIndexB);
		linVel1 = v.linear;
		angVel1 = v.angular;
	}


	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc.constraint;

	const PxU8* PX_RESTRICT last = currPtr + desc.constraintLengthOver16*16;

	Vec3V linImpulse0 = V3Zero(), linImpulse1 = V3Zero(), angImpulse0 = V3Zero(), angImpulse1 = V3Zero();

	while(currPtr < last)
	{
	
		const PxcSolverFrictionHeader* PX_RESTRICT frictionHeader = (PxcSolverFrictionHeader*)currPtr;
		currPtr += sizeof(PxcSolverFrictionHeader);
		PxF32* appliedImpulse = (PxF32*)currPtr;
		currPtr += frictionHeader->getAppliedForcePaddingSize();

		PxcSolverFrictionExt* PX_RESTRICT frictions = (PxcSolverFrictionExt*)currPtr;
		const PxU32 numFrictionConstr = frictionHeader->numFrictionConstr;

		currPtr += numFrictionConstr * sizeof(PxcSolverContactFrictionExt);
		const FloatV staticFriction = frictionHeader->getStaticFriction();
	
	
		Vec3V li0 = V3Zero(), li1 = V3Zero(), ai0 = V3Zero(), ai1 = V3Zero();

		for(PxU32 i=0;i<numFrictionConstr;i++)
		{

			PxcSolverFrictionExt& f = frictions[i];
			Ps::prefetchLine(&frictions[i+1]);
		

			const Vec3V t0 = Vec3V_From_Vec4V(f.normalXYZ_appliedForceW);
			const Vec3V raXt0 = Vec3V_From_Vec4V(f.raXnXYZ_velMultiplierW);
			const Vec3V rbXt0 = Vec3V_From_Vec4V(f.rbXnXYZ_targetVelocityW);

			const FloatV appliedForce = V4GetW(f.normalXYZ_appliedForceW);
			const FloatV velMultiplier = V4GetW(f.raXnXYZ_velMultiplierW);

			const FloatV normalImpulse = FLoad(appliedImpulse[f.contactIndex]);//contacts[f.contactIndex].getAppliedForce();
			const FloatV maxFriction = FMul(staticFriction, normalImpulse);
			const FloatV nMaxFriction = FNeg(maxFriction);

			//Compute the normal velocity of the constraint.

			const FloatV t0Vel1 = V3Dot(t0, linVel0);
			const FloatV t0Vel2 = V3Dot(raXt0, angVel0);
			const FloatV t0Vel3 = V3Dot(t0, linVel1);
			const FloatV t0Vel4 = V3Dot(rbXt0, angVel1);

			//const FloatV unbiasedErr = FMul(targetVel, velMultiplier);
			//const FloatV biasedErr = FMulAdd(targetVel, velMultiplier, nScaledBias);

			const FloatV t0Vel = FSub(FAdd(t0Vel1, t0Vel2), FAdd(t0Vel3, t0Vel4));

			// still lots to do here: using loop pipelining we can interweave this code with the
			// above - the code here has a lot of stalls that we would thereby eliminate

			//FloatV deltaF = FSub(scaledBias, FMul(t0Vel, velMultiplier));//FNeg(FMul(t0Vel, velMultiplier));
			FloatV deltaF = FNeg(FMul(t0Vel, velMultiplier));
			FloatV newForce = FAdd(appliedForce, deltaF);
			newForce = FClamp(newForce, nMaxFriction, maxFriction);
			deltaF = FSub(newForce, appliedForce);

			linVel0 = V3ScaleAdd(Vec3V_From_Vec4V(f.linDeltaVA), deltaF, linVel0);	
			angVel0 = V3ScaleAdd(Vec3V_From_Vec4V(f.delAngVel0_InvMassADom), deltaF, angVel0);
			linVel1 = V3ScaleAdd(Vec3V_From_Vec4V(f.linDeltaVB), deltaF, linVel1);	
			angVel1 = V3ScaleAdd(Vec3V_From_Vec4V(f.delAngVel1_InvMassBDom), deltaF, angVel1);

			li0 = V3ScaleAdd(t0, deltaF, li0);	ai0 = V3ScaleAdd(raXt0, deltaF, ai0);
			li1 = V3ScaleAdd(t0, deltaF, li1);	ai1 = V3ScaleAdd(rbXt0, deltaF, ai1);

			f.setAppliedForce(newForce);
		}


		linImpulse0 = V3ScaleAdd(li0, FLoad(frictionHeader->dominance0), linImpulse0);		
		angImpulse0 = V3ScaleAdd(ai0, FLoad(frictionHeader->angDom0), angImpulse0);
		linImpulse1 = V3NegScaleSub(li1, FLoad(frictionHeader->dominance1), linImpulse1);	
		angImpulse1 = V3NegScaleSub(ai1, FLoad(frictionHeader->angDom1), angImpulse1);
	}

	if(desc.linkIndexA == PxcSolverConstraintDesc::NO_LINK)
	{
		V3StoreA(linVel0, desc.bodyA->linearVelocity);
		V3StoreA(angVel0, desc.bodyA->angularVelocity);
	}
	else
		PxcFsApplyImpulse(*desc.articulationA, desc.linkIndexA, linImpulse0, angImpulse0);

	if(desc.linkIndexB == PxcSolverConstraintDesc::NO_LINK)
	{
		V3StoreA(linVel1, desc.bodyB->linearVelocity);
		V3StoreA(angVel1, desc.bodyB->angularVelocity);
	}
	else
		PxcFsApplyImpulse(*desc.articulationB, desc.linkIndexB, linImpulse1, angImpulse1);

	PX_ASSERT(currPtr == last);

}

void solveExtFrictionBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveExtFriction(desc[a], cache);
	}
}

void solveExtFrictionConcludeBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveExtFriction(desc[a], cache);
	}
}

void solveExtFrictionBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
								   PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveExtFriction(desc[a], cache);
	}
}


void solveConcludeExtContactCoulomb		(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solveExtContactCoulomb(desc, cache);
	concludeContactCoulomb(desc, cache);
}

void solveExtContactCoulombBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveExtContactCoulomb(desc[a], cache);
	}
}

void solveExtContactCoulombConcludeBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveExtContactCoulomb(desc[a], cache);
		concludeContactCoulomb(desc[a], cache);
	}
}

void solveExtContactCoulombBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
								   PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a].linkIndexA != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a].linkIndexB != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyBDataIndex];

		solveExtContactCoulomb(desc[a], cache);
		writeBackContactCoulomb(desc[a], cache, bd0, bd1);
	}
	if(cache.mThresholdStreamIndex > 0)
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

}
