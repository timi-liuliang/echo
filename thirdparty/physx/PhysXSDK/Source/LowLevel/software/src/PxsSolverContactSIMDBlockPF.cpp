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

#include "PxcSolverBody.h"
#include "PxcSolverContact.h"
#include "PxcSolverContactPF.h"
#include "PxcSolverConstraintTypes.h"
#include "PxcSolverConstraintDesc.h"
#include "PsMathUtils.h"
#include "PxsSolverContact.h"
#include "PxContactModifyCallback.h"
#include "PxcNpWorkUnit.h"
#include "PxsMaterialManager.h"
#include "PxsMaterialCombiner.h"
#include "PxcNpContactPrep.h"
#include "PxcSolverContact4.h"
#include "PxcSolverContactPF4.h"

//#ifdef	__SPU__
//#include "CellUtil.h"
//#endif

using namespace physx;

struct CorrelationListIteratorCoulomb
{
	PxcCorrelationBufferCoulomb& buffer;
	PxU32 currPatch;
	PxU32 currContact;

	CorrelationListIteratorCoulomb(PxcCorrelationBufferCoulomb& correlationBuffer, PxU32 startPatch) : buffer(correlationBuffer)
	{
		//We need to force us to advance the correlation buffer to the first available contact (if one exists)
		PxU32 newPatch = startPatch, newContact = 0;

		while(newPatch != PxcCorrelationBuffer::LIST_END && newContact == buffer.contactPatches[newPatch].count)
		{
			newPatch = buffer.contactPatches[newPatch].next;
			newContact = 0;
		}

		currPatch = newPatch;
		currContact = newContact;
	}

	//Returns true if it has another contact pre-loaded. Returns false otherwise
	PX_FORCE_INLINE bool hasNextContact()
	{
		return (currPatch != PxcCorrelationBuffer::LIST_END && currContact < buffer.contactPatches[currPatch].count);
	}

	void nextContact(PxU32& patch, PxU32& contact)
	{
		PX_ASSERT(currPatch != PxcCorrelationBuffer::LIST_END);
		PX_ASSERT(currContact < buffer.contactPatches[currPatch].count);

		patch = currPatch;
		contact = currContact;
		PxU32 newPatch = currPatch, newContact = currContact + 1;

		while(newPatch != PxcCorrelationBuffer::LIST_END && newContact == buffer.contactPatches[newPatch].count)
		{
			newPatch = buffer.contactPatches[newPatch].next;
			newContact = 0;
		}

		currPatch = newPatch;
		currContact = newContact;
	}

private:
	CorrelationListIteratorCoulomb& operator=(const CorrelationListIteratorCoulomb&);

};


bool physx::setupFinalizeSolverConstraintsCoulomb4(PxcSolverContact4Desc* PX_RESTRICT descs, PxcNpThreadContext& threadContext, PxU8* PX_RESTRICT workspace, 
											const PxReal invDtF32, PxReal bounceThresholdF32, PxcCorrelationBufferCoulomb& c, const PxU32 numFrictionPerPoint,
											const PxU32 numContactPoints4, const PxU32 /*solverConstraintByteSize*/,
											const Ps::aos::Vec4VArg invMassScale0, const Ps::aos::Vec4VArg invInertiaScale0, 
											const Ps::aos::Vec4VArg invMassScale1, const Ps::aos::Vec4VArg invInertiaScale1)
{
	//KS - final step. Create the constraints in the place we pre-allocated...
	const Vec4V zero = V4Zero();

	PxU8 flags[4] = {	PxU8(descs[0].unit->flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0),
						PxU8(descs[1].unit->flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0),
						PxU8(descs[2].unit->flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0),
						PxU8(descs[3].unit->flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0) };


	//The block is dynamic if **any** of the constraints have a non-static body B. This allows us to batch static and non-static constraints but we only get a memory/perf
	//saving if all 4 are static. This simplifies the constraint partitioning such that it only needs to care about separating contacts and 1D constraints (which it already does)
	const bool isDynamic = ((descs[0].unit->flags | descs[1].unit->flags | descs[2].unit->flags | descs[3].unit->flags) & PxcNpWorkUnitFlag::eDYNAMIC_BODY1) != 0;

	const PxU32 constraintSize = isDynamic ? sizeof(PxcSolverContact4Dynamic) : sizeof(PxcSolverContact4Base);
	const PxU32 frictionSize = isDynamic ? sizeof(PxcSolverFriction4Dynamic) : sizeof(PxcSolverFriction4Base);

	PxU8* PX_RESTRICT ptr = workspace;

	const Vec4V _dom0 = V4Merge(FLoad(descs[0].unit->dominance0), FLoad(descs[1].unit->dominance0), FLoad(descs[2].unit->dominance0),
		FLoad(descs[3].unit->dominance0));
	const Vec4V _dom1 = V4Merge(FLoad(descs[0].unit->dominance1), FLoad(descs[1].unit->dominance1), FLoad(descs[2].unit->dominance1),
		FLoad(descs[3].unit->dominance1));

	const Vec4V dom0 = V4Mul(_dom0, invMassScale0);
	const Vec4V dom1 = V4Mul(_dom1, invMassScale1);
	const Vec4V angDom0 = V4Mul(_dom0, invInertiaScale0);
	const Vec4V angDom1 = V4Mul(_dom1, invInertiaScale1);

	const Vec4V nDom1 = V4Neg(dom1);

	const Vec4V maxPenBias = V4Max(V4Merge(FLoad(descs[0].data0->penBiasClamp), FLoad(descs[1].data0->penBiasClamp), 
		FLoad(descs[2].data0->penBiasClamp), FLoad(descs[3].data0->penBiasClamp)), 
		V4Merge(FLoad(descs[0].data1->penBiasClamp), FLoad(descs[1].data1->penBiasClamp), 
		FLoad(descs[2].data1->penBiasClamp), FLoad(descs[3].data1->penBiasClamp)));

	const Vec4V restDistance = V4Merge(FLoad(descs[0].unit->restDistance), FLoad(descs[1].unit->restDistance), FLoad(descs[2].unit->restDistance),
		FLoad(descs[3].unit->restDistance)); 

	//load up velocities
	Vec4V linVel00 = V4LoadA(&descs[0].b0->linearVelocity.x);
	Vec4V linVel10 = V4LoadA(&descs[1].b0->linearVelocity.x);
	Vec4V linVel20 = V4LoadA(&descs[2].b0->linearVelocity.x);
	Vec4V linVel30 = V4LoadA(&descs[3].b0->linearVelocity.x);

	Vec4V linVel01 = V4LoadA(&descs[0].b1->linearVelocity.x);
	Vec4V linVel11 = V4LoadA(&descs[1].b1->linearVelocity.x);
	Vec4V linVel21 = V4LoadA(&descs[2].b1->linearVelocity.x);
	Vec4V linVel31 = V4LoadA(&descs[3].b1->linearVelocity.x);

	Vec4V angVel00 = V4LoadA(&descs[0].b0->angularVelocity.x);
	Vec4V angVel10 = V4LoadA(&descs[1].b0->angularVelocity.x);
	Vec4V angVel20 = V4LoadA(&descs[2].b0->angularVelocity.x);
	Vec4V angVel30 = V4LoadA(&descs[3].b0->angularVelocity.x);

	Vec4V angVel01 = V4LoadA(&descs[0].b1->angularVelocity.x);
	Vec4V angVel11 = V4LoadA(&descs[1].b1->angularVelocity.x);
	Vec4V angVel21 = V4LoadA(&descs[2].b1->angularVelocity.x);
	Vec4V angVel31 = V4LoadA(&descs[3].b1->angularVelocity.x);

	Vec4V linVelT00, linVelT10, linVelT20;
	Vec4V linVelT01, linVelT11, linVelT21;
	Vec4V angVelT00, angVelT10, angVelT20;
	Vec4V angVelT01, angVelT11, angVelT21;

	PX_TRANSPOSE_44_34(linVel00, linVel10, linVel20, linVel30, linVelT00, linVelT10, linVelT20);
	PX_TRANSPOSE_44_34(linVel01, linVel11, linVel21, linVel31, linVelT01, linVelT11, linVelT21);
	PX_TRANSPOSE_44_34(angVel00, angVel10, angVel20, angVel30, angVelT00, angVelT10, angVelT20);
	PX_TRANSPOSE_44_34(angVel01, angVel11, angVel21, angVel31, angVelT01, angVelT11, angVelT21);

	const Vec4V vrelX = V4Sub(linVelT00, linVelT01);
	const Vec4V vrelY = V4Sub(linVelT10, linVelT11);
	const Vec4V vrelZ = V4Sub(linVelT20, linVelT21);



	//Load up masses and invInertia

	const Vec4V invMass0 = V4Merge(FLoad(descs[0].data0->invMass), FLoad(descs[1].data0->invMass), FLoad(descs[2].data0->invMass),
		FLoad(descs[3].data0->invMass));

	const Vec4V invMass1 = V4Merge(FLoad(descs[0].data1->invMass), FLoad(descs[1].data1->invMass), FLoad(descs[2].data1->invMass),
		FLoad(descs[3].data1->invMass));

	const Vec4V invMass0_dom0fV = V4Mul(dom0, invMass0);
	const Vec4V invMass1_dom1fV = V4Mul(nDom1, invMass1);

	Vec4V invInertia00X = Vec4V_From_Vec3V(V3LoadU(descs[0].data0->invInertia.column0));
	Vec4V invInertia00Y = Vec4V_From_Vec3V(V3LoadU(descs[0].data0->invInertia.column1));
	Vec4V invInertia00Z = Vec4V_From_Vec3V(V3LoadU(descs[0].data0->invInertia.column2));

	Vec4V invInertia10X = Vec4V_From_Vec3V(V3LoadU(descs[1].data0->invInertia.column0));
	Vec4V invInertia10Y = Vec4V_From_Vec3V(V3LoadU(descs[1].data0->invInertia.column1));
	Vec4V invInertia10Z = Vec4V_From_Vec3V(V3LoadU(descs[1].data0->invInertia.column2));

	Vec4V invInertia20X = Vec4V_From_Vec3V(V3LoadU(descs[2].data0->invInertia.column0));
	Vec4V invInertia20Y = Vec4V_From_Vec3V(V3LoadU(descs[2].data0->invInertia.column1));
	Vec4V invInertia20Z = Vec4V_From_Vec3V(V3LoadU(descs[2].data0->invInertia.column2));

	Vec4V invInertia30X = Vec4V_From_Vec3V(V3LoadU(descs[3].data0->invInertia.column0));
	Vec4V invInertia30Y = Vec4V_From_Vec3V(V3LoadU(descs[3].data0->invInertia.column1));
	Vec4V invInertia30Z = Vec4V_From_Vec3V(V3LoadU(descs[3].data0->invInertia.column2));

	Vec4V invInertia01X = Vec4V_From_Vec3V(V3LoadU(descs[0].data1->invInertia.column0));
	Vec4V invInertia01Y = Vec4V_From_Vec3V(V3LoadU(descs[0].data1->invInertia.column1));
	Vec4V invInertia01Z = Vec4V_From_Vec3V(V3LoadU(descs[0].data1->invInertia.column2));

	Vec4V invInertia11X = Vec4V_From_Vec3V(V3LoadU(descs[1].data1->invInertia.column0));
	Vec4V invInertia11Y = Vec4V_From_Vec3V(V3LoadU(descs[1].data1->invInertia.column1));
	Vec4V invInertia11Z = Vec4V_From_Vec3V(V3LoadU(descs[1].data1->invInertia.column2));

	Vec4V invInertia21X = Vec4V_From_Vec3V(V3LoadU(descs[2].data1->invInertia.column0));
	Vec4V invInertia21Y = Vec4V_From_Vec3V(V3LoadU(descs[2].data1->invInertia.column1));
	Vec4V invInertia21Z = Vec4V_From_Vec3V(V3LoadU(descs[2].data1->invInertia.column2));

	Vec4V invInertia31X = Vec4V_From_Vec3V(V3LoadU(descs[3].data1->invInertia.column0));
	Vec4V invInertia31Y = Vec4V_From_Vec3V(V3LoadU(descs[3].data1->invInertia.column1));
	Vec4V invInertia31Z = Vec4V_From_Vec3V(V3LoadU(descs[3].data1->invInertia.column2));

	Vec4V invInertia0X0, invInertia0X1, invInertia0X2;
	Vec4V invInertia0Y0, invInertia0Y1, invInertia0Y2;
	Vec4V invInertia0Z0, invInertia0Z1, invInertia0Z2;

	Vec4V invInertia1X0, invInertia1X1, invInertia1X2;
	Vec4V invInertia1Y0, invInertia1Y1, invInertia1Y2;
	Vec4V invInertia1Z0, invInertia1Z1, invInertia1Z2;

	PX_TRANSPOSE_44_34(invInertia00X, invInertia10X, invInertia20X, invInertia30X, invInertia0X0, invInertia0Y0, invInertia0Z0);
	PX_TRANSPOSE_44_34(invInertia00Y, invInertia10Y, invInertia20Y, invInertia30Y, invInertia0X1, invInertia0Y1, invInertia0Z1);
	PX_TRANSPOSE_44_34(invInertia00Z, invInertia10Z, invInertia20Z, invInertia30Z, invInertia0X2, invInertia0Y2, invInertia0Z2);

	PX_TRANSPOSE_44_34(invInertia01X, invInertia11X, invInertia21X, invInertia31X, invInertia1X0, invInertia1Y0, invInertia1Z0);
	PX_TRANSPOSE_44_34(invInertia01Y, invInertia11Y, invInertia21Y, invInertia31Y, invInertia1X1, invInertia1Y1, invInertia1Z1);
	PX_TRANSPOSE_44_34(invInertia01Z, invInertia11Z, invInertia21Z, invInertia31Z, invInertia1X2, invInertia1Y2, invInertia1Z2);

	invInertia0X0 = V4Mul(invInertia0X0, angDom0);
	invInertia0Y0 = V4Mul(invInertia0Y0, angDom0);
	invInertia0Z0 = V4Mul(invInertia0Z0, angDom0);

	invInertia0X1 = V4Mul(invInertia0X1, angDom0);
	invInertia0Y1 = V4Mul(invInertia0Y1, angDom0);
	invInertia0Z1 = V4Mul(invInertia0Z1, angDom0);

	invInertia0X2 = V4Mul(invInertia0X2, angDom0);
	invInertia0Y2 = V4Mul(invInertia0Y2, angDom0);
	invInertia0Z2 = V4Mul(invInertia0Z2, angDom0);

	invInertia1X0 = V4Mul(invInertia1X0, angDom1);
	invInertia1Y0 = V4Mul(invInertia1Y0, angDom1);
	invInertia1Z0 = V4Mul(invInertia1Z0, angDom1);

	invInertia1X1 = V4Mul(invInertia1X1, angDom1);
	invInertia1Y1 = V4Mul(invInertia1Y1, angDom1);
	invInertia1Z1 = V4Mul(invInertia1Z1, angDom1);

	invInertia1X2 = V4Mul(invInertia1X2, angDom1);
	invInertia1Y2 = V4Mul(invInertia1Y2, angDom1);
	invInertia1Z2 = V4Mul(invInertia1Z2, angDom1);


	const FloatV invDt = FLoad(invDtF32);
	const FloatV p8 = FLoad(0.8f);
	//const Vec4V p84 = V4Splat(p8);
	const Vec4V p1 = V4Splat(FLoad(0.1f));
	const Vec4V bounceThreshold = V4Splat(FLoad(bounceThresholdF32));
	const Vec4V orthoThreshold = V4Splat(FLoad(0.70710678f));

	const FloatV invDtp8 = FMul(invDt, p8);

	const Vec3V bodyFrame00p = V3LoadU(descs[0].bodyFrame0->p);
	const Vec3V bodyFrame01p = V3LoadU(descs[1].bodyFrame0->p);
	const Vec3V bodyFrame02p = V3LoadU(descs[2].bodyFrame0->p);
	const Vec3V bodyFrame03p = V3LoadU(descs[3].bodyFrame0->p);

	Vec4V bodyFrame00p4 = Vec4V_From_Vec3V(bodyFrame00p);
	Vec4V bodyFrame01p4 = Vec4V_From_Vec3V(bodyFrame01p);
	Vec4V bodyFrame02p4 = Vec4V_From_Vec3V(bodyFrame02p);
	Vec4V bodyFrame03p4 = Vec4V_From_Vec3V(bodyFrame03p);

	Vec4V bodyFrame0pX, bodyFrame0pY, bodyFrame0pZ;
	PX_TRANSPOSE_44_34(bodyFrame00p4, bodyFrame01p4, bodyFrame02p4, bodyFrame03p4, bodyFrame0pX, bodyFrame0pY, bodyFrame0pZ);

	
	const Vec3V bodyFrame10p = V3LoadU(descs[0].bodyFrame1->p);
	const Vec3V bodyFrame11p = V3LoadU(descs[1].bodyFrame1->p);
	const Vec3V bodyFrame12p = V3LoadU(descs[2].bodyFrame1->p);
	const Vec3V bodyFrame13p = V3LoadU(descs[3].bodyFrame1->p);

	Vec4V bodyFrame10p4 = Vec4V_From_Vec3V(bodyFrame10p);
	Vec4V bodyFrame11p4 = Vec4V_From_Vec3V(bodyFrame11p);
	Vec4V bodyFrame12p4 = Vec4V_From_Vec3V(bodyFrame12p);
	Vec4V bodyFrame13p4 = Vec4V_From_Vec3V(bodyFrame13p);

	Vec4V bodyFrame1pX, bodyFrame1pY, bodyFrame1pZ;
	PX_TRANSPOSE_44_34(bodyFrame10p4, bodyFrame11p4, bodyFrame12p4, bodyFrame13p4, bodyFrame1pX, bodyFrame1pY, bodyFrame1pZ);

	
	Ps::prefetchLine(c.contactID);
	Ps::prefetchLine(c.contactID, 128);

	PxU32 frictionIndex0 = 0, frictionIndex1 = 0, frictionIndex2 = 0, frictionIndex3 = 0;


	PxU32 maxPatches = PxMax(descs[0].numFrictionPatches, PxMax(descs[1].numFrictionPatches, PxMax(descs[2].numFrictionPatches, descs[3].numFrictionPatches)));
	PxU32 maxContacts = numContactPoints4;

	//This is the address at which the first friction patch exists
	PxU8* ptr2 = ptr + ((sizeof(PxcSolverContactCoulombHeader4) * maxPatches) + constraintSize * maxContacts);

	//PxU32 contactId = 0;

	for(PxU32 i=0;i<maxPatches;i++)
	{
		const bool hasFinished0 = i >= descs[0].numFrictionPatches;
		const bool hasFinished1 = i >= descs[1].numFrictionPatches;
		const bool hasFinished2 = i >= descs[2].numFrictionPatches;
		const bool hasFinished3 = i >= descs[3].numFrictionPatches;


		frictionIndex0 = hasFinished0 ? frictionIndex0 : descs[0].startFrictionPatchIndex + i;
		frictionIndex1 = hasFinished1 ? frictionIndex1 : descs[1].startFrictionPatchIndex + i;
		frictionIndex2 = hasFinished2 ? frictionIndex2 : descs[2].startFrictionPatchIndex + i;
		frictionIndex3 = hasFinished3 ? frictionIndex3 : descs[3].startFrictionPatchIndex + i;

		PxU32 clampedContacts0 = hasFinished0 ? 0 : c.frictionPatchContactCounts[frictionIndex0];
		PxU32 clampedContacts1 = hasFinished1 ? 0 : c.frictionPatchContactCounts[frictionIndex1];
		PxU32 clampedContacts2 = hasFinished2 ? 0 : c.frictionPatchContactCounts[frictionIndex2];
		PxU32 clampedContacts3 = hasFinished3 ? 0 : c.frictionPatchContactCounts[frictionIndex3];

		PxU32 clampedFric0 = PxU32(hasFinished0 ? 0 : c.frictionPatches[frictionIndex0].numConstraints);
		PxU32 clampedFric1 = PxU32(hasFinished1 ? 0 : c.frictionPatches[frictionIndex1].numConstraints);
		PxU32 clampedFric2 = PxU32(hasFinished2 ? 0 : c.frictionPatches[frictionIndex2].numConstraints);
		PxU32 clampedFric3 = PxU32(hasFinished3 ? 0 : c.frictionPatches[frictionIndex3].numConstraints);


		const PxU32 numContacts = PxMax(clampedContacts0, PxMax(clampedContacts1, PxMax(clampedContacts2, clampedContacts3)));
		const PxU32 numFrictions = PxMax(clampedFric0, PxMax(clampedFric1, PxMax(clampedFric2, clampedFric3)));

		PxU32 firstPatch0 = c.correlationListHeads[frictionIndex0];
		PxU32 firstPatch1 = c.correlationListHeads[frictionIndex1];
		PxU32 firstPatch2 = c.correlationListHeads[frictionIndex2];
		PxU32 firstPatch3 = c.correlationListHeads[frictionIndex3];

		const Gu::ContactPoint* contactBase0 = threadContext.mContactBuffer.contacts + c.contactPatches[firstPatch0].start;
		const Gu::ContactPoint* contactBase1 = threadContext.mContactBuffer.contacts + c.contactPatches[firstPatch1].start;
		const Gu::ContactPoint* contactBase2 = threadContext.mContactBuffer.contacts + c.contactPatches[firstPatch2].start;
		const Gu::ContactPoint* contactBase3 = threadContext.mContactBuffer.contacts + c.contactPatches[firstPatch3].start;

		const Vec4V restitution = V4Merge(FLoad(contactBase0->restitution), FLoad(contactBase1->restitution), FLoad(contactBase2->restitution),
			FLoad(contactBase3->restitution));

		const Vec4V staticFriction = V4Merge(FLoad(contactBase0->staticFriction), FLoad(contactBase1->staticFriction), FLoad(contactBase2->staticFriction),
			FLoad(contactBase3->staticFriction));

		PxcSolverContactCoulombHeader4* PX_RESTRICT header = reinterpret_cast<PxcSolverContactCoulombHeader4*>(ptr);

		header->frictionOffset = PxU16(ptr2 - ptr);

		ptr += sizeof(PxcSolverContactCoulombHeader4);	

		PxcSolverFrictionHeader4* PX_RESTRICT fricHeader = reinterpret_cast<PxcSolverFrictionHeader4*>(ptr2);
		ptr2 += sizeof(PxcSolverFrictionHeader4) + sizeof(Vec4V) * numContacts;


		header->numNormalConstr0 = Ps::to8(clampedContacts0);
		header->numNormalConstr1 = Ps::to8(clampedContacts1);
		header->numNormalConstr2 = Ps::to8(clampedContacts2);
		header->numNormalConstr3 = Ps::to8(clampedContacts3);
		header->numNormalConstr = Ps::to8(numContacts);
		header->invMassADom = invMass0_dom0fV;
		header->invMassBDom = invMass1_dom1fV;
		header->restitution = restitution;

		header->flags[0] = flags[0]; header->flags[1] = flags[1]; header->flags[2] = flags[2]; header->flags[3] = flags[3];

		header->type = Ps::to8(isDynamic ? PXS_SC_TYPE_BLOCK_RB_CONTACT : PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT);


		fricHeader->invMassADom = invMass0_dom0fV;
		fricHeader->invMassBDom = invMass1_dom1fV;
		fricHeader->numFrictionConstr0 = Ps::to8(clampedFric0);
		fricHeader->numFrictionConstr1 = Ps::to8(clampedFric1);
		fricHeader->numFrictionConstr2 = Ps::to8(clampedFric2);
		fricHeader->numFrictionConstr3 = Ps::to8(clampedFric3);
		fricHeader->numNormalConstr = Ps::to8(numContacts);
		fricHeader->numNormalConstr0 = Ps::to8(clampedContacts0);
		fricHeader->numNormalConstr1 = Ps::to8(clampedContacts1);
		fricHeader->numNormalConstr2 = Ps::to8(clampedContacts2);
		fricHeader->numNormalConstr3 = Ps::to8(clampedContacts3);
		fricHeader->type = Ps::to8(isDynamic ? PXS_SC_TYPE_BLOCK_FRICTION : PXS_SC_TYPE_BLOCK_STATIC_FRICTION);
		fricHeader->staticFriction = staticFriction;
		fricHeader->frictionPerContact = PxU32(numFrictionPerPoint == 2 ? 1 : 0);

		fricHeader->numFrictionConstr = Ps::to8(numFrictions);
		
		Vec4V normal0 = V4LoadA(&contactBase0->normal.x);
		Vec4V normal1 = V4LoadA(&contactBase1->normal.x);
		Vec4V normal2 = V4LoadA(&contactBase2->normal.x);
		Vec4V normal3 = V4LoadA(&contactBase3->normal.x);

		Vec4V normalX, normalY, normalZ;
		PX_TRANSPOSE_44_34(normal0, normal1, normal2, normal3, normalX, normalY, normalZ);
		header->normalX = normalX;
		header->normalY = normalY;
		header->normalZ = normalZ;

		const Vec4V normalLenSq = V4MulAdd(normalZ, normalZ, V4MulAdd(normalY, normalY, V4Mul(normalX, normalX)));

		const Vec4V norVel0 = V4MulAdd(normalZ, linVelT20, V4MulAdd(normalY, linVelT10, V4Mul(normalX, linVelT00)));
		const Vec4V norVel1 = V4MulAdd(normalZ, linVelT21, V4MulAdd(normalY, linVelT11, V4Mul(normalX, linVelT01)));

		const Vec4V invMassNorLenSq0 = V4Mul(invMass0_dom0fV, normalLenSq);
		const Vec4V invMassNorLenSq1 = V4Mul(invMass1_dom1fV, normalLenSq);		


		//Calculate friction directions
		const BoolV cond =V4IsGrtr(orthoThreshold, V4Abs(normalX));

		const Vec4V t0FallbackX = V4Sel(cond, zero, V4Neg(normalY));
		const Vec4V t0FallbackY = V4Sel(cond, V4Neg(normalZ), normalX);
		const Vec4V t0FallbackZ = V4Sel(cond, normalY, zero);

		const Vec4V dotNormalVrel = V4MulAdd(normalZ, vrelZ, V4MulAdd(normalY, vrelY, V4Mul(normalX, vrelX)));
		const Vec4V vrelSubNorVelX = V4NegMulSub(normalX, dotNormalVrel, vrelX);
		const Vec4V vrelSubNorVelY = V4NegMulSub(normalY, dotNormalVrel, vrelY);
		const Vec4V vrelSubNorVelZ = V4NegMulSub(normalZ, dotNormalVrel, vrelZ);

		const Vec4V lenSqvrelSubNorVelZ = V4MulAdd(vrelSubNorVelX, vrelSubNorVelX, V4MulAdd(vrelSubNorVelY, vrelSubNorVelY, V4Mul(vrelSubNorVelZ, vrelSubNorVelZ)));

		const BoolV bcon2 = V4IsGrtr(lenSqvrelSubNorVelZ, p1);

		Vec4V t0X = V4Sel(bcon2, vrelSubNorVelX, t0FallbackX);
		Vec4V t0Y = V4Sel(bcon2, vrelSubNorVelY, t0FallbackY);
		Vec4V t0Z = V4Sel(bcon2, vrelSubNorVelZ, t0FallbackZ);

		//Now normalize this...
		const Vec4V recipLen = V4Rsqrt(V4MulAdd(t0X, t0X, V4MulAdd(t0Y, t0Y, V4Mul(t0Z, t0Z))));

		t0X = V4Mul(t0X, recipLen);
		t0Y = V4Mul(t0Y, recipLen);
		t0Z = V4Mul(t0Z, recipLen);

		const Vec4V t1X = V4NegMulSub(normalZ, t0Y, V4Mul(normalY, t0Z));
		const Vec4V t1Y = V4NegMulSub(normalX, t0Z, V4Mul(normalZ, t0X));
		const Vec4V t1Z = V4NegMulSub(normalY, t0X, V4Mul(normalX, t0Y));

		const Vec4V tFallbackX[2] = {t0X, t1X};
		const Vec4V tFallbackY[2] = {t0Y, t1Y};
		const Vec4V tFallbackZ[2] = {t0Z, t1Z};


		//For all correlation heads - need to pull this out I think

		//OK, we have a counter for all our patches...
		PxU32 finished = ((PxU32)(hasFinished0)) | 
						 (((PxU32)(hasFinished1)) << 1) | 
						 (((PxU32)(hasFinished2)) << 2) | 
						 (((PxU32)(hasFinished3)) << 3);

		CorrelationListIteratorCoulomb iter0(c, firstPatch0);
		CorrelationListIteratorCoulomb iter1(c, firstPatch1);
		CorrelationListIteratorCoulomb iter2(c, firstPatch2);
		CorrelationListIteratorCoulomb iter3(c, firstPatch3);

		PxU32 contact0, contact1, contact2, contact3;
		PxU32 patch0, patch1, patch2, patch3;

		iter0.nextContact(patch0, contact0);
		iter1.nextContact(patch1, contact1);
		iter2.nextContact(patch2, contact2);
		iter3.nextContact(patch3, contact3);

		PxU8* p = ptr;

		PxU32 contactCount = 0;
		PxU32 newFinished = 
			((PxU32)(hasFinished0 || !iter0.hasNextContact()))		| 
			(((PxU32)(hasFinished1 || !iter1.hasNextContact())) << 1) | 
			(((PxU32)(hasFinished2 || !iter2.hasNextContact())) << 2) | 
			(((PxU32)(hasFinished3 || !iter3.hasNextContact())) << 3);

		PxU32 fricIndex = 0;

		while(finished != 0xf)
		{
			finished = newFinished;
			++contactCount;
			Ps::prefetchLine(p, 384);
			Ps::prefetchLine(p, 512);
			Ps::prefetchLine(p, 640);	

			PxcSolverContact4Base* PX_RESTRICT solverContact = reinterpret_cast<PxcSolverContact4Base*>(p);
			p += constraintSize;

			const Gu::ContactPoint& con0 = threadContext.mContactBuffer.contacts[c.contactPatches[patch0].start + contact0];
			const Gu::ContactPoint& con1 = threadContext.mContactBuffer.contacts[c.contactPatches[patch1].start + contact1];
			const Gu::ContactPoint& con2 = threadContext.mContactBuffer.contacts[c.contactPatches[patch2].start + contact2];
			const Gu::ContactPoint& con3 = threadContext.mContactBuffer.contacts[c.contactPatches[patch3].start + contact3];

			//Now we need to splice these 4 contacts into a single structure

			{
				Vec4V point0 = V4LoadA(&con0.point.x);
				Vec4V point1 = V4LoadA(&con1.point.x);
				Vec4V point2 = V4LoadA(&con2.point.x);
				Vec4V point3 = V4LoadA(&con3.point.x);

				Vec4V pointX, pointY, pointZ;
				PX_TRANSPOSE_44_34(point0, point1, point2, point3, pointX, pointY, pointZ);

				Vec4V cTargetVel0 = V4LoadA(&con0.targetVel.x);
				Vec4V cTargetVel1 = V4LoadA(&con1.targetVel.x);
				Vec4V cTargetVel2 = V4LoadA(&con2.targetVel.x);
				Vec4V cTargetVel3 = V4LoadA(&con3.targetVel.x);

				Vec4V cTargetVelX, cTargetVelY, cTargetVelZ;
				PX_TRANSPOSE_44_34(cTargetVel0, cTargetVel1, cTargetVel2, cTargetVel3, cTargetVelX, cTargetVelY, cTargetVelZ);

				const Vec4V separation = V4Merge(FLoad(con0.separation), FLoad(con1.separation), FLoad(con2.separation),
					FLoad(con3.separation));
				const Vec4V maxImpulse = V4Merge(FLoad(con0.maxImpulse), FLoad(con1.maxImpulse), FLoad(con2.maxImpulse),
					FLoad(con3.maxImpulse));

				const Vec4V cTargetVel = V4MulAdd(normalX, cTargetVelX, V4MulAdd(normalY, cTargetVelY, V4Mul(normalZ, cTargetVelZ)));

				const Vec4V raX = V4Sub(pointX, bodyFrame0pX);
				const Vec4V raY = V4Sub(pointY, bodyFrame0pY);
				const Vec4V raZ = V4Sub(pointZ, bodyFrame0pZ);

				const Vec4V rbX = V4Sub(pointX, bodyFrame1pX);
				const Vec4V rbY = V4Sub(pointY, bodyFrame1pY);
				const Vec4V rbZ = V4Sub(pointZ, bodyFrame1pZ);


				//raXn = cross(ra, normal) which = Vec3V( a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
				const Vec4V raXnX0 = V4NegMulSub(raZ, normalY, V4Mul(raY, normalZ));
				const Vec4V raXnY0 = V4NegMulSub(raX, normalZ, V4Mul(raZ, normalX));
				const Vec4V raXnZ0 = V4NegMulSub(raY, normalX, V4Mul(raX, normalY));

				const Vec4V v0a00 = V4Mul(invInertia0X0, raXnX0);
				const Vec4V v0a10 = V4Mul(invInertia0X1, raXnX0);
				const Vec4V v0a20 = V4Mul(invInertia0X2, raXnX0);

				const Vec4V v0PlusV1a00 = V4MulAdd(invInertia0Y0, raXnY0, v0a00);
				const Vec4V v0PlusV1a10 = V4MulAdd(invInertia0Y1, raXnY0, v0a10);
				const Vec4V v0PlusV1a20 = V4MulAdd(invInertia0Y2, raXnY0, v0a20);

				const Vec4V delAngVel0X0 = V4MulAdd(invInertia0Z0, raXnZ0, v0PlusV1a00);
				const Vec4V delAngVel0Y0 = V4MulAdd(invInertia0Z1, raXnZ0, v0PlusV1a10);
				const Vec4V delAngVel0Z0 = V4MulAdd(invInertia0Z2, raXnZ0, v0PlusV1a20);

				const Vec4V dotRaXnDelAngVel00 = V4MulAdd(raXnZ0, delAngVel0Z0, V4MulAdd(raXnY0, delAngVel0Y0, V4Mul(raXnX0, delAngVel0X0)));
				const Vec4V dotRaXnAngVel0 = V4MulAdd(raXnZ0, angVelT20, V4MulAdd(raXnY0, angVelT10, V4Mul(raXnX0, angVelT00)));

				Vec4V unitResponse0 = V4Add(invMassNorLenSq0, dotRaXnDelAngVel00);
				Vec4V vrel = V4Add(norVel0, dotRaXnAngVel0);


				//The dynamic-only parts - need to if-statement these up. A branch here shouldn't cost us too much
				if(isDynamic)
				{
					PxcSolverContact4Dynamic* PX_RESTRICT dynamicContact = static_cast<PxcSolverContact4Dynamic*>(solverContact);
					const Vec4V rbXnX = V4NegMulSub(rbZ, normalY, V4Mul(rbY, normalZ));
					const Vec4V rbXnY = V4NegMulSub(rbX, normalZ, V4Mul(rbZ, normalX));
					const Vec4V rbXnZ = V4NegMulSub(rbY, normalX, V4Mul(rbX, normalY));

					const Vec4V v0b0 = V4Mul(invInertia1X0, rbXnX);
					const Vec4V v0b1 = V4Mul(invInertia1X1, rbXnX);
					const Vec4V v0b2 = V4Mul(invInertia1X2, rbXnX);

					const Vec4V v0PlusV1b0 = V4MulAdd(invInertia1Y0, rbXnY, v0b0);
					const Vec4V v0PlusV1b1 = V4MulAdd(invInertia1Y1, rbXnY, v0b1);
					const Vec4V v0PlusV1b2 = V4MulAdd(invInertia1Y2, rbXnY, v0b2);

					const Vec4V delAngVel1X = V4MulAdd(invInertia1Z0, rbXnZ, v0PlusV1b0);
					const Vec4V delAngVel1Y = V4MulAdd(invInertia1Z1, rbXnZ, v0PlusV1b1);
					const Vec4V delAngVel1Z = V4MulAdd(invInertia1Z2, rbXnZ, v0PlusV1b2);


					//V3Dot(raXn, delAngVel0)
					
					const Vec4V dotRbXnDelAngVel1 = V4MulAdd(rbXnZ, delAngVel1Z, V4MulAdd(rbXnY, delAngVel1Y, V4Mul(rbXnX, delAngVel1X)));
					
					const Vec4V dotRbXnAngVel1 = V4MulAdd(rbXnZ, angVelT21, V4MulAdd(rbXnY, angVelT11, V4Mul(rbXnX, angVelT01)));

					const Vec4V resp1 = V4Sub(dotRbXnDelAngVel1, invMassNorLenSq1);

					unitResponse0 = V4Add(unitResponse0, resp1);

					const Vec4V vrel2 = V4Add(norVel1, dotRbXnAngVel1);
					vrel = V4Sub(vrel, vrel2);

					//These are for dynamic-only contacts.
					dynamicContact->rbXnX = rbXnX;
					dynamicContact->rbXnY = rbXnY;
					dynamicContact->rbXnZ = rbXnZ;
					dynamicContact->delAngVel1X = V4Neg(delAngVel1X);
					dynamicContact->delAngVel1Y = V4Neg(delAngVel1Y);
					dynamicContact->delAngVel1Z = V4Neg(delAngVel1Z);

				}

				const Vec4V velMultiplier0 = V4Sel(V4IsGrtr(unitResponse0, zero), V4Recip(unitResponse0), zero);

				const Vec4V penetration = V4Sub(separation, restDistance);

				const Vec4V penInvDtp8 = V4Max(maxPenBias, V4Scale(penetration, invDtp8));

				Vec4V scaledBias = V4Mul(velMultiplier0, penInvDtp8);

				const Vec4V penetrationInvDt = V4Scale(penetration, invDt);

				const BoolV isGreater2 = BAnd(BAnd(V4IsGrtr(restitution, zero), V4IsGrtr(bounceThreshold, vrel)), 
					V4IsGrtr(V4Neg(vrel), penetrationInvDt));

				scaledBias = V4Sel(isGreater2, zero, scaledBias);

				const Vec4V sumVRel(vrel);

				const Vec4V targetVelocity = V4Add(V4Sel(isGreater2, V4Mul(V4Neg(sumVRel), restitution), zero), cTargetVel);

				//These values are present for static and dynamic contacts			
				solverContact->raXnX = raXnX0;
				solverContact->raXnY = raXnY0;
				solverContact->raXnZ = raXnZ0;
				solverContact->velMultiplier = velMultiplier0;
				solverContact->appliedForce = zero;
				solverContact->scaledBias = scaledBias;
				solverContact->targetVelocity = targetVelocity;
				solverContact->maxImpulse = maxImpulse;
				solverContact->delAngVel0X = delAngVel0X0;
				solverContact->delAngVel0Y = delAngVel0Y0;
				solverContact->delAngVel0Z = delAngVel0Z0;		

				//PxU32 conId = contactId++;

				Vec4V targetVel0 = V4LoadA(&con0.targetVel.x);
				Vec4V targetVel1 = V4LoadA(&con1.targetVel.x);
				Vec4V targetVel2 = V4LoadA(&con2.targetVel.x);
				Vec4V targetVel3 = V4LoadA(&con3.targetVel.x);

				Vec4V targetVelX, targetVelY, targetVelZ;
				PX_TRANSPOSE_44_34(targetVel0, targetVel1, targetVel2, targetVel3, targetVelX, targetVelY, targetVelZ);

				for(PxU32 a = 0; a < numFrictionPerPoint; ++a)
				{
					PxcSolverFriction4Base* PX_RESTRICT friction = reinterpret_cast<PxcSolverFriction4Base*>(ptr2);

					ptr2 += frictionSize;

					const Vec4V tX = tFallbackX[fricIndex];
					const Vec4V tY = tFallbackY[fricIndex];
					const Vec4V tZ = tFallbackZ[fricIndex];

					fricIndex = 1 - fricIndex;

					const Vec4V raXnX = V4NegMulSub(raZ, tY, V4Mul(raY, tZ));
					const Vec4V raXnY = V4NegMulSub(raX, tZ, V4Mul(raZ, tX));
					const Vec4V raXnZ = V4NegMulSub(raY, tX, V4Mul(raX, tY));

					const Vec4V v0a0 = V4Mul(invInertia0X0, raXnX);
					const Vec4V v0a1 = V4Mul(invInertia0X1, raXnX);
					const Vec4V v0a2 = V4Mul(invInertia0X2, raXnX);

					const Vec4V v0PlusV1a0 = V4MulAdd(invInertia0Y0, raXnY, v0a0);
					const Vec4V v0PlusV1a1 = V4MulAdd(invInertia0Y1, raXnY, v0a1);
					const Vec4V v0PlusV1a2 = V4MulAdd(invInertia0Y2, raXnY, v0a2);

					const Vec4V delAngVel0X = V4MulAdd(invInertia0Z0, raXnZ, v0PlusV1a0);
					const Vec4V delAngVel0Y = V4MulAdd(invInertia0Z1, raXnZ, v0PlusV1a1);
					const Vec4V delAngVel0Z = V4MulAdd(invInertia0Z2, raXnZ, v0PlusV1a2);

					const Vec4V dotRaXnDelAngVel0 = V4MulAdd(raXnZ, delAngVel0Z, V4MulAdd(raXnY, delAngVel0Y, V4Mul(raXnX, delAngVel0X)));
					
					Vec4V unitResponse = V4Add(invMass0_dom0fV, dotRaXnDelAngVel0);
					
					if(isDynamic)
					{
						PxcSolverFriction4Dynamic* PX_RESTRICT dFric = static_cast<PxcSolverFriction4Dynamic*>(friction);

						const Vec4V rbXnX = V4NegMulSub(rbZ, tY, V4Mul(rbY, tZ));
						const Vec4V rbXnY = V4NegMulSub(rbX, tZ, V4Mul(rbZ, tX));
						const Vec4V rbXnZ = V4NegMulSub(rbY, tX, V4Mul(rbX, tY));
						
						const Vec4V v0b0 = V4Mul(invInertia1X0, rbXnX);
						const Vec4V v0b1 = V4Mul(invInertia1X1, rbXnX);
						const Vec4V v0b2 = V4Mul(invInertia1X2, rbXnX);

						const Vec4V v0PlusV1b0 = V4MulAdd(invInertia1Y0, rbXnY, v0b0);
						const Vec4V v0PlusV1b1 = V4MulAdd(invInertia1Y1, rbXnY, v0b1);
						const Vec4V v0PlusV1b2 = V4MulAdd(invInertia1Y2, rbXnY, v0b2);

						const Vec4V delAngVel1X = V4MulAdd(invInertia1Z0, rbXnZ, v0PlusV1b0);
						const Vec4V delAngVel1Y = V4MulAdd(invInertia1Z1, rbXnZ, v0PlusV1b1);
						const Vec4V delAngVel1Z = V4MulAdd(invInertia1Z2, rbXnZ, v0PlusV1b2);

						const Vec4V dotRbXnDelAngVel1 = V4MulAdd(rbXnZ, delAngVel1Z, V4MulAdd(rbXnY, delAngVel1Y, V4Mul(rbXnX, delAngVel1X)));
				
						const Vec4V resp1 = V4Sub(dotRbXnDelAngVel1, invMassNorLenSq1);

						unitResponse = V4Add(unitResponse, resp1);

						dFric->rbXnX = rbXnX;
						dFric->rbXnY = rbXnY;
						dFric->rbXnZ = rbXnZ;
						dFric->delAngVel1X = V4Neg(delAngVel1X);
						dFric->delAngVel1Y = V4Neg(delAngVel1Y);
						dFric->delAngVel1Z = V4Neg(delAngVel1Z);
					}

					const Vec4V velMultiplier = V4Neg(V4Sel(V4IsGrtr(unitResponse, zero), V4Recip(unitResponse), zero));

					friction->appliedForce = zero;
					friction->delAngVel0X = delAngVel0X;
					friction->delAngVel0Y = delAngVel0Y;
					friction->delAngVel0Z = delAngVel0Z;
					friction->raXnX = raXnX;
					friction->raXnY = raXnY;
					friction->raXnZ = raXnZ;
					friction->velMultiplier = velMultiplier;
					friction->targetVelocity = V4MulAdd(targetVelZ, tZ, V4MulAdd(targetVelY, tY, V4Mul(targetVelX, tX)));
					friction->normalX = tX;
					friction->normalY = tY;
					friction->normalZ = tZ;
				}
			}
			if(!(finished & 0x1))
			{
				iter0.nextContact(patch0, contact0);
				newFinished |= (PxU32)!iter0.hasNextContact();
			}

			if(!(finished & 0x2))
			{
				iter1.nextContact(patch1, contact1);
				newFinished |= ((PxU32)!iter1.hasNextContact()) << 1;
			}

			if(!(finished & 0x4))
			{
				iter2.nextContact(patch2, contact2);
				newFinished |= ((PxU32)!iter2.hasNextContact()) << 2;
			}

			if(!(finished & 0x8))
			{
				iter3.nextContact(patch3, contact3);
				newFinished |= ((PxU32)!iter3.hasNextContact()) << 3;
			}
		}
		ptr = p;
	}
	return true;
}
