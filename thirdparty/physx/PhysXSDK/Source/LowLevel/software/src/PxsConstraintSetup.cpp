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
#include "PxMathUtils.h"

#ifdef __SPU__
#include "../include/spu/SpuNpMemBlock.h"
#define FORCE_INLINE 
#else
#define FORCE_INLINE PX_FORCE_INLINE
#endif

using namespace physx;


namespace physx
{
	// dsequeira:
	//
	// we can choose any linear combination of equality constraints and get the same solution
	// Hence we can orthogonalize the constraints using the inner product given by the
	// inverse mass matrix, so that when we use PGS, solving a constraint row for a joint 
	// don't disturb the solution of prior rows.
	//
	// We also eliminate the equality constraints from the hard inequality constraints - 
	// (essentially projecting the direction corresponding to the lagrange multiplier 
	// onto the equality constraint subspace) but 'til I've verified this generates 
	// exactly the same KKT/complementarity conditions, status is 'experimental'. 	
	//
	// since for equality constraints the resulting rows have the property that applying
	// an impulse along one row doesn't alter the projected velocity along another row, 
	// all equality constraints (plus one inequality constraint) can be processed in parallel
	// using SIMD
	//
	// Eliminating the inequality constraints from each other would require a solver change
	// and not give us any more parallelism, although we might get better convergence.

namespace
{
	PX_FORCE_INLINE Vec3V V3FromV4(Vec4V x)			{ return Vec3V_From_Vec4V(x); }
	PX_FORCE_INLINE Vec3V V3FromV4Unsafe(Vec4V x)	{ return Vec3V_From_Vec4V_WUndefined(x); }
	PX_FORCE_INLINE Vec4V V4FromV3(Vec3V x)			{ return Vec4V_From_Vec3V(x); }
	//PX_FORCE_INLINE Vec4V V4ClearW(Vec4V x)			{ return V4SetW(x, FZero()); }

struct MassProps
{
	Mat33V invInertia0;
	Mat33V invInertia1;
	FloatV invMass0;
	FloatV invMass1;

	FORCE_INLINE MassProps(const PxcSolverBodyData& bd0,
							  const PxcSolverBodyData& bd1,
							  const PxConstraintInvMassScale& ims)
	:	invInertia0(M33Scale(Mat33V_From_PxMat33(bd0.invInertia), FLoad(ims.angular0)))
	,	invInertia1(M33Scale(Mat33V_From_PxMat33(bd1.invInertia), FLoad(ims.angular1)))
	,	invMass0(FLoad(bd0.invMass * ims.linear0))
	,	invMass1(FLoad(bd1.invMass * ims.linear1))
	{}
};


FORCE_INLINE PxReal innerProduct(const Px1DConstraint& row0, Px1DConstraint& row1, const MassProps& m)
{
	FloatV l0 = V3Dot(V3Scale(V3LoadA(row0.linear0), m.invMass0), V3LoadA(row1.linear0));
	FloatV l1 = V3Dot(V3Scale(V3LoadA(row0.linear1), m.invMass1), V3LoadA(row1.linear1));
	FloatV a0 = V3Dot(M33MulV3(m.invInertia0, V3LoadA(row0.angular0)), V3LoadA(row1.angular0));
	FloatV a1 = V3Dot(M33MulV3(m.invInertia1, V3LoadA(row0.angular1)), V3LoadA(row1.angular1));
	return FStore(FAdd(FAdd(l0, l1), FAdd(a0, a1)));
}



// indexed rotation around axis, with sine and cosine of half-angle
PX_FORCE_INLINE PxQuat indexedRotation(PxU32 axis, PxReal s, PxReal c)
{
	PxQuat q(0,0,0,c);
	reinterpret_cast<PxReal*>(&q)[axis] = s;
	return q;
}

PxQuat diagonalize(const PxMat33& m)	// jacobi rotation using quaternions 
{
	const PxU32 MAX_ITERS = 5;

	PxQuat q = PxQuat(PxIdentity);

	PxMat33 d;
	for(PxU32 i=0; i < MAX_ITERS;i++)
	{
		PxMat33 axes(q);
		d = axes.getTranspose() * m * axes;

		PxReal d0 = PxAbs(d[1][2]), d1 = PxAbs(d[0][2]), d2 = PxAbs(d[0][1]);
		PxU32 a = PxU32(d0 > d1 && d0 > d2 ? 0 : d1 > d2 ? 1 : 2);						// rotation axis index, from largest off-diagonal element

		PxU32 a1 = Ps::getNextIndex3(a), a2 = Ps::getNextIndex3(a1);											
		if(d[a1][a2] == 0.0f || PxAbs(d[a1][a1]-d[a2][a2]) > 2e6*PxAbs(2.0f*d[a1][a2]))
			break;

		PxReal w = (d[a1][a1]-d[a2][a2]) / (2.0f*d[a1][a2]);					// cot(2 * phi), where phi is the rotation angle
		PxReal absw = PxAbs(w);

		PxQuat r;
		if(absw>1000)
			r = indexedRotation(a, 1/(4*w), 1.f);									// h will be very close to 1, so use small angle approx instead
		else
		{
  			PxReal t = 1 / (absw + PxSqrt(w*w+1));								// absolute value of tan phi
			PxReal h = 1 / PxSqrt(t*t+1);										// absolute value of cos phi

			PX_ASSERT(h!=1);													// |w|<1000 guarantees this with typical IEEE754 machine eps (approx 6e-8)
			r = indexedRotation(a, PxSqrt((1-h)/2) * PxSign(w), PxSqrt((1+h)/2));
		}
	
		q = (q*r).getNormalized();
	}

	return q;
}


template<typename T>
FORCE_INLINE void rescale(const PxMat33& m, T& a0, T& a1, T& a2)
{
	T b0 = a0*m(0,0) + a1 * m(1,0) + a2 * m(2,0);
	T b1 = a0*m(0,1) + a1 * m(1,1) + a2 * m(2,1);
	T b2 = a0*m(0,2) + a1 * m(1,2) + a2 * m(2,2);

	a0 = b0;
	a1 = b1;
	a2 = b2;
}

void diagonalize(Px1DConstraint** row,
				 const MassProps &m)
{
	PxReal a00 = innerProduct(*row[0], *row[0], m);
	PxReal a01 = innerProduct(*row[0], *row[1], m);
	PxReal a02 = innerProduct(*row[0], *row[2], m);
	PxReal a11 = innerProduct(*row[1], *row[1], m);
	PxReal a12 = innerProduct(*row[1], *row[2], m);
	PxReal a22 = innerProduct(*row[2], *row[2], m);

	PxMat33 a(PxVec3(a00, a01, a02),
			  PxVec3(a01, a11, a12),
			  PxVec3(a02, a12, a22));

	PxQuat q = diagonalize(a);

	PxMat33 n(-q);

	rescale(n, row[0]->linear0, row[1]->linear0, row[2]->linear0);
	rescale(n, row[0]->linear1, row[1]->linear1, row[2]->linear1);
	rescale(n, row[0]->angular0, row[1]->angular0, row[2]->angular0);
	rescale(n, row[0]->angular1, row[1]->angular1, row[2]->angular1);
	rescale(n, row[0]->velocityTarget, row[1]->velocityTarget, row[2]->velocityTarget);
	rescale(n, row[0]->geometricError, row[1]->geometricError, row[2]->geometricError);
}


void orthogonalize(Px1DConstraint** row,
				   PxU32 rowCount,
				   PxU32 eqRowCount,
				   const MassProps &m)
{
	PX_ASSERT(eqRowCount<=6);

	Vec3V lin1m[6], ang1m[6], lin1[6], ang1[6];	
	Vec4V lin0m[6], ang0m[6];			// must have 0 in the W-field
	Vec4V lin0AndG[6], ang0AndT[6];

	for(PxU32 i=0;i<rowCount;i++)
	{
		// set the geometric error for the velocity solve
		row[i]->forInternalUse = row[i]->flags & Px1DConstraintFlag::eKEEPBIAS ? row[i]->geometricError : 0;

		Vec4V l0AndG = V4LoadA(&row[i]->linear0.x);		// linear0 and geometric error
		Vec4V a0AndT = V4LoadA(&row[i]->angular0.x);	// angular0 and velocity target

		Vec3V l1 = V3FromV4(V4LoadA(&row[i]->linear1.x));
		Vec3V a1 = V3FromV4(V4LoadA(&row[i]->angular1.x));

		PxU32 eliminationRows = PxMin<PxU32>(i, eqRowCount);
		for(PxU32 j=0;j<eliminationRows;j++)
		{
			const Vec3V s0 = V3MulAdd(l1, lin1m[j], V3FromV4Unsafe(V4Mul(l0AndG, lin0m[j])));
			const Vec3V s1 = V3MulAdd(a1, ang1m[j], V3FromV4Unsafe(V4Mul(a0AndT, ang0m[j])));
			FloatV t = V3SumElems(V3Add(s0, s1));

			l0AndG = V4NegScaleSub(lin0AndG[j], t, l0AndG);
			a0AndT = V4NegScaleSub(ang0AndT[j], t, a0AndT);
			l1 = V3NegScaleSub(lin1[j], t, l1);
			a1 = V3NegScaleSub(ang1[j], t, a1);
		}

		V4StoreA(l0AndG, &row[i]->linear0.x);
		V4StoreA(a0AndT, &row[i]->angular0.x);
		V3StoreA(l1, row[i]->linear1);
		V3StoreA(a1, row[i]->angular1);

		if(i<eqRowCount)
		{
			lin0AndG[i] = l0AndG;	
			ang0AndT[i] = a0AndT;
			lin1[i] = l1;	
			ang1[i] = a1;		
			
			const Vec3V l0 = V3FromV4(l0AndG);
			const Vec3V a0 = V3FromV4(a0AndT);

			const Vec3V l0m = V3Scale(l0, m.invMass0);
			const Vec3V l1m = V3Scale(l1, m.invMass1);
			const Vec3V a0m = M33MulV3(m.invInertia0, a0);
			const Vec3V a1m = M33MulV3(m.invInertia1, a1);

			const Vec3V s0 = V3MulAdd(l0, l0m, V3Mul(l1, l1m));
			const Vec3V s1 = V3MulAdd(a0, a0m, V3Mul(a1, a1m));
			const FloatV s = V3SumElems(V3Add(s0, s1));
			const FloatV a = FSel(FIsGrtr(s, FZero()), FRecip(s), FZero());	// with mass scaling, it's possible for the inner product of a row to be zero

			lin0m[i] = V4Scale(V4ClearW(V4FromV3(l0m)), a);	
			ang0m[i] = V4Scale(V4ClearW(V4FromV3(a0m)), a);
			lin1m[i] = V3Scale(l1m, a);
			ang1m[i] = V3Scale(a1m, a);
		}
	}
}
}

void preprocessRows(Px1DConstraint** sorted, 
					Px1DConstraint* rows,
					PxU32 rowCount,
					const PxcSolverBodyData& bd0,
					const PxcSolverBodyData& bd1,
					const PxConstraintInvMassScale& ims,
					bool isExtended,
					bool diagonalizeDrive)
{
	// j is maxed at 12, typically around 7, so insertion sort is fine
	for(PxU32 i=0; i<rowCount; i++)
	{
		Px1DConstraint* r = rows+i;
		
		PxU32 j = i;
		for(;j>0 && r->solveHint < sorted[j-1]->solveHint; j--)
			sorted[j] = sorted[j-1];

		sorted[j] = r;
	}

	for(PxU32 i=0;i<rowCount-1;i++)
		PX_ASSERT(sorted[i]->solveHint <= sorted[i+1]->solveHint);

	if(isExtended)
		return;

	MassProps m(bd0, bd1, ims);
	for(PxU32 i=0;i<rowCount;)
	{
		const PxU32 groupMajorId = PxU32(sorted[i]->solveHint>>8), start = i++;
		while(i<rowCount && PxU32(sorted[i]->solveHint>>8) == groupMajorId)
			i++;

		if(groupMajorId == 4)
		{
			PxU32 bCount = start;		// count of bilateral constraints 
			for(; bCount<i && (sorted[bCount]->solveHint&255)==0; bCount++)
				;
			orthogonalize(sorted+start,i-start, bCount-start, m);
		}

		if(groupMajorId == 1 && diagonalizeDrive)
		{			
			PxU32 slerp = start;		// count of bilateral constraints 
			for(; slerp<i && (sorted[slerp]->solveHint&255)!=2; slerp++)
				;
			if(slerp+3 == i)
				diagonalize(sorted+slerp, m);

			PX_ASSERT(i-start==3);
			diagonalize(sorted+start, m);
		}
	}
}





PxU32 PxsConstraintHelper::setupSolverConstraint
(const PxTransform& pose0, const PxTransform& pose1, const PxcSolverBody* sBody0, const PxcSolverBody* sBody1,
 PxcSolverBodyData* sBodyData0, PxcSolverBodyData* sBodyData1,
 const PxConstraintSolverPrep solverPrep, const void* constantBlock, const PxU32 /*constantBlockByteSize*/,
 const PxReal dt, const PxReal recipdt, 
 PxcSolverConstraintDesc& desc, const PxsConstraint& constraint,
#ifdef __SPU__
 SpuNpMemBlock& blockStream, uintptr_t* eaConstraintData)
#else
 PxcConstraintBlockStream& blockStream, PxsConstraintBlockManager& constraintBlockManager)
#endif
{
	setConstraintLength(desc,0);

	if (!solverPrep)
		return 0;

	//PxU32 numAxisConstraints = 0;

	Px1DConstraint rows[PxsConstraint::MAX_CONSTRAINTS];

	// This is necessary so that there will be sensible defaults and shaders will
	// continue to work (albeit with a recompile) if the row format changes.
	// It's a bit inefficient because it fills in all constraint rows even if there
	// is only going to be one generated. A way around this would be for the shader to
	// specify the maximum number of rows it needs, or it could call a subroutine to
	// prep the row before it starts filling it it. Not sure how workable subroutines
	// for PS3 SPU shaders?

	for(PxU32 i=0;i<PxsConstraint::MAX_CONSTRAINTS;i++)
	{
		Px1DConstraint& c = rows[i];
		Px1DConstraintInit(c);
	}
	PxConstraintInvMassScale ims = {1,1,1,1};
   
	PxVec3 body0WorldOffset(0.f);
	PxU32 constraintCount = (*solverPrep)(rows,
		body0WorldOffset,
		PxsConstraint::MAX_CONSTRAINTS,
		ims,
		constantBlock, 
		pose0, pose1);

	if(constraintCount==0)
		return 0;

	bool isExtended = desc.linkIndexA != PxcSolverConstraintDesc::NO_LINK
				   || desc.linkIndexB != PxcSolverConstraintDesc::NO_LINK;

	PxU32 stride = isExtended ? sizeof(PxcSolverConstraint1DExt) : sizeof(PxcSolverConstraint1D);
	const PxU32 constraintLength = sizeof(PxcSolverConstraint1DHeader) + stride * constraintCount;
	
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
	desc.constraint = ptr;
	*eaConstraintData = ea;
#else
	//KS - +16 is for the constraint progress counter, which needs to be the last element in the constraint (so that we
	//know SPU DMAs have completed)
	PxU8* ptr = blockStream.reserve(constraintLength + 16u, constraintBlockManager);
	if(NULL == ptr || ((PxU8*)(-1))==ptr)
	{
		if(NULL==ptr)
		{
			PX_WARN_ONCE(true,
				"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for constraint prep. "
				"Either accept joints detaching/exploding or increase buffer size allocated for constraint prep by increasing PxSceneDesc::maxNbContactDataBlocks.");
			return 0;
		}
		else
		{
			PX_WARN_ONCE(true,
				"Attempting to allocate more than 16K of constraint data. "
				"Either accept joints detaching/exploding or simplify constraints.");
			ptr=NULL;
			return 0;
		}
	}
	desc.constraint = ptr;
#endif
	setConstraintLength(desc,constraintLength);

	desc.writeBack = constraint.writeback;
	setWritebackLength(desc, sizeof(PxsConstraintWriteback));

	memset(desc.constraint, 0, constraintLength);

	PxcSolverConstraint1DHeader* header = reinterpret_cast<PxcSolverConstraint1DHeader*>(desc.constraint);
	PxU8* constraints = desc.constraint + sizeof(PxcSolverConstraint1DHeader);
	init(*header, Ps::to8(constraintCount), isExtended, ims);
	header->body0WorldOffset = body0WorldOffset;
	header->linBreakImpulse = constraint.linBreakForce * dt;
	header->angBreakImpulse = constraint.angBreakForce * dt;


	Px1DConstraint* sorted[PxsConstraint::MAX_CONSTRAINTS];
	preprocessRows(sorted, rows, constraintCount, *sBodyData0, *sBodyData1, ims, isExtended,
		(constraint.flags & PxConstraintFlag::eIMPROVED_SLERP)!=0);

	PxReal erp = constraint.flags & PxConstraintFlag::eDEPRECATED_32_COMPATIBILITY ? 0.7f : 1.0f;
	for(PxU32 i=0;i<constraintCount;i++)
	{
		Ps::prefetchLine(constraints, 128);
		PxcSolverConstraint1D &s = *reinterpret_cast<PxcSolverConstraint1D *>(constraints);
		Px1DConstraint& c = *sorted[i];

		PxReal driveScale = c.flags&Px1DConstraintFlag::eHAS_DRIVE_LIMIT && constraint.flags&PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES ? PxMin(dt, 1.0f) : 1.0f;
		init(s, c.linear0, c.linear1, c.angular0, c.angular1, c.minImpulse * driveScale, c.maxImpulse * driveScale);

		PxReal unitResponse;
		PxReal normalVel = 0.0f;

		if(!isExtended)
		{
			unitResponse = sBody0->getResponse(c.linear0, c.angular0, s.ang0InvInertia, *sBodyData0, ims.linear0, ims.angular0) 
				         + sBody1->getResponse(-c.linear1, -c.angular1, s.ang1InvInertia, *sBodyData1, ims.linear1, ims.angular1);
			s.invMass0 = sBodyData0->invMass * ims.linear0;
			s.invMass1 = sBodyData1->invMass * -ims.linear1;
			if(needsNormalVel(c))
				normalVel = sBody0->projectVelocity(s.lin0, s.ang0) - sBody1->projectVelocity(s.lin1, s.ang1);
		}
		else
		{
			PxcSolverConstraint1DExt& e = static_cast<PxcSolverConstraint1DExt&>(s);

			PxsSolverExtBody eb0((void*)sBody0, sBodyData0, desc.linkIndexA);
			PxsSolverExtBody eb1((void*)sBody1, sBodyData1, desc.linkIndexB);

			unitResponse = getImpulseResponse(eb0, Cm::SpatialVector(e.lin0, e.ang0), unsimdRef(e.deltaVA), ims.linear0, ims.angular0, 
												  eb1, Cm::SpatialVector(-e.lin1, -e.ang1), unsimdRef(e.deltaVB), ims.linear1, ims.angular1, true);
			if(needsNormalVel(c))
				normalVel = eb0.projectVelocity(s.lin0, s.ang0) - eb1.projectVelocity(s.lin1, s.ang1);
		}

		setSolverConstants(s.constant, s.unbiasedConstant, s.velMultiplier, s.impulseMultiplier, 
						   c, normalVel, unitResponse, erp, dt, recipdt);

		if(c.flags & Px1DConstraintFlag::eOUTPUT_FORCE)
			s.flags |= PXS_SC_FLAG_OUTPUT_FORCE;

		constraints += stride;
	}

	//KS - Set the solve count at the end to 0 
	*((PxU32*)constraints) = 0;
	*((PxU32*)(constraints + 4)) = 0;
	PX_ASSERT(desc.constraint + getConstraintLength(desc) == constraints);
	return constraintCount;
}

}
