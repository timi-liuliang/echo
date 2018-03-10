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


#include "PxsConstraint.h"
#include "PxsRigidBody.h"
#include "PxsSolverConstraint1D.h"
#include "PsSort.h"
#include "PxcSolverConstraintDesc.h"
#include "PxcConstraintBlockStream.h"
#include "PxsSolverConstraintExt.h"
#include "PxcSolverConstraintTypes.h"

#ifdef __SPU__
#include "../include/spu/SpuNpMemBlock.h"
#endif

namespace physx
{

void preprocessRows(Px1DConstraint** sorted, 
					Px1DConstraint* rows,
					PxU32 rowCount,
					const PxcSolverBodyData& bd0,
					const PxcSolverBodyData& bd1,
					const PxConstraintInvMassScale& ims,
					bool isExtended,
					bool diagonalizeDrive);


namespace
{
void setConstants(PxReal& constant, PxReal& unbiasedConstant, PxReal& velMultiplier, PxReal& impulseMultiplier,
				  const Px1DConstraint& c, PxReal unitResponse, PxReal erp, PxReal dt, PxReal recipdt,
				  const PxcSolverBody& b0, const PxcSolverBody& b1, bool finished)
{
	if(finished)
	{
		constant = 0.f;
		unbiasedConstant = 0.f;
		velMultiplier = 0.f;
		impulseMultiplier = 0.f;
		return;
	}
	PxReal nv = needsNormalVel(c) ? b0.projectVelocity(c.linear0, c.angular0) - b1.projectVelocity(c.linear1, c.angular1)
								  : 0;
	
	setSolverConstants(constant, unbiasedConstant, velMultiplier, impulseMultiplier, 
					   c, nv, unitResponse, erp, dt, recipdt);
}
}

PxcSolverConstraintPrepState::Enum setupSolverConstraint4
(PxcSolverConstraint4Desc* PX_RESTRICT constraintDescs, 
 const PxReal dt, const PxReal recipdt,
#ifdef __SPU__
 SpuNpMemBlock& blockStream, uintptr_t* eaConstraintData)
#else
 PxcConstraintBlockStream& blockStream, PxsConstraintBlockManager& constraintBlockManager)
#endif
{	
	//KS - we will never get here with constraints involving articulations so we don't need to stress about those in here

	const Vec4V zero = V4Zero();

	Px1DConstraint allRows[PxsConstraint::MAX_CONSTRAINTS*4];
	Px1DConstraint* allSorted[PxsConstraint::MAX_CONSTRAINTS*4];
	PxU32 numRows = 0;

	PxU32 maxRows = 0;
	PxU32 preppedIndex = 0;

	PxVec3 body0WorldOffset[4] = {PxVec3(0), PxVec3(0), PxVec3(0), PxVec3(0)};

	PxConstraintInvMassScale ims[4] = { {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1} };
	for(PxU32 a = 0; a < 4; ++a)
	{
		Px1DConstraint* rows = allRows + numRows;
		PxU32 startIndex = numRows;
		PxcSolverConstraint4Desc& desc = constraintDescs[a];

		if(!desc.solverPrep)
			return PxcSolverConstraintPrepState::eUNBATCHABLE;

		for(PxU32 b = preppedIndex; b < PxsConstraint::MAX_CONSTRAINTS; ++b)
		{
			Px1DConstraint& c = rows[b];
			Px1DConstraintInit(c);
		}

		PxU32 constraintCount = (*desc.solverPrep)(rows,
												   body0WorldOffset[a],
												   PxsConstraint::MAX_CONSTRAINTS,
												   ims[a],
												   desc.constantBlock, 
												   *desc.pose0, *desc.pose1);

		preppedIndex = PxsConstraint::MAX_CONSTRAINTS - constraintCount;

		maxRows = PxMax(constraintCount, maxRows);

		if(constraintCount==0)
			return PxcSolverConstraintPrepState::eUNBATCHABLE;

		desc.startRowIndex = startIndex;
		desc.numRows = constraintCount;
		numRows += constraintCount;

		Px1DConstraint** sorted = allSorted + startIndex;

		preprocessRows(sorted, rows, constraintCount, *desc.sBodyData0, *desc.sBodyData1, ims[a], false, (desc.constraint->flags&PxConstraintFlag::eIMPROVED_SLERP)!=0);
	}


	PxU32 stride = sizeof(PxcSolverConstraint1DDynamic4);

	
	const PxU32 constraintLength = sizeof(PxcSolverConstraint1DHeader4) + stride * maxRows;

#ifdef __SPU__
	//Need to perform a single reserve to avoid the situation where the second reserve forces 
	//a dma writeback before we have filled out the ls data returned by the first reserve.
	PX_ASSERT(eaConstraintData && NULL == *eaConstraintData);
	PX_ASSERT(eaSolverOutput && NULL == *eaSolverOutput);
	uintptr_t ea=NULL;
	PxU8* ptr = blockStream.reserve(constraintLength + 16u, &ea);
	if(0==ptr)
	{
		return -1;
	}
	//desc.constraint = ptr;
	*eaConstraintData = ea;
#else
	//KS - +16 is for the constraint progress counter, which needs to be the last element in the constraint (so that we
	//know SPU DMAs have completed)
	PxU8* ptr = blockStream.reserve(constraintLength + 16u, constraintBlockManager);
	if(NULL == ptr || ((PxU8*)(-1))==ptr)
	{
		for(PxU32 a = 0; a < 4; ++a)
		{
			PxcSolverConstraint4Desc& desc = constraintDescs[a];
			desc.desc->constraint = NULL;
			setConstraintLength(*desc.desc, 0);
			desc.desc->writeBack = desc.constraint->writeback;
		}

		if(NULL==ptr)
		{
			PX_WARN_ONCE(true,
				"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for constraint prep. "
				"Either accept joints detaching/exploding or increase buffer size allocated for constraint prep by increasing PxSceneDesc::maxNbContactDataBlocks.");
			return PxcSolverConstraintPrepState::eOUT_OF_MEMORY;
		}
		else
		{
			PX_WARN_ONCE(true,
				"Attempting to allocate more than 16K of constraint data. "
				"Either accept joints detaching/exploding or simplify constraints.");
			ptr=NULL;
			return PxcSolverConstraintPrepState::eOUT_OF_MEMORY;
		}
	}
	//desc.constraint = ptr;
#endif

	for(PxU32 a = 0; a < 4; ++a)
	{
		PxcSolverConstraint4Desc& desc = constraintDescs[a];
		desc.desc->constraint = ptr;
		setConstraintLength(*desc.desc, constraintLength);
		desc.desc->writeBack = desc.constraint->writeback;
	}

	PxReal erp[4];
	for(PxU32 a = 0; a < 4; ++a)
		erp[a] = constraintDescs[a].constraint->flags & PxConstraintFlag::eDEPRECATED_32_COMPATIBILITY ? 0.7f : 1.0f;

	//OK, now we build all 4 constraints into a single set of rows

	{
		PxU8* currPtr = ptr;
		PxcSolverConstraint1DHeader4* header = (PxcSolverConstraint1DHeader4*)currPtr;
		currPtr += sizeof(PxcSolverConstraint1DHeader4);

		//const PxcSolverBody& b00 = *constraintDescs[0].solverBody0;
		//const PxcSolverBody& b01 = *constraintDescs[1].solverBody0;
		//const PxcSolverBody& b02 = *constraintDescs[2].solverBody0;
		//const PxcSolverBody& b03 = *constraintDescs[3].solverBody0;

		//const PxcSolverBody& b10 = *constraintDescs[0].solverBody1;
		//const PxcSolverBody& b11 = *constraintDescs[1].solverBody1;
		//const PxcSolverBody& b12 = *constraintDescs[2].solverBody1;
		//const PxcSolverBody& b13 = *constraintDescs[3].solverBody1;

		const PxcSolverBodyData& bd00 = *constraintDescs[0].sBodyData0;
		const PxcSolverBodyData& bd01 = *constraintDescs[1].sBodyData0;
		const PxcSolverBodyData& bd02 = *constraintDescs[2].sBodyData0;
		const PxcSolverBodyData& bd03 = *constraintDescs[3].sBodyData0;

		const PxcSolverBodyData& bd10 = *constraintDescs[0].sBodyData1;
		const PxcSolverBodyData& bd11 = *constraintDescs[1].sBodyData1;
		const PxcSolverBodyData& bd12 = *constraintDescs[2].sBodyData1;
		const PxcSolverBodyData& bd13 = *constraintDescs[3].sBodyData1;

		//Load up masses, invInertia, velocity etc.

		const Vec4V invMassScale0 = V4Merge(FLoad(ims[0].linear0), FLoad(ims[1].linear0), FLoad(ims[2].linear0), FLoad(ims[3].linear0));
		const Vec4V invMassScale1 = V4Merge(FLoad(ims[0].linear1), FLoad(ims[1].linear1), FLoad(ims[2].linear1), FLoad(ims[3].linear1));


		const Vec4V invMass0 = V4Mul(V4Merge(FLoad(bd00.invMass), 
									   FLoad(bd01.invMass), 
									   FLoad(bd02.invMass), 
									   FLoad(bd03.invMass)), invMassScale0);

		const Vec4V invMass1 = V4Neg(V4Mul(V4Merge(FLoad(bd10.invMass), 
											 FLoad(bd11.invMass), 
											 FLoad(bd12.invMass), 
											 FLoad(bd13.invMass)), invMassScale1));

		const Vec4V invInertiaScale0 = V4Merge(FLoad(ims[0].angular0), FLoad(ims[1].angular0), FLoad(ims[2].angular0), FLoad(ims[3].angular0));
		const Vec4V invInertiaScale1 = V4Merge(FLoad(ims[0].angular1), FLoad(ims[1].angular1), FLoad(ims[2].angular1), FLoad(ims[3].angular1));

		Vec4V invInertia00X = Vec4V_From_Vec3V(V3LoadU(bd00.invInertia.column0));
		Vec4V invInertia00Y = Vec4V_From_Vec3V(V3LoadU(bd00.invInertia.column1));
		Vec4V invInertia00Z = Vec4V_From_Vec3V(V3LoadU(bd00.invInertia.column2));

		Vec4V invInertia10X = Vec4V_From_Vec3V(V3LoadU(bd01.invInertia.column0));
		Vec4V invInertia10Y = Vec4V_From_Vec3V(V3LoadU(bd01.invInertia.column1));
		Vec4V invInertia10Z = Vec4V_From_Vec3V(V3LoadU(bd01.invInertia.column2));

		Vec4V invInertia20X = Vec4V_From_Vec3V(V3LoadU(bd02.invInertia.column0));
		Vec4V invInertia20Y = Vec4V_From_Vec3V(V3LoadU(bd02.invInertia.column1));
		Vec4V invInertia20Z = Vec4V_From_Vec3V(V3LoadU(bd02.invInertia.column2));

		Vec4V invInertia30X = Vec4V_From_Vec3V(V3LoadU(bd03.invInertia.column0));
		Vec4V invInertia30Y = Vec4V_From_Vec3V(V3LoadU(bd03.invInertia.column1));
		Vec4V invInertia30Z = Vec4V_From_Vec3V(V3LoadU(bd03.invInertia.column2));

		Vec4V invInertia01X = Vec4V_From_Vec3V(V3LoadU(bd10.invInertia.column0));
		Vec4V invInertia01Y = Vec4V_From_Vec3V(V3LoadU(bd10.invInertia.column1));
		Vec4V invInertia01Z = Vec4V_From_Vec3V(V3LoadU(bd10.invInertia.column2));

		Vec4V invInertia11X = Vec4V_From_Vec3V(V3LoadU(bd11.invInertia.column0));
		Vec4V invInertia11Y = Vec4V_From_Vec3V(V3LoadU(bd11.invInertia.column1));
		Vec4V invInertia11Z = Vec4V_From_Vec3V(V3LoadU(bd11.invInertia.column2));

		Vec4V invInertia21X = Vec4V_From_Vec3V(V3LoadU(bd12.invInertia.column0));
		Vec4V invInertia21Y = Vec4V_From_Vec3V(V3LoadU(bd12.invInertia.column1));
		Vec4V invInertia21Z = Vec4V_From_Vec3V(V3LoadU(bd12.invInertia.column2));

		Vec4V invInertia31X = Vec4V_From_Vec3V(V3LoadU(bd13.invInertia.column0));
		Vec4V invInertia31Y = Vec4V_From_Vec3V(V3LoadU(bd13.invInertia.column1));
		Vec4V invInertia31Z = Vec4V_From_Vec3V(V3LoadU(bd13.invInertia.column2));

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

		invInertia0X0 = V4Mul(invInertia0X0, invInertiaScale0);
		invInertia0Y0 = V4Mul(invInertia0Y0, invInertiaScale0);
		invInertia0Z0 = V4Mul(invInertia0Z0, invInertiaScale0);

		invInertia0X1 = V4Mul(invInertia0X1, invInertiaScale0);
		invInertia0Y1 = V4Mul(invInertia0Y1, invInertiaScale0);
		invInertia0Z1 = V4Mul(invInertia0Z1, invInertiaScale0);

		invInertia0X2 = V4Mul(invInertia0X2, invInertiaScale0);
		invInertia0Y2 = V4Mul(invInertia0Y2, invInertiaScale0);
		invInertia0Z2 = V4Mul(invInertia0Z2, invInertiaScale0);

		invInertia1X0 = V4Mul(invInertia1X0, invInertiaScale1);
		invInertia1Y0 = V4Mul(invInertia1Y0, invInertiaScale1);
		invInertia1Z0 = V4Mul(invInertia1Z0, invInertiaScale1);

		invInertia1X1 = V4Mul(invInertia1X1, invInertiaScale1);
		invInertia1Y1 = V4Mul(invInertia1Y1, invInertiaScale1);
		invInertia1Z1 = V4Mul(invInertia1Z1, invInertiaScale1);

		invInertia1X2 = V4Mul(invInertia1X2, invInertiaScale1);
		invInertia1Y2 = V4Mul(invInertia1Y2, invInertiaScale1);
		invInertia1Z2 = V4Mul(invInertia1Z2, invInertiaScale1);

		//body world offsets
		Vec4V workOffset0 = Vec4V_From_Vec3V(V3LoadU(body0WorldOffset[0]));
		Vec4V workOffset1 = Vec4V_From_Vec3V(V3LoadU(body0WorldOffset[1]));
		Vec4V workOffset2 = Vec4V_From_Vec3V(V3LoadU(body0WorldOffset[2]));
		Vec4V workOffset3 = Vec4V_From_Vec3V(V3LoadU(body0WorldOffset[3]));

		Vec4V workOffsetX, workOffsetY, workOffsetZ;

		PX_TRANSPOSE_44_34(workOffset0, workOffset1, workOffset2, workOffset3, workOffsetX, workOffsetY, workOffsetZ);

		const FloatV dtV = FLoad(dt);
		Vec4V linBreakForce = V4Merge(FLoad(constraintDescs[0].constraint->linBreakForce), FLoad(constraintDescs[1].constraint->linBreakForce),
			FLoad(constraintDescs[2].constraint->linBreakForce), FLoad(constraintDescs[3].constraint->linBreakForce));
		Vec4V angBreakForce = V4Merge(FLoad(constraintDescs[0].constraint->angBreakForce), FLoad(constraintDescs[1].constraint->angBreakForce),
			FLoad(constraintDescs[2].constraint->angBreakForce), FLoad(constraintDescs[3].constraint->angBreakForce));


		//OK, I think that's everything loaded in

		header->invMass0Dom0 = invMass0;
		header->invMass1Dom1 = invMass1;
		header->body0WorkOffsetX = workOffsetX;
		header->body0WorkOffsetY = workOffsetY;
		header->body0WorkOffsetZ = workOffsetZ;

		header->count = maxRows;
		header->type = PXS_SC_TYPE_BLOCK_1D;
		header->linBreakImpulse = V4Scale(linBreakForce, dtV);
		header->angBreakImpulse = V4Scale(angBreakForce, dtV);
		header->count0 = Ps::to8(constraintDescs[0].numRows);
		header->count1 = Ps::to8(constraintDescs[1].numRows);
		header->count2 = Ps::to8(constraintDescs[2].numRows);
		header->count3 = Ps::to8(constraintDescs[3].numRows);

		//Now we loop over the constraints and build the results...

		PxU32 index0 = 0;
		PxU32 endIndex0 = constraintDescs[0].numRows - 1;
		PxU32 index1 = constraintDescs[1].startRowIndex;
		PxU32 endIndex1 = index1 + constraintDescs[1].numRows - 1;
		PxU32 index2 = constraintDescs[2].startRowIndex;
		PxU32 endIndex2 = index2 + constraintDescs[2].numRows - 1;
		PxU32 index3 = constraintDescs[3].startRowIndex;
		PxU32 endIndex3 = index3 + constraintDescs[3].numRows - 1;

		const FloatV one = FOne();

		for(PxU32 a = 0; a < maxRows; ++a)
		{	
			PxcSolverConstraint1DDynamic4* c = (PxcSolverConstraint1DDynamic4*)currPtr;
			currPtr += stride;

			Px1DConstraint* con0 = allSorted[index0];
			Px1DConstraint* con1 = allSorted[index1];
			Px1DConstraint* con2 = allSorted[index2];
			Px1DConstraint* con3 = allSorted[index3];

			index0 = index0 == endIndex0 ? index0 : index0 + 1;
			index1 = index1 == endIndex1 ? index1 : index1 + 1;
			index2 = index2 == endIndex2 ? index2 : index2 + 1;
			index3 = index3 == endIndex3 ? index3 : index3 + 1;

			Vec4V driveScale = V4Splat(one);
			if(con0->flags&Px1DConstraintFlag::eHAS_DRIVE_LIMIT && constraintDescs[0].constraint->flags&PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES)
				driveScale = V4SetX(driveScale, FMin(one, dtV));
			if(con1->flags&Px1DConstraintFlag::eHAS_DRIVE_LIMIT && constraintDescs[1].constraint->flags&PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES)
				driveScale = V4SetY(driveScale, FMin(one, dtV));
			if(con2->flags&Px1DConstraintFlag::eHAS_DRIVE_LIMIT && constraintDescs[2].constraint->flags&PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES)
				driveScale = V4SetZ(driveScale, FMin(one, dtV));
			if(con3->flags&Px1DConstraintFlag::eHAS_DRIVE_LIMIT && constraintDescs[3].constraint->flags&PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES)
				driveScale = V4SetW(driveScale, FMin(one, dtV));


			Vec4V clin00 = V4LoadA(&con0->linear0.x);
			Vec4V clin01 = V4LoadA(&con1->linear0.x);
			Vec4V clin02 = V4LoadA(&con2->linear0.x);
			Vec4V clin03 = V4LoadA(&con3->linear0.x);

			Vec4V cang00 = V4LoadA(&con0->angular0.x);
			Vec4V cang01 = V4LoadA(&con1->angular0.x);
			Vec4V cang02 = V4LoadA(&con2->angular0.x);
			Vec4V cang03 = V4LoadA(&con3->angular0.x);

			Vec4V clin0X, clin0Y, clin0Z;
			Vec4V cang0X, cang0Y, cang0Z;
			
			PX_TRANSPOSE_44_34(clin00, clin01, clin02, clin03, clin0X, clin0Y, clin0Z);
			PX_TRANSPOSE_44_34(cang00, cang01, cang02, cang03, cang0X, cang0Y, cang0Z);
			
			const Vec4V maxImpulse = V4Merge(FLoad(con0->maxImpulse), FLoad(con1->maxImpulse), FLoad(con2->maxImpulse), 
				FLoad(con3->maxImpulse));
			const Vec4V minImpulse = V4Merge(FLoad(con0->minImpulse), FLoad(con1->minImpulse), FLoad(con2->minImpulse), 
				FLoad(con3->minImpulse));

			c->flags[0] = 0;
			c->flags[1] = 0;
			c->flags[2] = 0;
			c->flags[3] = 0;

			c->lin0X = clin0X;
			c->lin0Y = clin0Y;
			c->lin0Z = clin0Z;
			c->ang0X = cang0X;
			c->ang0Y = cang0Y;
			c->ang0Z = cang0Z;

			c->minImpulse = V4Mul(minImpulse, driveScale);
			c->maxImpulse = V4Mul(maxImpulse, driveScale);
			c->appliedForce = zero;

			const Vec4V angDelta0X = V4MulAdd(cang0Z, invInertia0X2, V4MulAdd(cang0Y, invInertia0X1, V4Mul(cang0X, invInertia0X0)));
			const Vec4V angDelta0Y = V4MulAdd(cang0Z, invInertia0Y2, V4MulAdd(cang0Y, invInertia0Y1, V4Mul(cang0X, invInertia0Y0)));
			const Vec4V angDelta0Z = V4MulAdd(cang0Z, invInertia0Z2, V4MulAdd(cang0Y, invInertia0Z1, V4Mul(cang0X, invInertia0Z0)));

			const Vec4V lin0MagSq = V4MulAdd(clin0Z, clin0Z, V4MulAdd(clin0Y, clin0Y, V4Mul(clin0X, clin0X)));
			const Vec4V cang0DotAngDelta = V4MulAdd(cang0Z, angDelta0Z, V4MulAdd(cang0Y, angDelta0Y, V4Mul(cang0X, angDelta0X)));

			c->ang0InvInertiaX = angDelta0X;
			c->ang0InvInertiaY = angDelta0Y;
			c->ang0InvInertiaZ = angDelta0Z;
			c->flags[0] = 0;
			c->flags[1] = 0;
			c->flags[2] = 0;
			c->flags[3] = 0;

			Vec4V unitResponse = V4MulAdd(lin0MagSq, invMass0, cang0DotAngDelta);

			//if(!isStatic)
			//{

				Vec4V clin10 = V4LoadA(&con0->linear1.x);
				Vec4V clin11 = V4LoadA(&con1->linear1.x);
				Vec4V clin12 = V4LoadA(&con2->linear1.x);
				Vec4V clin13 = V4LoadA(&con3->linear1.x);

				Vec4V cang10 = V4LoadA(&con0->angular1.x);
				Vec4V cang11 = V4LoadA(&con1->angular1.x);
				Vec4V cang12 = V4LoadA(&con2->angular1.x);
				Vec4V cang13 = V4LoadA(&con3->angular1.x);

				Vec4V clin1X, clin1Y, clin1Z;
				Vec4V cang1X, cang1Y, cang1Z;
				PX_TRANSPOSE_44_34(clin10, clin11, clin12, clin13, clin1X, clin1Y, clin1Z);
				PX_TRANSPOSE_44_34(cang10, cang11, cang12, cang13, cang1X, cang1Y, cang1Z);

				const Vec4V angDelta1X = V4MulAdd(cang1Z, invInertia1X2, V4MulAdd(cang1Y, invInertia1X1, V4Mul(cang1X, invInertia1X0)));
				const Vec4V angDelta1Y = V4MulAdd(cang1Z, invInertia1Y2, V4MulAdd(cang1Y, invInertia1Y1, V4Mul(cang1X, invInertia1Y0)));
				const Vec4V angDelta1Z = V4MulAdd(cang1Z, invInertia1Z2, V4MulAdd(cang1Y, invInertia1Z1, V4Mul(cang1X, invInertia1Z0)));

				const Vec4V lin1MagSq = V4MulAdd(clin1Z, clin1Z, V4MulAdd(clin1Y, clin1Y, V4Mul(clin1X, clin1X)));
				const Vec4V cang1DotAngDelta = V4MulAdd(cang1Z, angDelta1Z, V4MulAdd(cang1Y, angDelta1Y, V4Mul(cang1X, angDelta1X)));

				c->lin1X = clin1X;
				c->lin1Y = clin1Y;
				c->lin1Z = clin1Z;

				c->ang1X = cang1X;
				c->ang1Y = cang1Y;
				c->ang1Z = cang1Z;

				c->ang1InvInertia1X = V4Neg(angDelta1X);
				c->ang1InvInertia1Y = V4Neg(angDelta1Y);
				c->ang1InvInertia1Z = V4Neg(angDelta1Z);

				unitResponse = V4Add(unitResponse, V4NegMulSub(lin1MagSq, invMass1, cang1DotAngDelta));
			//}

			{
				const PxVec4& ur				= (const PxVec4&)unitResponse;
				PxVec4& cConstant				= (PxVec4&)c->constant;
				PxVec4& cUnbiasedConstant		= (PxVec4&)c->unbiasedConstant;
				PxVec4& cVelMultiplier			= (PxVec4&)c->velMultiplier;
				PxVec4& cImpulseMultiplier		= (PxVec4&)c->impulseMultiplier;

				setConstants(cConstant.x, cUnbiasedConstant.x, cVelMultiplier.x, cImpulseMultiplier.x, 
							 *con0, ur.x, erp[0], dt, recipdt, 
							 *constraintDescs[0].solverBody0, *constraintDescs[0].solverBody1, a >= constraintDescs[0].numRows);

				setConstants(cConstant.y, cUnbiasedConstant.y, cVelMultiplier.y, cImpulseMultiplier.y, 
							 *con1, ur.y, erp[1], dt, recipdt, 
							 *constraintDescs[1].solverBody0, *constraintDescs[1].solverBody1, a >= constraintDescs[1].numRows);
				
				setConstants(cConstant.z, cUnbiasedConstant.z, cVelMultiplier.z, cImpulseMultiplier.z, 
							 *con2, ur.z, erp[2], dt, recipdt, 
							 *constraintDescs[2].solverBody0, *constraintDescs[2].solverBody1, a >= constraintDescs[2].numRows);

				setConstants(cConstant.w, cUnbiasedConstant.w, cVelMultiplier.w, cImpulseMultiplier.w, 
							 *con3, ur.w, erp[3], dt, recipdt, 
							 *constraintDescs[3].solverBody0, *constraintDescs[3].solverBody1, a >= constraintDescs[3].numRows);
			}

			if(con0->flags & Px1DConstraintFlag::eOUTPUT_FORCE)
				c->flags[0] |= PXS_SC_FLAG_OUTPUT_FORCE;
			if(con1->flags & Px1DConstraintFlag::eOUTPUT_FORCE)
				c->flags[1] |= PXS_SC_FLAG_OUTPUT_FORCE;
			if(con2->flags & Px1DConstraintFlag::eOUTPUT_FORCE)
				c->flags[2] |= PXS_SC_FLAG_OUTPUT_FORCE;
			if(con3->flags & Px1DConstraintFlag::eOUTPUT_FORCE)
				c->flags[3] |= PXS_SC_FLAG_OUTPUT_FORCE;
		}
		*((PxU32*)currPtr) = 0;
		*((PxU32*)(currPtr + 4)) = 0;
	}
	
	//OK, we're ready to allocate and solve prep these constraints now :-)
	return PxcSolverConstraintPrepState::eSUCCESS;
}

}
