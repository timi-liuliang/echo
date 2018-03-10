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


// suppress LNK4221
#include "PxPreprocessor.h"
PX_DUMMY_SYMBOL

#include "PsVecMath.h"
#ifndef PX_SUPPORT_SIMD 

#include "CmPhysXCommon.h"
#include "PsUtilities.h"
#include "PxcSolverBody.h"
#include "PxcSolverContact.h"
#include "PxcSolverConstraint1D.h"
#include "PxcSolverConstraintDesc.h"
#include "PxcThresholdStreamElement.h"
#include "PxsSolverCoreGeneral.h"

// ==============================================================

namespace
{
	void solve1DRBody(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache, PxcSolverBody& b0, PxcSolverBody& b1)
	{
		PxU8* bPtr = desc.constraint;
		//PxU32 length = desc.constraintLength;

		PxcSolverConstraint1DHeader* header = reinterpret_cast<PxcSolverConstraint1DHeader *>(bPtr);
		PxcSolverConstraint1D* const base = reinterpret_cast<PxcSolverConstraint1D*>(bPtr + sizeof(PxcSolverConstraint1DHeader));

		PxReal d0 = dominance0(header->dominance), d1 = dominance1(header->dominance);

		for(PxU32 i=0; i<header->count;++i)
		{
			PxcSolverConstraint1D& c = base[i];

			const PxReal normalVel =  b0.projectVelocity(c.lin0, c.ang0) 
									- b1.projectVelocity(c.lin1, c.ang1);

			const PxReal deltaVF = (c.targetVelocity - normalVel) * c.velMultiplier;

			c.appliedVForce += c.solverExtrapolation * (deltaVF - c.appliedVForce * c.impulseMultiplier);

			PxReal deltaF = c.solverExtrapolation * (deltaVF - c.bias * c.posMultiplier 
				- c.appliedForce * c.impulseMultiplier);

			const PxReal newForce = PxMin(c.maxImpulse, PxMax(c.minImpulse, c.appliedForce + deltaF));
			deltaF = newForce - c.appliedForce;
			c.appliedForce = newForce;

			const PxReal deltaF0 = deltaF * d0;
			b0.applyImpulse(c.lin0 * deltaF0, c.ang0 * deltaF0);

			const PxReal deltaF1 = -deltaF * d1;
			b1.applyImpulse(c.lin1 * deltaF1, c.ang1 * deltaF1);
		}
	}


}

void solve1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solve1DRBody(desc, cache, *desc.bodyA, *desc.bodyB);
}


void conclude1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	PxU8* bPtr = desc.constraint;
	//PxU32 length = desc.constraintLength;

	PxcSolverConstraint1DHeader* header = reinterpret_cast<PxcSolverConstraint1DHeader *>(bPtr);
	PxcSolverConstraint1D* const base = reinterpret_cast<PxcSolverConstraint1D*>(bPtr + sizeof(PxcSolverConstraint1DHeader));

	for(PxU32 i=0; i<header->count; i++)
	{
		PxcSolverConstraint1D& c = base[i];

		if(!(c.flags & PXS_SC_FLAG_KEEP_BIAS))
			c.bias = 0.0f;

		// The impulse produced by the position change, i.e., bias, should be discarded in the last iteration
		// step. Hence, the impulse term used for the implicit spring should take only the impulse of the
		// velocity change (appliedVForce) into account. Since we want to use the same solver routine for the
		// last iteration, we adjust the impulse multiplier such that only the velocity change impulse is
		// regarded.
		// Note: This is a bit of a hack, i.e., the adjusted impulse multiplier is correct for computing
		//       the applied total impulse (appliedForce) but introduces an error for the total velocity
		//       change impulse (appliedVForce). This could be fixed by having a separate impulse multiplier
		//       for the velocity change impulse, but the constraint structure would have to be enlarged further.

		if (PxAbs(c.appliedForce) > 1e-10f)
			c.impulseMultiplier *= c.appliedVForce / c.appliedForce;

		// We don't want to over compensate when we remove the impulse that produced the position change
		// so we need to remove the over-relaxation on this last iteration.
		c.solverExtrapolation = 1.0f;
	}
}

// ==============================================================

namespace 
{

	template <class Body>
	PX_FORCE_INLINE
	void solveContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache, Body &b0, Body &b1)
	{
		PxU8* cPtr = desc.constraint;
		PxU32 length = getConstraintLength(desc);

		//hopefully pointer aliasing doesn't bite.
		PxU8* currPtr = cPtr;

		while(currPtr < cPtr + length)
		{
			const PxcSolverContactHeader* PX_RESTRICT hdr = (PxcSolverContactHeader*)currPtr;
			currPtr += sizeof(PxcSolverContactHeader);

			PxcSolverContactPoint* PX_RESTRICT contacts = (PxcSolverContactPoint*)currPtr;
			currPtr += hdr->numNormalConstr * sizeof(PxcSolverContactPoint);

			PxcSolverContactFriction* PX_RESTRICT frictions = (PxcSolverContactFriction*)currPtr;
			currPtr += hdr->numFrictionConstr * sizeof(PxcSolverContactFriction);

			//PxReal dom0 = dominance0(hdr->flags), dom1 = dominance1(hdr->flags);
			PxReal dom0 = hdr->getDominance0PxF32();
			PxReal dom1 = hdr->getDominance1PxF32();
			PxReal accumulatedNormalImpulse = 0.0f;

			for(PxU32 i=0;i<hdr->numNormalConstr;i++)
			{
				PxcSolverContactPoint& c = contacts[i];

				//const PxReal normalVel = b0.projectVelocity(c.normal,c.raXn)
				//                       - b1.projectVelocity(c.normal,c.rbXn);
				const PxReal normalVel = b0.projectVelocity(c.getNormalPxVec3(), c.getRaXnPxVec3())
					                   - b1.projectVelocity(c.getNormalPxVec3(), c.getRbXnPxVec3());

				//const PxReal deltaVF = (c.targetVelocity - normalVel) * c.velMultiplier;
				const PxReal deltaVF = (c.getTargetVelocityPxF32() - normalVel) * c.getVelMultiplierPxF32();

				//c.appliedVForce += deltaVF;
				c.setAppliedVForce(c.getAppliedVForcePxF32() + deltaVF);

				//PxReal deltaF = deltaVF - c.scaledBias;
				PxReal deltaF = deltaVF - c.getScaledBiasPxF32();
				//const PxReal newForce = PxMax(c.appliedForce + deltaF,0.0f);
				const PxReal newForce = PxMax(c.getAppliedForcePxF32() + deltaF,0.0f);
				//deltaF = newForce - c.appliedForce;
				deltaF = newForce - c.getAppliedForcePxF32();
				//c.appliedForce = newForce;
				c.setAppliedForce(newForce);

				const PxReal deltaF0 = deltaF * dom0;
				//b0.applyImpulse(c.normal * deltaF0, c.raXn * deltaF0);
				b0.applyImpulse(c.getNormalPxVec3() * deltaF0, c.getRaXnPxVec3() * deltaF0);

				const PxReal deltaF1 = -deltaF * dom1;
				//b1.applyImpulse(c.normal * deltaF1, c.rbXn * deltaF1);
				b1.applyImpulse(c.getNormalPxVec3() * deltaF1, c.getRbXnPxVec3() * deltaF1);

				//accumulatedNormalImpulse += c.appliedForce;
				accumulatedNormalImpulse += c.getAppliedForcePxF32();
			}


			//if((cache.mIteration <= 1) && accumulatedNormalImpulse != 0)
			if(cache.doFriction && hdr->numFrictionConstr)
			{
				//const PxReal maxFrictionImpulse = hdr->staticFriction * accumulatedNormalImpulse;
				const PxReal maxFrictionImpulse = hdr->getStaticFrictionPxF32() * accumulatedNormalImpulse;
				//const PxReal maxDynFrictionImpulse = hdr->dynamicFriction * accumulatedNormalImpulse;
				const PxReal maxDynFrictionImpulse = hdr->getDynamicFrictionPxF32() * accumulatedNormalImpulse;

				for(PxU32 i=0;i<hdr->numFrictionConstr;i++)
				{
					PxcSolverContactFriction &f = frictions[i];
					//const PxReal normalVel = b0.projectVelocity(f.normal,f.raXn)
					//					   - b1.projectVelocity(f.normal,f.rbXn);
					const PxReal normalVel = b0.projectVelocity(f.getNormalPxVec3(), f.getRaXnPxVec3())
										   - b1.projectVelocity(f.getNormalPxVec3(), f.getRbXnPxVec3());

					//PxReal deltaF = -(f.bias + normalVel) * f.velMultiplier;
					PxReal deltaF = -(f.getBiasPxF32() + normalVel) * f.getVelMultiplierPxF32();

					//PxReal potentialSumF = f.appliedForce + deltaF;
					PxReal potentialSumF = f.getAppliedForcePxF32() + deltaF;

					const PxReal absPotentialSumF = PxAbs(potentialSumF);

					if(absPotentialSumF > maxFrictionImpulse)
					{
						potentialSumF *= (maxDynFrictionImpulse / absPotentialSumF);

						//deltaF = potentialSumF -  f.appliedForce;
						deltaF = potentialSumF -  f.getAppliedForcePxF32();
						f.broken = 1;
					}

					//f.appliedForce = potentialSumF;
					f.setAppliedForce(potentialSumF);

					const PxReal deltaF0 = deltaF * dom0;
					//b0.applyImpulse(f.normal * deltaF0, f.raXn * deltaF0);
					b0.applyImpulse(f.getNormalPxVec3() * deltaF0, f.getRaXnPxVec3() * deltaF0);

					const PxReal deltaF1 = -deltaF * dom1;
					//b1.applyImpulse(f.normal * deltaF1, f.rbXn * deltaF1);
					b1.applyImpulse(f.getNormalPxVec3() * deltaF1, f.getRbXnPxVec3() * deltaF1);
				}
			}

		}

		PX_ASSERT(currPtr == cPtr + length);
	}
}

void solveContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	solveContact(desc,cache,*desc.bodyA, *desc.bodyB);
}

void concludeContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	PxU8* cPtr = desc.constraint;
	while(cPtr < desc.constraint + getConstraintLength(desc))
	{
		const PxcSolverContactHeader* PX_RESTRICT hdr = (PxcSolverContactHeader*)cPtr;
		cPtr += sizeof(PxcSolverContactHeader);

		PxcSolverContactPoint* PX_RESTRICT contacts = (PxcSolverContactPoint*)cPtr;
		cPtr += hdr->numNormalConstr * sizeof(PxcSolverContactPoint);

		PxcSolverContactFriction* PX_RESTRICT frictions = (PxcSolverContactFriction*)cPtr;
		cPtr += hdr->numFrictionConstr * sizeof(PxcSolverContactFriction);

		for(PxU32 i=0;i<hdr->numNormalConstr;i++)
			//contacts[i].scaledBias = PxMax(contacts[i].scaledBias, 0.0f);
			//V3WriteX(contacts[i].scaledBiasX_targetVelocityY_restitutionZ, PxMax(V3ReadX(contacts[i].scaledBiasX_targetVelocityY_restitutionZ), 0.0f));
			contacts[i].setScaledBias(PxMax(V3ReadX(
				contacts[i].getScaledBias()), 0.0f));

		for(PxU32 i=0;i<hdr->numFrictionConstr;i++)
			//frictions[i].bias = 0.0f;
			//V4WriteW(frictions[i].rbXnXYZ_biasW, 0.0f);
			frictions[i].setBias(0.0f);
	}
	PX_ASSERT(cPtr == desc.constraint + getConstraintLength(desc));
}

void writeBack1D(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	PxcSolverConstraint1DHeader* header = reinterpret_cast<PxcSolverConstraint1DHeader *>(desc.constraint);
	PxcSolverConstraint1D* const base = reinterpret_cast<PxcSolverConstraint1D*>(desc.constraint + sizeof(PxcSolverConstraint1DHeader));
	PxsSolverConstraintWriteback* writeback = reinterpret_cast<PxsSolverConstraintWriteback *>(desc.writeBack);
	if(writeback)
	{
		for(PxU32 i=0; i<header->count; i++)
		{
			writeback[i].linear = base[i].lin0;
			writeback[i].vForce = base[i].appliedVForce;
			writeback[i].angular = base[i].ang0;
			writeback[i].flags = base[i].flags;
		}
	}
}

void writeBackContact(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache)
{
	PxcSolverBody* b0 = desc.bodyA, *b1 = desc.bodyB;

	PxReal normalForce = 0;

	PxU8* cPtr = desc.constraint;
	PxReal* vForceWriteback = reinterpret_cast<PxReal*>(desc.writeBack);
	while(cPtr < desc.constraint + getConstraintLength(desc))
	{
		const PxcSolverContactHeader* PX_RESTRICT hdr = (PxcSolverContactHeader*)cPtr;
		cPtr += sizeof(PxcSolverContactHeader);

		PxcSolverContactPoint* PX_RESTRICT contacts = (PxcSolverContactPoint*)cPtr;
		cPtr += hdr->numNormalConstr * sizeof(PxcSolverContactPoint);

		PxcSolverContactFriction* PX_RESTRICT frictions = (PxcSolverContactFriction*)cPtr;
		cPtr += hdr->numFrictionConstr * sizeof(PxcSolverContactFriction);

		if(vForceWriteback!=NULL)
		{
			for(PxU32 i=0; i<hdr->numNormalConstr; i++)
			{
				//*vForceWriteback++ = contacts[i].appliedVForce;
				*vForceWriteback++ = V4ReadW(contacts[i].rbXnXYZ_appliedVForceW);
				//normalForce += contacts[i].appliedVForce;
				normalForce += V4ReadW(contacts[i].rbXnXYZ_appliedVForceW);
			}
		}

		for(PxU32 i=0; i<hdr->numFrictionConstr; i++)
		{
			if((frictions[i].frictionBrokenWritebackByte != NULL) && frictions[i].broken)
				*(frictions[i].frictionBrokenWritebackByte) = 1;
		}
	}
	PX_ASSERT(cPtr == desc.constraint + desc.constraintLength);

	if(desc.linkIndexA == PxcSolverConstraintDesc::NO_LINK && desc.linkIndexB == PxcSolverConstraintDesc::NO_LINK &&
		normalForce !=0 && (b0->reportThreshold < PX_MAX_REAL  || b1->reportThreshold < PX_MAX_REAL))
	{
		PxcThresholdStreamElement elt;
		elt.normalForce = normalForce;
		elt.threshold = PxMin<float>(b0->reportThreshold, b1->reportThreshold);
		elt.body0 = b0->originalBody;
		elt.body1 = b1->originalBody;
		Ps::order(elt.body0,elt.body1);
		PX_ASSERT(cache.mThresholdStreamIndex<cache.mThresholdStreamLength);
		cache.mThresholdStream[cache.mThresholdStreamIndex++] = elt;
	}

}


#endif
