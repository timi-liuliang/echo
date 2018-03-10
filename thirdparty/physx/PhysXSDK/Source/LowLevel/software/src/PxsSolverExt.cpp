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

void writeBackContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache, PxcSolverBodyData& bd0, PxcSolverBodyData& bd1);
void writeBack1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache, PxcSolverBodyData& bd0, PxcSolverBodyData& bd1);

void concludeContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void conclude1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);

void solveExtContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);

//Port of scalar implementation to SIMD maths with some interleaving of instructions
void solveExt1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& /*cache*/)
{
	PxU8* PX_RESTRICT bPtr = desc.constraint;
	//PxU32 length = desc.constraintLength;

	const PxcSolverConstraint1DHeader* PX_RESTRICT  header = reinterpret_cast<const PxcSolverConstraint1DHeader*>(bPtr);
	PxcSolverConstraint1DExt* PX_RESTRICT base = reinterpret_cast<PxcSolverConstraint1DExt*>(bPtr + sizeof(PxcSolverConstraint1DHeader));

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

	Vec3V li0 = V3Zero(), li1 = V3Zero(), ai0 = V3Zero(), ai1 = V3Zero();

	for(PxU32 i=0; i<header->count;++i, base++)
	{
		Ps::prefetchLine(base+1);

		const Vec4V lin0XYZ_constantW						= V4LoadA(&base->lin0.x);	
		const Vec4V lin1XYZ_unbiasedConstantW				= V4LoadA(&base->lin1.x);
		const Vec4V ang0XYZ_velMultiplierW					= V4LoadA(&base->ang0.x);
		const Vec4V ang1XYZ_impulseMultiplierW				= V4LoadA(&base->ang1.x);	
		const Vec4V minImpulseX_maxImpulseY_appliedForceZ	= V4LoadA(&base->minImpulse);

		const Vec3V lin0 = Vec3V_From_Vec4V(lin0XYZ_constantW);				FloatV constant = V4GetW(lin0XYZ_constantW);
		const Vec3V lin1 = Vec3V_From_Vec4V(lin1XYZ_unbiasedConstantW);
		const Vec3V ang0 = Vec3V_From_Vec4V(ang0XYZ_velMultiplierW);		FloatV vMul = V4GetW(ang0XYZ_velMultiplierW);
		const Vec3V ang1 = Vec3V_From_Vec4V(ang1XYZ_impulseMultiplierW);	FloatV iMul = V4GetW(ang1XYZ_impulseMultiplierW);

		const FloatV minImpulse		= V4GetX(minImpulseX_maxImpulseY_appliedForceZ);
		const FloatV maxImpulse		= V4GetY(minImpulseX_maxImpulseY_appliedForceZ);
		const FloatV appliedForce	= V4GetZ(minImpulseX_maxImpulseY_appliedForceZ);

		const Vec3V v0 = V3MulAdd(linVel0, lin0, V3Mul(angVel0, ang0));
		const Vec3V v1 = V3MulAdd(linVel1, lin1, V3Mul(angVel1, ang1));
		const FloatV normalVel = V3SumElems(V3Sub(v0, v1));

		const FloatV unclampedForce = FMulAdd(iMul, appliedForce, FMulAdd(vMul, normalVel, constant));
		const FloatV clampedForce = FMin(maxImpulse, (FMax(minImpulse, unclampedForce)));
		const FloatV deltaF = FSub(clampedForce, appliedForce);

		FStore(clampedForce, &base->appliedForce);
		li0 = V3ScaleAdd(lin0, deltaF, li0);	ai0 = V3ScaleAdd(ang0, deltaF, ai0);
		li1 = V3ScaleAdd(lin1, deltaF, li1);	ai1 = V3ScaleAdd(ang1, deltaF, ai1);

		linVel0 = V3ScaleAdd(base->deltaVA.linear, deltaF, linVel0); 		angVel0 = V3ScaleAdd(base->deltaVA.angular, deltaF, angVel0);
		linVel1 = V3ScaleAdd(base->deltaVB.linear, deltaF, linVel1); 		angVel1 = V3ScaleAdd(base->deltaVB.angular, deltaF, angVel1);
	}

	if(desc.linkIndexA == PxcSolverConstraintDesc::NO_LINK)
	{
		V3StoreA(linVel0, desc.bodyA->linearVelocity);
		V3StoreA(angVel0, desc.bodyA->angularVelocity);
	}
	else
		PxcFsApplyImpulse(*desc.articulationA, desc.linkIndexA, V3Scale(li0, FLoad(header->linearInvMassScale0)),
																V3Scale(ai0, FLoad(header->angularInvMassScale0)));

	if(desc.linkIndexB == PxcSolverConstraintDesc::NO_LINK)
	{
		V3StoreA(linVel1, desc.bodyB->linearVelocity);
		V3StoreA(angVel1, desc.bodyB->angularVelocity);
	}
	else
		PxcFsApplyImpulse(*desc.articulationB, desc.linkIndexB, V3Scale(li1, FLoad(header->linearInvMassScale1)), 
																V3Scale(ai1, FLoad(header->angularInvMassScale1)));
}

void solveExtContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	//We'll need this.
	const FloatV one	= FOne();

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

	const PxU8* PX_RESTRICT last = desc.constraint + desc.constraintLengthOver16*16;

	//hopefully pointer aliasing doesn't bite.
	const PxU8* PX_RESTRICT currPtr = desc.constraint;

	Vec3V linImpulse0 = V3Zero(), linImpulse1 = V3Zero(), angImpulse0 = V3Zero(), angImpulse1 = V3Zero();

	while(currPtr < last)
	{
		const PxcSolverContactHeader* PX_RESTRICT hdr = (PxcSolverContactHeader*)currPtr;
		currPtr += sizeof(PxcSolverContactHeader);

		const PxU32 numNormalConstr = hdr->numNormalConstr;
		const PxU32	numFrictionConstr = hdr->numFrictionConstr;

		PxcSolverContactPointExt* PX_RESTRICT contacts = (PxcSolverContactPointExt*)currPtr;
		Ps::prefetchLine(contacts);
		currPtr += numNormalConstr * sizeof(PxcSolverContactPointExt);

		PxcSolverContactFrictionExt* PX_RESTRICT frictions = (PxcSolverContactFrictionExt*)currPtr;
		currPtr += numFrictionConstr * sizeof(PxcSolverContactFrictionExt);

		FloatV accumulatedNormalImpulse = FZero();

		Vec3V li0 = V3Zero(), li1 = V3Zero(), ai0 = V3Zero(), ai1 = V3Zero();

		for(PxU32 i=0;i<numNormalConstr;i++)
		{
			PxcSolverContactPointExt& c = contacts[i];
			Ps::prefetchLine(&contacts[i+1]);

			const Vec4V normalXYZ_velMultiplierW = c.normalXYZ_velMultiplierW;
			const Vec4V raXnXYZ_appliedForceW = c.raXnXYZ_appliedForceW;
			const Vec4V rbXnXYZ_scaledBiasW = c.rbXnXYZ_scaledBiasW;

			const Vec3V normal = Vec3V_From_Vec4V(normalXYZ_velMultiplierW);
			const Vec3V raXn = Vec3V_From_Vec4V(raXnXYZ_appliedForceW);
			const Vec3V rbXn = Vec3V_From_Vec4V(rbXnXYZ_scaledBiasW);

			const FloatV appliedForce = V4GetW(raXnXYZ_appliedForceW);
			const FloatV velMultiplier = V4GetW(normalXYZ_velMultiplierW);

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
			angVel0 = V3ScaleAdd(Vec3V_From_Vec4V(c.delAngVel0_targetVelocityW), deltaF, angVel0);
			linVel1 = V3ScaleAdd(Vec3V_From_Vec4V(c.linDeltaVB), deltaF, linVel1);	
			angVel1 = V3ScaleAdd(Vec3V_From_Vec4V(c.delAngVel1_maxImpulseW), deltaF, angVel1);

			li0 = V3ScaleAdd(normal, deltaF, li0);	ai0 = V3ScaleAdd(raXn, deltaF, ai0);
			li1 = V3ScaleAdd(normal, deltaF, li1);	ai1 = V3ScaleAdd(rbXn, deltaF, ai1);

			c.setAppliedForce(FAdd(appliedForce, deltaF));

			accumulatedNormalImpulse = FAdd(FAdd(accumulatedNormalImpulse, appliedForce), deltaF);
		}


		if(cache.doFriction && numFrictionConstr)
		{
			Ps::prefetchLine(frictions);
			const FloatV maxFrictionImpulse = FMul(hdr->getStaticFriction(), accumulatedNormalImpulse);
			const FloatV maxDynFrictionImpulse = FMul(hdr->getDynamicFriction(), accumulatedNormalImpulse);

			for(PxU32 i=0;i<numFrictionConstr;i++)
			{
				PxcSolverContactFrictionExt& f = frictions[i];
				Ps::prefetchLine(&frictions[i+1]);

				const Vec4V normalXYZ_appliedForceW = f.normalXYZ_appliedForceW;
				const Vec4V raXnXYZ_velMultiplierW = f.raXnXYZ_velMultiplierW;
				const Vec4V rbXnXYZ_biasW = f.rbXnXYZ_biasW;

				const Vec3V normal = Vec3V_From_Vec4V(normalXYZ_appliedForceW);
				const Vec3V raXn = Vec3V_From_Vec4V(raXnXYZ_velMultiplierW);
				const Vec3V rbXn = Vec3V_From_Vec4V(rbXnXYZ_biasW);

				const FloatV appliedForce = V4GetW(normalXYZ_appliedForceW);
				const FloatV bias = V4GetW(rbXnXYZ_biasW);
				const FloatV velMultiplier = V4GetW(raXnXYZ_velMultiplierW);

				const FloatV targetVel = V4GetW(f.delAngVel1_TargetVelW);

				const FloatV negMaxDynFrictionImpulse = FNeg(maxDynFrictionImpulse);
				const FloatV negMaxFrictionImpulse = FNeg(maxFrictionImpulse);

				const Vec3V v0 = V3MulAdd(linVel0, normal, V3Mul(angVel0, raXn));
				const Vec3V v1 = V3MulAdd(linVel1, normal, V3Mul(angVel1, rbXn));
				const FloatV normalVel = V3SumElems(V3Sub(v0, v1));

				// appliedForce -bias * velMultiplier - a hoisted part of the total impulse computation
				const FloatV tmp1 = FNegMulSub(FSub(bias, targetVel),velMultiplier,appliedForce); 

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

				const BoolV clampLow = FIsGrtr(negMaxFrictionImpulse, totalImpulse);
				const BoolV clampHigh = FIsGrtr(totalImpulse, maxFrictionImpulse);

				const FloatV totalClampedLow = FMax(negMaxDynFrictionImpulse, totalImpulse);
				const FloatV totalClampedHigh = FMin(maxDynFrictionImpulse, totalImpulse);

				const FloatV newAppliedForce = FSel(clampLow, totalClampedLow,
															  FSel(clampHigh, totalClampedHigh, totalImpulse));

				const FloatV broken = FSel(clampLow, one, FSel(clampHigh, one, oldBroken));

				FloatV deltaF = FSub(newAppliedForce, appliedForce);

				linVel0 = V3ScaleAdd(Vec3V_From_Vec4V(f.linDeltaVA), deltaF, linVel0);	
				angVel0 = V3ScaleAdd(Vec3V_From_Vec4V(f.delAngVel0_BrokenW), deltaF, angVel0);
				linVel1 = V3ScaleAdd(Vec3V_From_Vec4V(f.linDeltaVB), deltaF, linVel1);	
				angVel1 = V3ScaleAdd(Vec3V_From_Vec4V(f.delAngVel1_TargetVelW), deltaF, angVel1);

				li0 = V3ScaleAdd(normal, deltaF, li0);	ai0 = V3ScaleAdd(raXn, deltaF, ai0);
				li1 = V3ScaleAdd(normal, deltaF, li1);	ai1 = V3ScaleAdd(rbXn, deltaF, ai1);

				f.setAppliedForce(newAppliedForce);

				f.delAngVel0_BrokenW = V4SetW(f.delAngVel0_BrokenW, broken);
			}
		}

		linImpulse0 = V3ScaleAdd(li0, hdr->getDominance0(), linImpulse0);		
		angImpulse0 = V3ScaleAdd(ai0, FLoad(hdr->angDom0), angImpulse0);
		linImpulse1 = V3NegScaleSub(li1, hdr->getDominance1(), linImpulse1);	
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


void solveExtContactBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveExtContact(desc[a], cache);
	}
}

void solveExtContactConcludeBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveExtContact(desc[a], cache);
		concludeContact(desc[a], cache);
	}
}

void solveExtContactBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
								   PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a].linkIndexA != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a].linkIndexB != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyBDataIndex];

		solveExtContact(desc[a], cache);
		writeBackContact(desc[a], cache, bd0, bd1);
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

void extContactBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
								   PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 /*thresholdStreamLength*/, PxI32* outThresholdPairs)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a].linkIndexA != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a].linkIndexB != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyBDataIndex];
		writeBackContact(desc[a], cache, bd0, bd1);
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

void solveExt1DBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveExt1D(desc[a], cache);
	}
}

void solveExt1DConcludeBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		solveExt1D(desc[a], cache);
		conclude1D(desc[a], cache);
	}
}

void solveExt1DBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
							  PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a].linkIndexA != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a].linkIndexB != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyBDataIndex];
		solveExt1D(desc[a], cache);
		writeBack1D(desc[a], cache, bd0, bd1);
	}
}

void ext1DBlockWriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
							  PxcThresholdStreamElement* PX_RESTRICT /*thresholdStream*/, const PxU32 /*thresholdStreamLength*/, PxI32* /*outThresholdPairs*/)
{
	for(PxU32 a = 0; a < constraintCount; ++a)
	{
		PxcSolverBodyData& bd0 = cache.solverBodyArray[desc[a].linkIndexA != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyADataIndex];
		PxcSolverBodyData& bd1 = cache.solverBodyArray[desc[a].linkIndexB != PxcSolverConstraintDesc::NO_LINK ? 0 : desc[a].bodyBDataIndex];
		writeBack1D(desc[a], cache, bd0, bd1);
	}
}

}
