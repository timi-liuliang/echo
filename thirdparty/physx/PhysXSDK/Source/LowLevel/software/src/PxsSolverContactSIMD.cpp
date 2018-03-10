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

#ifdef	__SPU__
#include "CmPS3CellUtil.h"
#endif

using namespace physx;
using namespace Gu;

void physx::setupFinalizeSolverConstraints(PxcNpWorkUnit& n,
						    const ContactBuffer& buffer,
							const PxcCorrelationBuffer& c,
							const PxTransform& bodyFrame0,
							const PxTransform& bodyFrame1,
							bool /*perPointFriction*/,
							PxU8* workspace,
							const PxcSolverBody& b0,
							const PxcSolverBody& b1,
							const PxcSolverBodyData& data0,
							const PxcSolverBodyData& data1,
							const PxReal invDtF32,
							PxReal bounceThresholdF32,
							PxReal invMassScale0, PxReal invInertiaScale0, 
							PxReal invMassScale1, PxReal invInertiaScale1)	
{
	// NOTE II: the friction patches are sparse (some of them have no contact patches, and
	// therefore did not get written back to the cache) but the patch addresses are dense,
	// corresponding to valid patches

	if(buffer.count == 0)
		return;

	PxU8 flags = PxU8(n.flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0);


	PX_ASSERT(n.frictionDataPtr);

	/*const bool haveFriction = PX_IR(n.staticFriction) > 0 || PX_IR(n.dynamicFriction) > 0;*/

	PxU8* PX_RESTRICT ptr = workspace;

	const FloatV zero=FZero();

	const FloatV iMScale0 = FLoad(invMassScale0);
	const FloatV iMScale1 = FLoad(invMassScale1); 
	const FloatV iInertiaScale0 = FLoad(invInertiaScale0);
	const FloatV iInertiaScale1 = FLoad(invInertiaScale1);
	
	const FloatV _d0 = FLoad(n.dominance0);
	const FloatV _d1 = FLoad(n.dominance1);
	const FloatV d0 = FMul(_d0, iMScale0);
	const FloatV d1 = FMul(_d1, iMScale1);
	const FloatV angD0 = FMul(_d0, iInertiaScale0);
	const FloatV angD1 = FMul(_d1, iInertiaScale1);
	const FloatV nDom1fV = FNeg(d1);

	const FloatV invMass0 = FLoad(data0.invMass);
	const FloatV invMass1 = FLoad(data1.invMass);

	const FloatV invMass0_dom0fV = FMul(d0, invMass0);
	const FloatV invMass1_dom1fV = FMul(nDom1fV, invMass1);


	Vec4V staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W = V4Zero();
	staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, invMass0_dom0fV);
	staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, invMass1_dom1fV);

	const FloatV restDistance = FLoad(n.restDistance); 

	const FloatV maxPenBias = FMax(FLoad(data0.penBiasClamp), FLoad(data1.penBiasClamp));

	const QuatV bodyFrame0q = QuatVLoadU(&bodyFrame0.q.x);
	const Vec3V bodyFrame0p = V3LoadU(bodyFrame0.p);
	
	const QuatV bodyFrame1q = QuatVLoadU(&bodyFrame1.q.x);
	const Vec3V bodyFrame1p = V3LoadU(bodyFrame1.p);

	PxU32 frictionPatchWritebackAddrIndex = 0;
	PxU32 contactWritebackCount = 0;

	Ps::prefetchLine(c.contactID);
	Ps::prefetchLine(c.contactID, 128);

	const Vec3V linVel0 = V3LoadU(b0.linearVelocity);
	const Vec3V linVel1 = V3LoadU(b1.linearVelocity);
	const Vec3V angVel0 = V3LoadU(b0.angularVelocity);
	const Vec3V angVel1 = V3LoadU(b1.angularVelocity);


	

	// PT: the matrix is symmetric so we can read it as a PxMat33! Gets rid of 25000+ LHS.
	const PxMat33& invIn0 = reinterpret_cast<const PxMat33&>(data0.invInertia);
	PX_ALIGN(16, const Mat33V invInertia0)
	(
		V3LoadU(invIn0.column0),
		V3LoadU(invIn0.column1),
		V3LoadU(invIn0.column2)
	);
	const PxMat33& invIn1 = reinterpret_cast<const PxMat33&>(data1.invInertia);
	PX_ALIGN(16, const Mat33V invInertia1)
	(
		V3LoadU(invIn1.column0),
		V3LoadU(invIn1.column1),
		V3LoadU(invIn1.column2)
	);

	const FloatV invDt = FLoad(invDtF32);
	const FloatV p8 = FLoad(0.8f);
	const FloatV bounceThreshold = FLoad(bounceThresholdF32);

	const FloatV invDtp8 = FMul(invDt, p8);

	bool useExtContacts = (n.flags & (PxcNpWorkUnitFlag::eARTICULATION_BODY0|PxcNpWorkUnitFlag::eARTICULATION_BODY1))!=0;

	PX_ALIGN(16, const Mat33V invInertia0_dom0)
	(
		V3Scale(invInertia0.col0, angD0),
		V3Scale(invInertia0.col1, angD0),
		V3Scale(invInertia0.col2, angD0)
	);

	PX_ALIGN(16, const Mat33V invInertia1_dom1)
	(
		V3Scale(invInertia1.col0, angD1),
		V3Scale(invInertia1.col1, angD1),
		V3Scale(invInertia1.col2, angD1)
	);


	for(PxU32 i=0;i<c.frictionPatchCount;i++)
	{
		PxU32 contactCount = c.frictionPatchContactCounts[i];
		if(contactCount == 0)
			continue;

		const PxcFrictionPatch& frictionPatch = c.frictionPatches[i];
		PX_ASSERT(frictionPatch.anchorCount <= 2);

		PxU32 firstPatch = c.correlationListHeads[i];
		const Gu::ContactPoint* contactBase0 = buffer.contacts + c.contactPatches[firstPatch].start;

		const PxReal combinedRestitution = contactBase0->restitution;
		
		PxcSolverContactHeader* PX_RESTRICT header = reinterpret_cast<PxcSolverContactHeader*>(ptr);
		ptr += sizeof(PxcSolverContactHeader);		


		Ps::prefetchLine(ptr, 128);
		Ps::prefetchLine(ptr, 256);

		header->flags = flags;
		const FloatV restitution = FLoad(combinedRestitution);
	
		PxU32 pointStride = useExtContacts ? sizeof(PxcSolverContactPointExt) : sizeof(PxcSolverContactPoint);
		PxU32 frictionStride = useExtContacts ? sizeof(PxcSolverContactFrictionExt) : sizeof(PxcSolverContactFriction);

		const Vec3V normal = V3LoadA(buffer.contacts[c.contactPatches[c.correlationListHeads[i]].start].normal);
		const FloatV normalLenSq = V3LengthSq(normal);
		const VecCrossV norCross = V3PrepareCross(normal);
		const FloatV norVel0 = V3Dot(normal, linVel0);
		const FloatV norVel1 = V3Dot(normal, linVel1);

		const FloatV invMassNorLenSq0 = FMul(invMass0_dom0fV, normalLenSq);
		const FloatV invMassNorLenSq1 = FMul(invMass1_dom1fV, normalLenSq);
		
		for(PxU32 patch=c.correlationListHeads[i]; 
			patch!=PxcCorrelationBuffer::LIST_END; 
			patch = c.contactPatches[patch].next)
		{
			//Ps::prefetchLine(&c.contactID[i][0]);
			const PxU32 count = c.contactPatches[patch].count;
			const Gu::ContactPoint* contactBase = buffer.contacts + c.contactPatches[patch].start;
				
			PxU8* p = ptr;
			

			//KS: Can we do 4-at-a-time?
			for(PxU32 j=0;j<count;j++)
			{
				Ps::prefetchLine(p, 256);
				const Gu::ContactPoint& contact = contactBase[j];

				PxcSolverContactPoint* PX_RESTRICT solverContact = reinterpret_cast<PxcSolverContactPoint*>(p);
				p += pointStride;

				//const Vec3V targetVel = Vec3V_From_PxVec3_Aligned(contact.targetVel);
				
				const Vec3V point = V3LoadA(contact.point);
				const FloatV separation = FLoad(contact.separation);

				const FloatV cTargetVel = V3Dot(normal, V3LoadA(contact.targetVel));

				const Vec3V ra = V3Sub(point, bodyFrame0p);
				const Vec3V rb = V3Sub(point, bodyFrame1p);

				const Vec3V raXn = V3Cross(ra, norCross);
				const Vec3V rbXn = V3Cross(rb, norCross);
				

				const Vec3V v0a = V3Scale(invInertia0_dom0.col0, V3GetX(raXn));
				const Vec3V v0PlusV1a = V3ScaleAdd(invInertia0_dom0.col1, V3GetY(raXn), v0a);
				const Vec3V delAngVel0 = V3ScaleAdd(invInertia0_dom0.col2, V3GetZ(raXn), v0PlusV1a);

				const Vec3V v0b = V3Scale(invInertia1_dom1.col0, V3GetX(rbXn));
				const Vec3V v0PlusV1b = V3ScaleAdd(invInertia1_dom1.col1, V3GetY(rbXn), v0b);
				const Vec3V delAngVel1 = V3ScaleAdd(invInertia1_dom1.col2, V3GetZ(rbXn), v0PlusV1b);

				const FloatV resp0 = FAdd(invMassNorLenSq0, V3Dot(raXn, delAngVel0));
				const FloatV resp1 = FSub(V3Dot(rbXn, delAngVel1), invMassNorLenSq1);

				const FloatV unitResponse = FAdd(resp0, resp1);

				const FloatV vrel1 = FAdd(norVel0, V3Dot(raXn, angVel0));
				const FloatV vrel2 = FAdd(norVel1, V3Dot(rbXn, angVel1));
				const FloatV vrel = FSub(vrel1, vrel2);

				const FloatV velMultiplier = FSel(FIsGrtr(unitResponse, zero), FRecip(unitResponse), zero);

				const FloatV penetration = FSub(separation, restDistance);

				const FloatV penetrationInvDt = FMul(penetration, invDt);

				const FloatV penetrationInvDtPt8 = FMax(maxPenBias, FMul(penetration, invDtp8));

				FloatV scaledBias = FMul(velMultiplier, penetrationInvDtPt8);

				const BoolV isGreater2 = BAnd(BAnd(FIsGrtr(restitution, zero), FIsGrtr(bounceThreshold, vrel)), FIsGrtr(FNeg(vrel), penetrationInvDt));

				scaledBias = FSel(isGreater2, zero, scaledBias);

				//const FloatV targetRelVel = V3Dot(normal,targetVel);
				//const FloatV sumVRel(V3Add(targetRelVel, vrel));
				const FloatV sumVRel(vrel);

				const FloatV targetVelocity = FAdd(cTargetVel, FSel(isGreater2, FMul(FNeg(sumVRel), restitution), zero));

				solverContact->normalXYZ_velMultiplierW =V4SetW(Vec4V_From_Vec3V(normal), velMultiplier);

				solverContact->delAngVel0_targetVelocityW = V4SetW(Vec4V_From_Vec3V(delAngVel0), targetVelocity);
				solverContact->delAngVel1_maxImpulseW = V4SetW(Vec4V_From_Vec3V(V3Neg(delAngVel1)), FLoad(contact.maxImpulse));
				solverContact->raXnXYZ_appliedForceW = V4SetW(Vec4V_From_Vec3V(raXn), zero);
				solverContact->rbXnXYZ_scaledBiasW = V4SetW(Vec4V_From_Vec3V(rbXn), scaledBias);
			}

			ptr = p;
		}
		contactWritebackCount += contactCount;

		const PxReal staticFriction = contactBase0->staticFriction;
		const PxReal dynamicFriction = contactBase0->dynamicFriction;
		const PxU32 disableStrongFriction = contactBase0->internalFaceIndex1 & PxMaterialFlag::eDISABLE_FRICTION;
		staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, FLoad(staticFriction));
		staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, FLoad(dynamicFriction));

		const bool haveFriction = (disableStrongFriction == 0 && frictionPatch.anchorCount != 0) ;//PX_IR(n.staticFriction) > 0 || PX_IR(n.dynamicFriction) > 0;
		header->numNormalConstr		= Ps::to8(contactCount);
		header->numFrictionConstr	= Ps::to8(haveFriction ? frictionPatch.anchorCount*2 : 0);
	
		header->type				= Ps::to8(useExtContacts ? PXS_SC_TYPE_EXT_CONTACT 
			                                         : haveFriction == 0 ? PXS_SC_TYPE_NOFRICTION_RB_CONTACT 
													 : (n.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1) == 0 ? PXS_SC_TYPE_STATIC_CONTACT 
									                 : PXS_SC_TYPE_RB_CONTACT);

		header->staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W = staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W;
		FStore(angD0, &header->angDom0);
		FStore(angD1, &header->angDom1);

		if(haveFriction)
		{
			const Vec3V vrel = V3Sub(linVel0, linVel1);
			//const Vec3V normal = Vec3V_From_PxVec3_Aligned(buffer.contacts[c.contactPatches[c.correlationListHeads[i]].start].normal);

			const FloatV orthoThreshold = FLoad(0.70710678f);
			const FloatV p1 = FLoad(0.1f);
			// fallback: normal.cross((1,0,0)) or normal.cross((0,0,1))
			const FloatV normalX = V3GetX(normal);
			const FloatV normalY = V3GetY(normal);
			const FloatV normalZ = V3GetZ(normal);
			
			Vec3V t0Fallback1 = V3Merge(zero, FNeg(normalZ), normalY);
			Vec3V t0Fallback2 = V3Merge(FNeg(normalY), normalX, zero) ;
			Vec3V t0Fallback = V3Sel(FIsGrtr(orthoThreshold, FAbs(normalX)), t0Fallback1, t0Fallback2);

			Vec3V t0 = V3Sub(vrel, V3Scale(normal, V3Dot(normal, vrel)));
			t0 = V3Sel(FIsGrtr(V3LengthSq(t0), p1), t0, t0Fallback);
			t0 = V3Normalize(t0);

			const VecCrossV t0Cross = V3PrepareCross(t0);

			const Vec3V t1 = V3Cross(norCross, t0Cross);
			const VecCrossV t1Cross = V3PrepareCross(t1);

			
			// since we don't even have the body velocities we can't compute the tangent dirs, so 
			// the only thing we can do right now is to write the geometric information (which is the
			// same for both axis constraints of an anchor) We put ra in the raXn field, rb in the rbXn
			// field, and the error in the the normal field. See corresponding comments in
			// completeContactFriction()

			//We want to set the writeBack ptr to point to the broken flag of the friction patch.
			//On spu we have a slight problem here because the friction patch array is 
			//in local store rather than in main memory. The good news is that the address of the friction 
			//patch array in main memory is stored in the work unit. These two addresses will be equal 
			//except on spu where one is local store memory and the other is the effective address in main memory.
			//Using the value stored in the work unit guarantees that the main memory address is used on all platforms.
			PxU8* PX_RESTRICT writeback = n.frictionDataPtr + frictionPatchWritebackAddrIndex*sizeof(PxcFrictionPatch);

			for(PxU32 j = 0; j < frictionPatch.anchorCount; j++)
			{
				Ps::prefetchLine(ptr, 256);
				Ps::prefetchLine(ptr, 384);
				PxcSolverContactFriction* PX_RESTRICT f0 = reinterpret_cast<PxcSolverContactFriction*>(ptr);
				ptr += frictionStride;
				PxcSolverContactFriction* PX_RESTRICT f1 = reinterpret_cast<PxcSolverContactFriction*>(ptr);
				ptr += frictionStride;

				Vec3V body0Anchor = V3LoadU(frictionPatch.body0Anchors[j]);
				Vec3V body1Anchor = V3LoadU(frictionPatch.body1Anchors[j]);

				f0->frictionBrokenWritebackByte = writeback;
				f1->frictionBrokenWritebackByte = writeback;

				Vec3V ra = QuatRotate(bodyFrame0q, body0Anchor);
				Vec3V rb = QuatRotate(bodyFrame1q, body1Anchor);
				Vec3V error =V3Sub(V3Add(ra, bodyFrame0p), V3Add(rb, bodyFrame1p));

				{
					const Vec3V raXn = V3Cross(ra, t0Cross);
					const Vec3V rbXn = V3Cross(rb, t0Cross);

					const Vec3V delAngVel0 = M33MulV3(invInertia0_dom0, raXn);
					const Vec3V delAngVel1 = M33MulV3(invInertia1_dom1, rbXn);

					const FloatV resp0 = FAdd(invMass0_dom0fV, V3Dot(raXn, delAngVel0));
					const FloatV resp1 = FSub(V3Dot(rbXn, delAngVel1), invMass1_dom1fV);
					const FloatV resp = FAdd(resp0, resp1);

					const FloatV velMultiplier = FSel(FIsGrtr(resp, zero), FDiv(p8, resp), zero);

					PxU32 index = /*perPointFriction ? c.contactID[i][j] : */c.contactPatches[c.correlationListHeads[i]].start;
					const FloatV targetVel = V3Dot(V3LoadU(buffer.contacts[index].targetVel), t0);

					f0->normalXYZ_appliedForceW = V4SetW(t0, zero);
					f0->raXnXYZ_velMultiplierW = V4SetW(raXn, velMultiplier);
					f0->rbXnXYZ_biasW = V4SetW(rbXn, FMul(V3Dot(t0, error), invDt));
					f0->delAngVel0_BrokenW = V4SetW(Vec4V_From_Vec3V(delAngVel0), zero);
					f0->delAngVel1_TargetVelW = V4SetW(Vec4V_From_Vec3V(V3Neg(delAngVel1)), targetVel);
				}

				{

					const Vec3V raXn = V3Cross(ra, t1Cross);
					const Vec3V rbXn = V3Cross(rb, t1Cross);

					const Vec3V delAngVel0 = M33MulV3(invInertia0_dom0, raXn);
					const Vec3V delAngVel1 = M33MulV3(invInertia1_dom1, rbXn);

					const FloatV resp0 = FAdd(invMass0_dom0fV, V3Dot(raXn, delAngVel0));
					const FloatV resp1 = FSub(V3Dot(rbXn, delAngVel1), invMass1_dom1fV);
					const FloatV resp = FAdd(resp0, resp1);

					const FloatV velMultiplier = FSel(FIsGrtr(resp, zero), FDiv(p8, resp), zero);

					PxU32 index = /*perPointFriction ? c.contactID[i][j] : */c.contactPatches[c.correlationListHeads[i]].start;
					const FloatV targetVel = V3Dot(V3LoadU(buffer.contacts[index].targetVel), t1);

					f1->normalXYZ_appliedForceW = V4SetW(t1, zero);
					f1->raXnXYZ_velMultiplierW = V4SetW(raXn, velMultiplier);
					f1->rbXnXYZ_biasW = V4SetW(rbXn, FMul(V3Dot(t1, error), invDt));
					f1->delAngVel0_BrokenW = V4SetW(Vec4V_From_Vec3V(delAngVel0), zero);
					f1->delAngVel1_TargetVelW = V4SetW(Vec4V_From_Vec3V(V3Neg(delAngVel1)), targetVel);
				}
			}
		}

		frictionPatchWritebackAddrIndex++;
	}

	PX_ASSERT(PxU32(ptr - workspace) == n.solverConstraintSize);
}



#endif
