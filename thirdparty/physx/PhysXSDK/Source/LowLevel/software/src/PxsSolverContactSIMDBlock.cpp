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
#include "PxcSolverConstraintTypes.h"
#include "PxcSolverConstraintDesc.h"
#include "PsMathUtils.h"
#include "PxsSolverContact.h"
#include "PxContactModifyCallback.h"
#include "PxcNpWorkUnit.h"
#include "PxsMaterialManager.h"
#include "PxsMaterialCombiner.h"
#include "PxcSolverContact4.h"
#include "PxcNpContactPrep.h"
#include "PxvDynamics.h"

//#ifdef	__SPU__
//#include "CellUtil.h"
//#endif



using namespace physx;


struct CorrelationListIterator
{
	PxcCorrelationBuffer& buffer;
	PxU32 currPatch;
	PxU32 currContact;

	CorrelationListIterator(PxcCorrelationBuffer& correlationBuffer, PxU32 startPatch) : buffer(correlationBuffer)
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
	CorrelationListIterator& operator=(const CorrelationListIterator&);

};

void physx::setupFinalizeSolverConstraints4(PxcSolverContact4Desc* PX_RESTRICT descs, PxcNpThreadContext& threadContext, PxU8* PX_RESTRICT workspace, 
											const PxReal invDtF32, PxReal bounceThresholdF32,
											const Ps::aos::Vec4VArg invMassScale0, const Ps::aos::Vec4VArg invInertiaScale0, 
											const Ps::aos::Vec4VArg invMassScale1, const Ps::aos::Vec4VArg invInertiaScale1)
{

	//OK, we have a workspace of pre-allocated space to store all 4 descs in. We now need to create the constraints in it

	const Vec4V zero = V4Zero();
	const BoolV bFalse = BFFFF();
	const FloatV fZero = FZero();

	PxU8 flags[4] = {	PxU8(descs[0].unit->flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0),
						PxU8(descs[1].unit->flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0),
						PxU8(descs[2].unit->flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0),
						PxU8(descs[3].unit->flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0)};

	bool hasMaxImpulse = descs[0].hasMaxImpulse || descs[1].hasMaxImpulse || descs[2].hasMaxImpulse || descs[3].hasMaxImpulse;
	bool hasTargetVelocity = descs[0].hasTargetVelocity || descs[1].hasTargetVelocity || descs[2].hasTargetVelocity || descs[3].hasTargetVelocity;



	//The block is dynamic if **any** of the constraints have a non-static body B. This allows us to batch static and non-static constraints but we only get a memory/perf
	//saving if all 4 are static. This simplifies the constraint partitioning such that it only needs to care about separating contacts and 1D constraints (which it already does)
	const bool isDynamic = ((descs[0].unit->flags | descs[1].unit->flags | descs[2].unit->flags | descs[3].unit->flags) & PxcNpWorkUnitFlag::eDYNAMIC_BODY1) != 0;

	const PxU32 constraintSize = isDynamic ? sizeof(PxcSolverContactBatchPointDynamic4) : sizeof(PxcSolverContactBatchPointBase4);
	const PxU32 frictionSize = isDynamic ? sizeof(PxcSolverContactFrictionDynamic4) : sizeof(PxcSolverContactFrictionBase4);

	PxU8* PX_RESTRICT ptr = workspace;
	const Vec4V _dom0 = V4Merge(FLoad(descs[0].unit->dominance0), FLoad(descs[1].unit->dominance0), FLoad(descs[2].unit->dominance0),
		FLoad(descs[3].unit->dominance0));
	const Vec4V _dom1 = V4Merge(FLoad(descs[0].unit->dominance1), FLoad(descs[1].unit->dominance1), FLoad(descs[2].unit->dominance1),
		FLoad(descs[3].unit->dominance1));

	const Vec4V dom0 = V4Mul(_dom0, invMassScale0);
	const Vec4V dom1 = V4Mul(_dom1, invMassScale1);
	const Vec4V angDom0 = V4Mul(_dom0, invInertiaScale0);
	const Vec4V angDom1 = V4Mul(_dom1, invInertiaScale1);

	const Vec4V maxPenBias = V4Max(V4Merge(FLoad(descs[0].data0->penBiasClamp), FLoad(descs[1].data0->penBiasClamp), 
		FLoad(descs[2].data0->penBiasClamp), FLoad(descs[3].data0->penBiasClamp)), 
		V4Merge(FLoad(descs[0].data1->penBiasClamp), FLoad(descs[1].data1->penBiasClamp), 
		FLoad(descs[2].data1->penBiasClamp), FLoad(descs[3].data1->penBiasClamp)));

	const Vec4V nDom1 = V4Neg(dom1);

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

	//invInertia0X0 = (unit0.b0.invInertia.col0.x, unit1.b0.invInertia.col0.x, unit2.b0.invInertia.col0.x, unit3.b0.invInertia.col0.x)
	//invInertia0X1 = (unit0.b0.invInertia.col1.x, unit1.b0.invInertia.col1.x, unit2.b0.invInertia.col1.x, unit3.b0.invInertia.col1.x) etc.
	//invInertia0Y0 = (unit0.b0.invInertia.col0.y, unit1.b0.invInertia.col0.y, unit2.b0.invInertia.col0.y, unit3.b0.invInertia.col0.y) etc.

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
	const Vec4V p84 = V4Splat(p8);
	const Vec4V bounceThreshold = V4Splat(FLoad(bounceThresholdF32));

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


	const QuatV bodyFrame00q = QuatVLoadU(&descs[0].bodyFrame0->q.x);	
	const QuatV bodyFrame01q = QuatVLoadU(&descs[1].bodyFrame0->q.x);
	const QuatV bodyFrame02q = QuatVLoadU(&descs[2].bodyFrame0->q.x);
	const QuatV bodyFrame03q = QuatVLoadU(&descs[3].bodyFrame0->q.x);

	const QuatV bodyFrame10q = QuatVLoadU(&descs[0].bodyFrame1->q.x);	
	const QuatV bodyFrame11q = QuatVLoadU(&descs[1].bodyFrame1->q.x);
	const QuatV bodyFrame12q = QuatVLoadU(&descs[2].bodyFrame1->q.x);	
	const QuatV bodyFrame13q = QuatVLoadU(&descs[3].bodyFrame1->q.x);
	

	PxcCorrelationBuffer& c = threadContext.mCorrelationBuffer;

	PxU32 frictionPatchWritebackAddrIndex0 = 0;
	PxU32 frictionPatchWritebackAddrIndex1 = 0;
	PxU32 frictionPatchWritebackAddrIndex2 = 0;
	PxU32 frictionPatchWritebackAddrIndex3 = 0;

	Ps::prefetchLine(c.contactID);
	Ps::prefetchLine(c.contactID, 128);

	PxU32 frictionIndex0 = 0, frictionIndex1 = 0, frictionIndex2 = 0, frictionIndex3 = 0;
	//PxU32 contactIndex0 = 0, contactIndex1 = 0, contactIndex2 = 0, contactIndex3 = 0;


	//OK, we iterate through all friction patch counts in the constraint patch, building up the constraint list etc.

	PxU32 maxPatches = PxMax(descs[0].numFrictionPatches, PxMax(descs[1].numFrictionPatches, PxMax(descs[2].numFrictionPatches, descs[3].numFrictionPatches)));

	const Vec4V p1 = V4Splat(FLoad(0.1f));
	const Vec4V orthoThreshold = V4Splat(FLoad(0.70710678f));

	
	PxU32 contact0 = 0, contact1 = 0, contact2 = 0, contact3 = 0;
	PxU32 patch0 = 0, patch1 = 0, patch2 = 0, patch3 = 0;

	PxU8 flag = 0;
	if(hasMaxImpulse)
		flag |= PxcSolverContactHeader4::eHAS_MAX_IMPULSE;
	if(hasTargetVelocity)
		flag |= PxcSolverContactHeader4::eHAS_TARGET_VELOCITY;

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

		PxU32 firstPatch0 = c.correlationListHeads[frictionIndex0];
		PxU32 firstPatch1 = c.correlationListHeads[frictionIndex1];
		PxU32 firstPatch2 = c.correlationListHeads[frictionIndex2];
		PxU32 firstPatch3 = c.correlationListHeads[frictionIndex3];

		const Gu::ContactPoint* contactBase0 = threadContext.mContactBuffer.contacts + c.contactPatches[firstPatch0].start;
		const Gu::ContactPoint* contactBase1 = threadContext.mContactBuffer.contacts + c.contactPatches[firstPatch1].start;
		const Gu::ContactPoint* contactBase2 = threadContext.mContactBuffer.contacts + c.contactPatches[firstPatch2].start;
		const Gu::ContactPoint* contactBase3 = threadContext.mContactBuffer.contacts + c.contactPatches[firstPatch3].start;

		const Vec4V restitution = V4Neg(V4Merge(FLoad(contactBase0->restitution), FLoad(contactBase1->restitution), FLoad(contactBase2->restitution),
			FLoad(contactBase3->restitution)));

		PxcSolverContactHeader4* PX_RESTRICT header = reinterpret_cast<PxcSolverContactHeader4*>(ptr);
		ptr += sizeof(PxcSolverContactHeader4);	

		
		header->flags[0] = flags[0];
		header->flags[1] = flags[1];
		header->flags[2] = flags[2];
		header->flags[3] = flags[3];

		header->flag = flag;

		PxU32 totalContacts = PxMax(clampedContacts0, PxMax(clampedContacts1, PxMax(clampedContacts2, clampedContacts3)));

		Vec4V* PX_RESTRICT appliedNormalForces = (Vec4V*)ptr;
		ptr += sizeof(Vec4V)*totalContacts;

		PxMemZero(appliedNormalForces, sizeof(Vec4V) * totalContacts);

		header->numNormalConstr		= Ps::to8(totalContacts);
		header->numNormalConstr0 = Ps::to8(clampedContacts0);
		header->numNormalConstr1 = Ps::to8(clampedContacts1);
		header->numNormalConstr2 = Ps::to8(clampedContacts2);
		header->numNormalConstr3 = Ps::to8(clampedContacts3);
		header->invMassADom0 = invMass0_dom0fV;
		header->invMassBDom1 = invMass1_dom1fV;

		Vec4V* maxImpulse = (Vec4V*)(ptr + constraintSize * totalContacts);

		header->restitution = restitution;

		Vec4V normal0 = V4LoadA(&contactBase0->normal.x);
		Vec4V normal1 = V4LoadA(&contactBase1->normal.x);
		Vec4V normal2 = V4LoadA(&contactBase2->normal.x);
		Vec4V normal3 = V4LoadA(&contactBase3->normal.x);

		Vec4V normalX, normalY, normalZ;
		PX_TRANSPOSE_44_34(normal0, normal1, normal2, normal3, normalX, normalY, normalZ);

		header->normalX = normalX;
		header->normalY = normalY;
		header->normalZ = normalZ;

		const Vec4V norVel0 = V4MulAdd(normalZ, linVelT20, V4MulAdd(normalY, linVelT10, V4Mul(normalX, linVelT00)));
		const Vec4V norVel1 = V4MulAdd(normalZ, linVelT21, V4MulAdd(normalY, linVelT11, V4Mul(normalX, linVelT01)));
		const Vec4V relNorVel  = V4Sub(norVel0, norVel1);

		const Vec4V invMassNorLenSq0 = invMass0_dom0fV;//V4Mul(invMass0_dom0fV, normalLenSq);
		const Vec4V invMassNorLenSq1 = invMass1_dom1fV;//V4Mul(invMass1_dom1fV, normalLenSq);

		//For all correlation heads - need to pull this out I think

		//OK, we have a counter for all our patches...
		PxU32 finished = ((PxU32)(hasFinished0)) | 
						 (((PxU32)(hasFinished1)) << 1) | 
						 (((PxU32)(hasFinished2)) << 2) | 
						 (((PxU32)(hasFinished3)) << 3);

		CorrelationListIterator iter0(c, firstPatch0);
		CorrelationListIterator iter1(c, firstPatch1);
		CorrelationListIterator iter2(c, firstPatch2);
		CorrelationListIterator iter3(c, firstPatch3);

		//PxU32 contact0, contact1, contact2, contact3;
		//PxU32 patch0, patch1, patch2, patch3;

		if(!hasFinished0)
			iter0.nextContact(patch0, contact0);
		if(!hasFinished1)
			iter1.nextContact(patch1, contact1);
		if(!hasFinished2)
			iter2.nextContact(patch2, contact2);
		if(!hasFinished3)
			iter3.nextContact(patch3, contact3);

		PxU8* p = ptr;

		PxU32 contactCount = 0;
		PxU32 newFinished = 
			((PxU32)(hasFinished0 || !iter0.hasNextContact()))		| 
			(((PxU32)(hasFinished1 || !iter1.hasNextContact())) << 1) | 
			(((PxU32)(hasFinished2 || !iter2.hasNextContact())) << 2) | 
			(((PxU32)(hasFinished3 || !iter3.hasNextContact())) << 3);

		while(finished != 0xf)
		{
			finished = newFinished;
			++contactCount;
			Ps::prefetchLine(p, 384);
			Ps::prefetchLine(p, 512);
			Ps::prefetchLine(p, 640);	

			PxcSolverContactBatchPointBase4* PX_RESTRICT solverContact = reinterpret_cast<PxcSolverContactBatchPointBase4*>(p);
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

				const Vec4V cTargetNorVel = V4MulAdd(cTargetVelX, normalX, V4MulAdd(cTargetVelY, normalY, V4Mul(cTargetVelZ, normalZ)));

				const Vec4V raX = V4Sub(pointX, bodyFrame0pX);
				const Vec4V raY = V4Sub(pointY, bodyFrame0pY);
				const Vec4V raZ = V4Sub(pointZ, bodyFrame0pZ);

				const Vec4V rbX = V4Sub(pointX, bodyFrame1pX);
				const Vec4V rbY = V4Sub(pointY, bodyFrame1pY);
				const Vec4V rbZ = V4Sub(pointZ, bodyFrame1pZ);


				//raXn = cross(ra, normal) which = Vec3V( a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);

				const Vec4V raXnX = V4NegMulSub(raZ, normalY, V4Mul(raY, normalZ));
				const Vec4V raXnY = V4NegMulSub(raX, normalZ, V4Mul(raZ, normalX));
				const Vec4V raXnZ = V4NegMulSub(raY, normalX, V4Mul(raX, normalY));

				Vec4V delAngVel0X = V4Mul(invInertia0X0, raXnX);
				Vec4V delAngVel0Y = V4Mul(invInertia0X1, raXnX);
				Vec4V delAngVel0Z = V4Mul(invInertia0X2, raXnX);

				delAngVel0X = V4MulAdd(invInertia0Y0, raXnY, delAngVel0X);
				delAngVel0Y = V4MulAdd(invInertia0Y1, raXnY, delAngVel0Y);
				delAngVel0Z = V4MulAdd(invInertia0Y2, raXnY, delAngVel0Z);

				delAngVel0X = V4MulAdd(invInertia0Z0, raXnZ, delAngVel0X);
				delAngVel0Y = V4MulAdd(invInertia0Z1, raXnZ, delAngVel0Y);
				delAngVel0Z = V4MulAdd(invInertia0Z2, raXnZ, delAngVel0Z);

				const Vec4V dotRaXnDelAngVel0 = V4MulAdd(raXnZ, delAngVel0Z, V4MulAdd(raXnY, delAngVel0Y, V4Mul(raXnX, delAngVel0X)));
				const Vec4V dotRaXnAngVel0 = V4MulAdd(raXnZ, angVelT20, V4MulAdd(raXnY, angVelT10, V4Mul(raXnX, angVelT00)));

				Vec4V unitResponse = V4Add(invMassNorLenSq0, dotRaXnDelAngVel0);
				Vec4V vrel = V4Add(relNorVel, dotRaXnAngVel0);


				//The dynamic-only parts - need to if-statement these up. A branch here shouldn't cost us too much
				if(isDynamic)
				{
					PxcSolverContactBatchPointDynamic4* PX_RESTRICT dynamicContact = static_cast<PxcSolverContactBatchPointDynamic4*>(solverContact);
					const Vec4V rbXnX = V4NegMulSub(rbZ, normalY, V4Mul(rbY, normalZ));
					const Vec4V rbXnY = V4NegMulSub(rbX, normalZ, V4Mul(rbZ, normalX));
					const Vec4V rbXnZ = V4NegMulSub(rbY, normalX, V4Mul(rbX, normalY));

					Vec4V delAngVel1X = V4Mul(invInertia1X0, rbXnX);
					Vec4V delAngVel1Y = V4Mul(invInertia1X1, rbXnX);
					Vec4V delAngVel1Z = V4Mul(invInertia1X2, rbXnX);

					delAngVel1X = V4MulAdd(invInertia1Y0, rbXnY, delAngVel1X);
					delAngVel1Y = V4MulAdd(invInertia1Y1, rbXnY, delAngVel1Y);
					delAngVel1Z = V4MulAdd(invInertia1Y2, rbXnY, delAngVel1Z);

					delAngVel1X = V4MulAdd(invInertia1Z0, rbXnZ, delAngVel1X);
					delAngVel1Y = V4MulAdd(invInertia1Z1, rbXnZ, delAngVel1Y);
					delAngVel1Z = V4MulAdd(invInertia1Z2, rbXnZ, delAngVel1Z);


					//V3Dot(raXn, delAngVel0)
					
					const Vec4V dotRbXnDelAngVel1 = V4MulAdd(rbXnZ, delAngVel1Z, V4MulAdd(rbXnY, delAngVel1Y, V4Mul(rbXnX, delAngVel1X)));
					
					const Vec4V dotRbXnAngVel1 = V4MulAdd(rbXnZ, angVelT21, V4MulAdd(rbXnY, angVelT11, V4Mul(rbXnX, angVelT01)));

					const Vec4V resp1 = V4Sub(dotRbXnDelAngVel1, invMassNorLenSq1);

					unitResponse = V4Add(unitResponse, resp1);

					vrel = V4Sub(vrel, dotRbXnAngVel1);

					//These are for dynamic-only contacts.
					dynamicContact->rbXnX = rbXnX;
					dynamicContact->rbXnY = rbXnY;
					dynamicContact->rbXnZ = rbXnZ;
					dynamicContact->delAngVel1X = V4Neg(delAngVel1X);
					dynamicContact->delAngVel1Y = V4Neg(delAngVel1Y);
					dynamicContact->delAngVel1Z = V4Neg(delAngVel1Z);

				}

				const Vec4V velMultiplier = V4Sel(V4IsGrtr(unitResponse, zero), V4Recip(unitResponse), zero);

				const Vec4V penetration = V4Sub(separation, restDistance);
				const Vec4V penInvDtPt8 = V4Max(maxPenBias, V4Scale(penetration, invDtp8));
				Vec4V scaledBias = V4Mul(penInvDtPt8, velMultiplier);

				const Vec4V penetrationInvDt = V4Scale(penetration, invDt);

				const BoolV isGreater2 = BAnd(BAnd(V4IsGrtr(zero, restitution), V4IsGrtr(bounceThreshold, vrel)), 
					V4IsGrtr(V4Neg(vrel), penetrationInvDt));

				scaledBias = V4Sel(isGreater2, zero, V4Neg(scaledBias));

				const Vec4V targetVelocity = V4Mul(velMultiplier, V4Mul(vrel, restitution));

				//const Vec4V targetVelocity = V4Sel(isGreater2, V4Mul(V4Neg(sumVRel), restitution), zero);

				//These values are present for static and dynamic contacts			
				solverContact->raXnX = raXnX;
				solverContact->raXnY = raXnY;
				solverContact->raXnZ = raXnZ;
				solverContact->velMultiplier = velMultiplier;
				solverContact->biasedErr = V4Add(V4Sel(isGreater2, targetVelocity, scaledBias), cTargetNorVel);
				solverContact->scaledBias = V4Sel(V4IsGrtrOrEq(zero, scaledBias), zero, scaledBias);
				//solverContact->targetVelocity = V4Mul(targetVelocity, velMultiplier);
				solverContact->delAngVel0X = delAngVel0X;
				solverContact->delAngVel0Y = delAngVel0Y;
				solverContact->delAngVel0Z = delAngVel0Z;			

				if(hasMaxImpulse)
				{
					maxImpulse[contactCount-1] = V4Merge(FLoad(con0.maxImpulse), FLoad(con1.maxImpulse), FLoad(con2.maxImpulse),
						FLoad(con3.maxImpulse));					
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
		if(hasMaxImpulse)
		{
			ptr += sizeof(Vec4V) * totalContacts;
		}

		//OK...friction time :-)

		Vec4V maxImpulseScale = V4One();
		{
			const Vec4V staticFriction = V4Merge(FLoad(contactBase0->staticFriction), FLoad(contactBase1->staticFriction),
				FLoad(contactBase2->staticFriction), FLoad(contactBase3->staticFriction));

			const Vec4V dynamicFriction = V4Merge(FLoad(contactBase0->dynamicFriction), FLoad(contactBase1->dynamicFriction),
				FLoad(contactBase2->dynamicFriction), FLoad(contactBase3->dynamicFriction));			

			PX_ASSERT(totalContacts == contactCount);
			header->dynamicFriction = dynamicFriction;
			header->staticFriction = staticFriction;

			const PxcFrictionPatch& frictionPatch0 = c.frictionPatches[frictionIndex0];
			const PxcFrictionPatch& frictionPatch1 = c.frictionPatches[frictionIndex1];
			const PxcFrictionPatch& frictionPatch2 = c.frictionPatches[frictionIndex2];
			const PxcFrictionPatch& frictionPatch3 = c.frictionPatches[frictionIndex3];

			PxU32 anchorCount0 = frictionPatch0.anchorCount;
			PxU32 anchorCount1 = frictionPatch1.anchorCount;
			PxU32 anchorCount2 = frictionPatch2.anchorCount;
			PxU32 anchorCount3 = frictionPatch3.anchorCount;

			PxU32 clampedAnchorCount0 = hasFinished0 || (contactBase0->internalFaceIndex1 & PxMaterialFlag::eDISABLE_FRICTION) ? 0 : anchorCount0;
			PxU32 clampedAnchorCount1 = hasFinished1 || (contactBase1->internalFaceIndex1 & PxMaterialFlag::eDISABLE_FRICTION) ? 0 : anchorCount1;
			PxU32 clampedAnchorCount2 = hasFinished2 || (contactBase2->internalFaceIndex1 & PxMaterialFlag::eDISABLE_FRICTION) ? 0 : anchorCount2;
			PxU32 clampedAnchorCount3 = hasFinished3 || (contactBase3->internalFaceIndex1 & PxMaterialFlag::eDISABLE_FRICTION) ? 0 : anchorCount3;
			
			const PxU32 maxAnchorCount = PxMax(clampedAnchorCount0, PxMax(clampedAnchorCount1, PxMax(clampedAnchorCount2, clampedAnchorCount3)));

			//if(clampedAnchorCount0 != clampedAnchorCount1 || clampedAnchorCount0 != clampedAnchorCount2 || clampedAnchorCount0 != clampedAnchorCount3)
			//	Ps::debugBreak();


			//const bool haveFriction = maxAnchorCount != 0;
			header->numFrictionConstr	= Ps::to8(maxAnchorCount*2);
			header->numFrictionConstr0 = Ps::to8(clampedAnchorCount0*2);
			header->numFrictionConstr1 = Ps::to8(clampedAnchorCount1*2);
			header->numFrictionConstr2 = Ps::to8(clampedAnchorCount2*2);
			header->numFrictionConstr3 = Ps::to8(clampedAnchorCount3*2);
		
			//KS - TODO - extend this if needed
			header->type = Ps::to8(isDynamic ? PXS_SC_TYPE_BLOCK_RB_CONTACT : PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT);

			if(maxAnchorCount)
			{

				//Allocate the shared friction data...

				PxcSolverFrictionSharedData4* PX_RESTRICT fd = reinterpret_cast<PxcSolverFrictionSharedData4*>(ptr);
				ptr += sizeof(PxcSolverFrictionSharedData4);
				PX_UNUSED(fd);

				const BoolV cond =V4IsGrtr(orthoThreshold, V4Abs(normalX));

				const Vec4V t0FallbackX = V4Sel(cond, zero, V4Neg(normalY));
				const Vec4V t0FallbackY = V4Sel(cond, V4Neg(normalZ), normalX);
				const Vec4V t0FallbackZ = V4Sel(cond, normalY, zero);

				//const Vec4V dotNormalVrel = V4MulAdd(normalZ, vrelZ, V4MulAdd(normalY, vrelY, V4Mul(normalX, vrelX)));
				const Vec4V vrelSubNorVelX = V4NegMulSub(normalX, relNorVel, vrelX);
				const Vec4V vrelSubNorVelY = V4NegMulSub(normalY, relNorVel, vrelY);
				const Vec4V vrelSubNorVelZ = V4NegMulSub(normalZ, relNorVel, vrelZ);

				const Vec4V lenSqvrelSubNorVelZ = V4MulAdd(vrelSubNorVelX, vrelSubNorVelX, V4MulAdd(vrelSubNorVelY, vrelSubNorVelY, V4Mul(vrelSubNorVelZ, vrelSubNorVelZ)));

				const BoolV bcon2 = V4IsGrtr(lenSqvrelSubNorVelZ, p1);

				Vec4V t0X = V4Sel(bcon2, vrelSubNorVelX, t0FallbackX);
				Vec4V t0Y = V4Sel(bcon2, vrelSubNorVelY, t0FallbackY);
				Vec4V t0Z = V4Sel(bcon2, vrelSubNorVelZ, t0FallbackZ);



				/*Vec4V t0X = V4NegMulSub(normalY, t0FallbackZ, V4Mul(normalZ, t0FallbackY));
				Vec4V t0Y = V4NegMulSub(normalZ, t0FallbackX, V4Mul(normalX, t0FallbackZ));
				Vec4V t0Z = V4NegMulSub(normalX, t0FallbackY, V4Mul(normalY, t0FallbackX));*/

				//Now normalize this...
				const Vec4V recipLen = V4Rsqrt(V4MulAdd(t0Z, t0Z, V4MulAdd(t0Y, t0Y, V4Mul(t0X, t0X))));

				t0X = V4Mul(t0X, recipLen);
				t0Y = V4Mul(t0Y, recipLen);
				t0Z = V4Mul(t0Z, recipLen);

				/*Vec4V t1X = V4NegMulSub(normalY, t0Z, V4Mul(normalZ, t0Y));
				Vec4V t1Y = V4NegMulSub(normalZ, t0X, V4Mul(normalX, t0Z));
				Vec4V t1Z = V4NegMulSub(normalX, t0Y, V4Mul(normalY, t0X));*/

				Vec4V t1X = V4NegMulSub(normalZ, t0Y, V4Mul(normalY, t0Z));
				Vec4V t1Y = V4NegMulSub(normalX, t0Z, V4Mul(normalZ, t0X));
				Vec4V t1Z = V4NegMulSub(normalY, t0X, V4Mul(normalX, t0Y));


				PxU8* PX_RESTRICT writeback0 = descs[0].unit->frictionDataPtr + frictionPatchWritebackAddrIndex0*sizeof(PxcFrictionPatch);
				PxU8* PX_RESTRICT writeback1 = descs[1].unit->frictionDataPtr + frictionPatchWritebackAddrIndex1*sizeof(PxcFrictionPatch);
				PxU8* PX_RESTRICT writeback2 = descs[2].unit->frictionDataPtr + frictionPatchWritebackAddrIndex2*sizeof(PxcFrictionPatch);
				PxU8* PX_RESTRICT writeback3 = descs[3].unit->frictionDataPtr + frictionPatchWritebackAddrIndex3*sizeof(PxcFrictionPatch);

				PxU32 index0 = 0, index1 = 0, index2 = 0, index3 = 0;

				fd->broken = bFalse;
				fd->frictionBrokenWritebackByte[0] = writeback0;
				fd->frictionBrokenWritebackByte[1] = writeback1;
				fd->frictionBrokenWritebackByte[2] = writeback2;
				fd->frictionBrokenWritebackByte[3] = writeback3;


				fd->normalX[0] = t0X;
				fd->normalY[0] = t0Y;
				fd->normalZ[0] = t0Z;

				fd->normalX[1] = t1X;
				fd->normalY[1] = t1Y;
				fd->normalZ[1] = t1Z;

				Vec4V* PX_RESTRICT appliedForces = (Vec4V*)ptr;
				ptr += sizeof(Vec4V)*header->numFrictionConstr;

				PxMemZero(appliedForces, sizeof(Vec4V) * header->numFrictionConstr);

				Vec4V* targetVelocities = (Vec4V*)(ptr + frictionSize * header->numFrictionConstr);

				for(PxU32 j = 0; j < maxAnchorCount; j++)
				{
					Ps::prefetchLine(ptr, 384);
					Ps::prefetchLine(ptr, 512);
					Ps::prefetchLine(ptr, 640);
					PxcSolverContactFrictionBase4* PX_RESTRICT f0 = reinterpret_cast<PxcSolverContactFrictionBase4*>(ptr);
					ptr += frictionSize;
					PxcSolverContactFrictionBase4* PX_RESTRICT f1 = reinterpret_cast<PxcSolverContactFrictionBase4*>(ptr);
					ptr += frictionSize;

					index0 = j < clampedAnchorCount0 ? j : index0;
					index1 = j < clampedAnchorCount1 ? j : index1;
					index2 = j < clampedAnchorCount2 ? j : index2;
					index3 = j < clampedAnchorCount3 ? j : index3;

					if(j >= clampedAnchorCount0)
						maxImpulseScale = V4SetX(maxImpulseScale, fZero);
					if(j >= clampedAnchorCount1)
						maxImpulseScale = V4SetY(maxImpulseScale, fZero);
					if(j >= clampedAnchorCount2)
						maxImpulseScale = V4SetZ(maxImpulseScale, fZero);
					if(j >= clampedAnchorCount3)
						maxImpulseScale = V4SetW(maxImpulseScale, fZero);

					t0X = V4Mul(maxImpulseScale, t0X);
					t0Y = V4Mul(maxImpulseScale, t0Y);
					t0Z = V4Mul(maxImpulseScale, t0Z);

					t1X = V4Mul(maxImpulseScale, t1X);
					t1Y = V4Mul(maxImpulseScale, t1Y);
					t1Z = V4Mul(maxImpulseScale, t1Z);


					Vec3V body0Anchor0 = V3LoadU(frictionPatch0.body0Anchors[index0]);
					Vec3V body0Anchor1 = V3LoadU(frictionPatch1.body0Anchors[index1]);
					Vec3V body0Anchor2 = V3LoadU(frictionPatch2.body0Anchors[index2]);
					Vec3V body0Anchor3 = V3LoadU(frictionPatch3.body0Anchors[index3]);

					Vec4V ra0 = Vec4V_From_Vec3V(QuatRotate(bodyFrame00q, body0Anchor0));
					Vec4V ra1 = Vec4V_From_Vec3V(QuatRotate(bodyFrame01q, body0Anchor1));
					Vec4V ra2 = Vec4V_From_Vec3V(QuatRotate(bodyFrame02q, body0Anchor2));
					Vec4V ra3 = Vec4V_From_Vec3V(QuatRotate(bodyFrame03q, body0Anchor3));

					Vec4V raX, raY, raZ;
					PX_TRANSPOSE_44_34(ra0, ra1, ra2, ra3, raX, raY, raZ);

					const Vec4V raWorldX = V4Add(raX, bodyFrame0pX);
					const Vec4V raWorldY = V4Add(raY, bodyFrame0pY);
					const Vec4V raWorldZ = V4Add(raZ, bodyFrame0pZ);

					Vec3V body1Anchor0 = V3LoadU(frictionPatch0.body1Anchors[index0]);	
					Vec3V body1Anchor1 = V3LoadU(frictionPatch1.body1Anchors[index1]);
					Vec3V body1Anchor2 = V3LoadU(frictionPatch2.body1Anchors[index2]);
					Vec3V body1Anchor3 = V3LoadU(frictionPatch3.body1Anchors[index3]);
				
					Vec4V rb0 = Vec4V_From_Vec3V(QuatRotate(bodyFrame10q, body1Anchor0));
					Vec4V rb1 = Vec4V_From_Vec3V(QuatRotate(bodyFrame11q, body1Anchor1));
					Vec4V rb2 = Vec4V_From_Vec3V(QuatRotate(bodyFrame12q, body1Anchor2));
					Vec4V rb3 = Vec4V_From_Vec3V(QuatRotate(bodyFrame13q, body1Anchor3));

					Vec4V rbX, rbY, rbZ;
					PX_TRANSPOSE_44_34(rb0, rb1, rb2, rb3, rbX, rbY, rbZ);

					const Vec4V rbWorldX = V4Add(rbX, bodyFrame1pX);
					const Vec4V rbWorldY = V4Add(rbY, bodyFrame1pY);
					const Vec4V rbWorldZ = V4Add(rbZ, bodyFrame1pZ);

					const Vec4V errorX = V4Sub(raWorldX, rbWorldX);
					const Vec4V errorY = V4Sub(raWorldY, rbWorldY);
					const Vec4V errorZ = V4Sub(raWorldZ, rbWorldZ);
					
					{
						const Vec4V raXnX = V4NegMulSub(raZ, t0Y, V4Mul(raY, t0Z));
						const Vec4V raXnY = V4NegMulSub(raX, t0Z, V4Mul(raZ, t0X));
						const Vec4V raXnZ = V4NegMulSub(raY, t0X, V4Mul(raX, t0Y));

						Vec4V delAngVel0X = V4Mul(invInertia0X0, raXnX);
						Vec4V delAngVel0Y = V4Mul(invInertia0X1, raXnX);
						Vec4V delAngVel0Z = V4Mul(invInertia0X2, raXnX);

						delAngVel0X = V4MulAdd(invInertia0Y0, raXnY, delAngVel0X);
						delAngVel0Y = V4MulAdd(invInertia0Y1, raXnY, delAngVel0Y);
						delAngVel0Z = V4MulAdd(invInertia0Y2, raXnY, delAngVel0Z);

						delAngVel0X = V4MulAdd(invInertia0Z0, raXnZ, delAngVel0X);
						delAngVel0Y = V4MulAdd(invInertia0Z1, raXnZ, delAngVel0Y);
						delAngVel0Z = V4MulAdd(invInertia0Z2, raXnZ, delAngVel0Z);

						const Vec4V dotRaXnDelAngVel0 = V4MulAdd(raXnZ, delAngVel0Z, V4MulAdd(raXnY, delAngVel0Y, V4Mul(raXnX, delAngVel0X)));
					
						Vec4V resp = V4Add(invMass0_dom0fV, dotRaXnDelAngVel0);

						if(isDynamic)
						{
							PxcSolverContactFrictionDynamic4* PX_RESTRICT dynamicF0 = static_cast<PxcSolverContactFrictionDynamic4*>(f0);

							const Vec4V rbXnX = V4NegMulSub(rbZ, t0Y, V4Mul(rbY, t0Z));
							const Vec4V rbXnY = V4NegMulSub(rbX, t0Z, V4Mul(rbZ, t0X));
							const Vec4V rbXnZ = V4NegMulSub(rbY, t0X, V4Mul(rbX, t0Y));

							Vec4V delAngVel1X = V4Mul(invInertia1X0, rbXnX);
							Vec4V delAngVel1Y = V4Mul(invInertia1X1, rbXnX);
							Vec4V delAngVel1Z = V4Mul(invInertia1X2, rbXnX);

							delAngVel1X = V4MulAdd(invInertia1Y0, rbXnY, delAngVel1X);
							delAngVel1Y = V4MulAdd(invInertia1Y1, rbXnY, delAngVel1Y);
							delAngVel1Z = V4MulAdd(invInertia1Y2, rbXnY, delAngVel1Z);

							delAngVel1X = V4MulAdd(invInertia1Z0, rbXnZ, delAngVel1X);
							delAngVel1Y = V4MulAdd(invInertia1Z1, rbXnZ, delAngVel1Y);
							delAngVel1Z = V4MulAdd(invInertia1Z2, rbXnZ, delAngVel1Z);					
						
							const Vec4V dotRbXnDelAngVel1 = V4MulAdd(rbXnZ, delAngVel1Z, V4MulAdd(rbXnY, delAngVel1Y, V4Mul(rbXnX, delAngVel1X)));
							
							const Vec4V resp1 = V4Sub(dotRbXnDelAngVel1, invMass1_dom1fV);

							resp = V4Add(resp, resp1);
							
							dynamicF0->rbXnX = rbXnX;
							dynamicF0->rbXnY = rbXnY;
							dynamicF0->rbXnZ = rbXnZ;
							dynamicF0->delAngVel1X = V4Neg(delAngVel1X);
							dynamicF0->delAngVel1Y = V4Neg(delAngVel1Y);
							dynamicF0->delAngVel1Z = V4Neg(delAngVel1Z);

						}


						const Vec4V velMultiplier = V4Mul(maxImpulseScale, V4Sel(V4IsGrtr(resp, zero), V4Div(p84, resp), zero));

						//KS - todo - get this working with per-point friction
						//PxU32 index0 = /*perPointFriction ? c.contactID[i][j] : */c.contactPatches[c.correlationListHeads[i]].start;

						Vec4V targetVel0 = V4LoadA(&contactBase0->targetVel.x);
						Vec4V targetVel1 = V4LoadA(&contactBase1->targetVel.x);
						Vec4V targetVel2 = V4LoadA(&contactBase2->targetVel.x);
						Vec4V targetVel3 = V4LoadA(&contactBase3->targetVel.x);

						Vec4V targetVelX, targetVelY, targetVelZ;
						PX_TRANSPOSE_44_34(targetVel0, targetVel1, targetVel2, targetVel3, targetVelX, targetVelY, targetVelZ);

						const Vec4V targetVel = V4MulAdd(t0Z, targetVelZ,V4MulAdd(t0Y, targetVelY, V4Mul(t0X, targetVelX)));

						const Vec4V bias = V4Scale(V4MulAdd(t0Z, errorZ, V4MulAdd(t0Y, errorY, V4Mul(t0X, errorX))), invDt);

						f0->raXnX = raXnX;
						f0->raXnY = raXnY;
						f0->raXnZ = raXnZ;
						f0->delAngVel0X = delAngVel0X;
						f0->delAngVel0Y = delAngVel0Y;
						f0->delAngVel0Z = delAngVel0Z;
						f0->scaledBias = V4Mul(V4Sub(bias, targetVel), velMultiplier);
						f0->velMultiplier = velMultiplier;	
						if(hasTargetVelocity)
						{
							*targetVelocities = V4Neg(V4Mul(targetVel, velMultiplier));
							targetVelocities++;
						}
					}

					{
						const Vec4V raXnX = V4NegMulSub(raZ, t1Y, V4Mul(raY, t1Z));
						const Vec4V raXnY = V4NegMulSub(raX, t1Z, V4Mul(raZ, t1X));
						const Vec4V raXnZ = V4NegMulSub(raY, t1X, V4Mul(raX, t1Y));

						Vec4V delAngVel0X = V4Mul(invInertia0X0, raXnX);
						Vec4V delAngVel0Y = V4Mul(invInertia0X1, raXnX);
						Vec4V delAngVel0Z = V4Mul(invInertia0X2, raXnX);

						delAngVel0X = V4MulAdd(invInertia0Y0, raXnY, delAngVel0X);
						delAngVel0Y = V4MulAdd(invInertia0Y1, raXnY, delAngVel0Y);
						delAngVel0Z = V4MulAdd(invInertia0Y2, raXnY, delAngVel0Z);

						delAngVel0X = V4MulAdd(invInertia0Z0, raXnZ, delAngVel0X);
						delAngVel0Y = V4MulAdd(invInertia0Z1, raXnZ, delAngVel0Y);
						delAngVel0Z = V4MulAdd(invInertia0Z2, raXnZ, delAngVel0Z);

						const Vec4V dotRaXnDelAngVel0 = V4MulAdd(raXnZ, delAngVel0Z, V4MulAdd(raXnY, delAngVel0Y, V4Mul(raXnX, delAngVel0X)));
					
						Vec4V resp = V4Add(invMass0_dom0fV, dotRaXnDelAngVel0);

						if(isDynamic)
						{
							PxcSolverContactFrictionDynamic4* PX_RESTRICT dynamicF1 = static_cast<PxcSolverContactFrictionDynamic4*>(f1);

							const Vec4V rbXnX = V4NegMulSub(rbZ, t1Y, V4Mul(rbY, t1Z));
							const Vec4V rbXnY = V4NegMulSub(rbX, t1Z, V4Mul(rbZ, t1X));
							const Vec4V rbXnZ = V4NegMulSub(rbY, t1X, V4Mul(rbX, t1Y));

							Vec4V delAngVel1X = V4Mul(invInertia1X0, rbXnX);
							Vec4V delAngVel1Y = V4Mul(invInertia1X1, rbXnX);
							Vec4V delAngVel1Z = V4Mul(invInertia1X2, rbXnX);

							delAngVel1X = V4MulAdd(invInertia1Y0, rbXnY, delAngVel1X);
							delAngVel1Y = V4MulAdd(invInertia1Y1, rbXnY, delAngVel1Y);
							delAngVel1Z = V4MulAdd(invInertia1Y2, rbXnY, delAngVel1Z);

							delAngVel1X = V4MulAdd(invInertia1Z0, rbXnZ, delAngVel1X);
							delAngVel1Y = V4MulAdd(invInertia1Z1, rbXnZ, delAngVel1Y);
							delAngVel1Z = V4MulAdd(invInertia1Z2, rbXnZ, delAngVel1Z);					
						
							const Vec4V dotRbXnDelAngVel1 = V4MulAdd(rbXnZ, delAngVel1Z, V4MulAdd(rbXnY, delAngVel1Y, V4Mul(rbXnX, delAngVel1X)));
							
							const Vec4V resp1 = V4Sub(dotRbXnDelAngVel1, invMass1_dom1fV);

							resp = V4Add(resp, resp1);
							
							dynamicF1->rbXnX = rbXnX;
							dynamicF1->rbXnY = rbXnY;
							dynamicF1->rbXnZ = rbXnZ;
							dynamicF1->delAngVel1X = V4Neg(delAngVel1X);
							dynamicF1->delAngVel1Y = V4Neg(delAngVel1Y);
							dynamicF1->delAngVel1Z = V4Neg(delAngVel1Z);

						}


						const Vec4V velMultiplier = V4Mul(maxImpulseScale, V4Sel(V4IsGrtr(resp, zero), V4Div(p84, resp), zero));

						//KS - todo - get this working with per-point friction
						//PxU32 index0 = /*perPointFriction ? c.contactID[i][j] : */c.contactPatches[c.correlationListHeads[i]].start;

						Vec4V targetVel0 = V4LoadA(&contactBase0->targetVel.x);
						Vec4V targetVel1 = V4LoadA(&contactBase1->targetVel.x);
						Vec4V targetVel2 = V4LoadA(&contactBase2->targetVel.x);
						Vec4V targetVel3 = V4LoadA(&contactBase3->targetVel.x);

						Vec4V targetVelX, targetVelY, targetVelZ;
						PX_TRANSPOSE_44_34(targetVel0, targetVel1, targetVel2, targetVel3, targetVelX, targetVelY, targetVelZ);

						const Vec4V targetVel = V4MulAdd(t1Z, targetVelZ,V4MulAdd(t1Y, targetVelY, V4Mul(t1X, targetVelX)));

						const Vec4V bias = V4Scale(V4MulAdd(t1Z, errorZ, V4MulAdd(t1Y, errorY, V4Mul(t1X, errorX))), invDt);

						f1->raXnX = raXnX;
						f1->raXnY = raXnY;
						f1->raXnZ = raXnZ;
						f1->delAngVel0X = delAngVel0X;
						f1->delAngVel0Y = delAngVel0Y;
						f1->delAngVel0Z = delAngVel0Z;
						f1->scaledBias = V4Mul(V4Sub(bias, targetVel), velMultiplier);
						f1->velMultiplier = velMultiplier;
						if(hasTargetVelocity)
						{
							*targetVelocities = V4Neg(V4Mul(targetVel, velMultiplier));
							targetVelocities++;
						}
					}				
				}
				if(hasTargetVelocity)
					ptr += sizeof(Vec4V) * maxAnchorCount*2;

				frictionPatchWritebackAddrIndex0++;
				frictionPatchWritebackAddrIndex1++;
				frictionPatchWritebackAddrIndex2++;
				frictionPatchWritebackAddrIndex3++;
			}
		}
	}

	PX_ASSERT(PxU32(ptr - workspace) == descs[0].unit->solverConstraintSize);
}



