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
#include "PxcSolverContactPF.h"
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

bool physx::setupFinalizeSolverConstraintsCoulomb(PxcNpWorkUnit& n,
						    const ContactBuffer& buffer,
							const PxcCorrelationBufferCoulomb& c,
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
							PxU32 frictionPerPointCount,
							PxReal invMassScale0, PxReal invInertiaScale0, 
							PxReal invMassScale1, PxReal invInertiaScale1)	
{   
	PxU8* PX_RESTRICT ptr = workspace;
	const FloatV zero=FZero();

	PxU8 flags = PxU8(n.flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD ? PxcSolverContactHeader::eHAS_FORCE_THRESHOLDS : 0);

	//const Vec4V staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W = V4Merge(FloatV_From_F32(n.staticFriction), FloatV_From_F32(n.dynamicFriction), FloatV_From_F32(n.dominance0), FloatV_From_F32(n.dominance1));

	//const Vec3V scaledBiasX_targetVelocityY_restitutionZ = V3Merge(zero, zero, FloatV_From_F32(n.restitution));
	const FloatV restDistance = FLoad(n.restDistance); 

	const Vec3V bodyFrame0p = V3LoadU(bodyFrame0.p);
	const Vec3V bodyFrame1p = V3LoadU(bodyFrame1.p);

	Ps::prefetchLine(c.contactID);
	Ps::prefetchLine(c.contactID, 128);

	bool useExtContacts = (n.flags & (PxcNpWorkUnitFlag::eARTICULATION_BODY0|PxcNpWorkUnitFlag::eARTICULATION_BODY1))!=0;

	const PxU32 frictionPatchCount = c.frictionPatchCount;
	const bool staticBody = ((n.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1) == 0);

	const PxU32 pointStride = useExtContacts ? sizeof(PxcSolverContactExt) : sizeof(PxcSolverContact);
	const PxU32 frictionStride = useExtContacts ? sizeof(PxcSolverFrictionExt) : sizeof(PxcSolverFriction);
	const PxU8 pointHeaderType = Ps::to8(useExtContacts ? PXS_SC_TYPE_EXT_CONTACT : (staticBody ? PXS_SC_TYPE_STATIC_CONTACT : PXS_SC_TYPE_RB_CONTACT));
	const PxU8 frictionHeaderType = Ps::to8(useExtContacts ? PXS_SC_TYPE_EXT_FRICTION : (staticBody ? PXS_SC_TYPE_STATIC_FRICTION : PXS_SC_TYPE_FRICTION));


	const Vec3V linVel0 = V3LoadU(b0.linearVelocity);
	const Vec3V linVel1 = V3LoadU(b1.linearVelocity);
	const Vec3V angVel0 = V3LoadU(b0.angularVelocity);
	const Vec3V angVel1 = V3LoadU(b1.angularVelocity);


	const FloatV invMass0 = FLoad(data0.invMass);
	const FloatV invMass1 = FLoad(data1.invMass);

	const FloatV maxPenBias = FMax(FLoad(data0.penBiasClamp), FLoad(data1.penBiasClamp));

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
	const FloatV orthoThreshold = FLoad(0.70710678f);
	const FloatV eps = FLoad(0.00001f);

	const FloatV invDtp8 = FMul(invDt, p8);

	const FloatV iMScale0 = FLoad(invMassScale0);
	const FloatV iMScale1 = FLoad(invMassScale1); 
	const FloatV iInertiaScale0 = FLoad(invInertiaScale0);
	const FloatV iInertiaScale1 = FLoad(invInertiaScale1);

	const FloatV _d0 = FLoad(n.dominance0);
	const FloatV _d1 = FLoad(n.dominance1);
	const FloatV d0 = FMul(_d0, iMScale0);
	const FloatV d1 = FMul(_d1, iMScale1);
	const FloatV angD0 = FMul(_d0, iInertiaScale0);
	const FloatV angD1 = FNeg(FMul(_d1, iInertiaScale1));
	const FloatV nDom1fV = FNeg(d1);

	const FloatV invMass0_dom0fV = FMul(d0, invMass0);
	const FloatV invMass1_dom1fV = FMul(nDom1fV, invMass1);

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


	for(PxU32 i=0;i< frictionPatchCount;i++)
	{
		const PxU32 contactCount = c.frictionPatchContactCounts[i];
		if(contactCount == 0)
			continue;

		const PxcFrictionPatchCoulomb& frictionPatch = c.frictionPatches[i];

		const Gu::ContactPoint* contactBase0 = buffer.contacts + c.contactPatches[c.correlationListHeads[i]].start;

		const Vec3V normal = Ps::aos::V3LoadU(frictionPatch.normal);

		const FloatV restitution = FLoad(contactBase0->restitution);

		const FloatV normalVel0 = V3Dot(normal, linVel0);
		const FloatV normalVel1 = V3Dot(normal, linVel1);
	
		
		PxcSolverContactCoulombHeader* PX_RESTRICT header = reinterpret_cast<PxcSolverContactCoulombHeader*>(ptr);
		ptr += sizeof(PxcSolverContactCoulombHeader);

		Ps::prefetchLine(ptr, 128);
		Ps::prefetchLine(ptr, 256);
		Ps::prefetchLine(ptr, 384);


		header->numNormalConstr		= (PxU8)contactCount;
		header->type				= pointHeaderType;
		//header->setRestitution(n.restitution);
		header->setRestitution(contactBase0->restitution);
		
		header->setDominance0(invMass0_dom0fV);
		header->setDominance1(invMass1_dom1fV);
		header->setNormal(normal);
		header->flags = flags;

		
		for(PxU32 patch=c.correlationListHeads[i]; 
			patch!=PxcCorrelationBuffer::LIST_END; 
			patch = c.contactPatches[patch].next)
		{
			const PxU32 count = c.contactPatches[patch].count;
			const Gu::ContactPoint* contactBase = buffer.contacts + c.contactPatches[patch].start;
				
			PxU8* p = ptr;
			for(PxU32 j=0;j<count;j++)
			{
				const Gu::ContactPoint& contact = contactBase[j];

				PxcSolverContact* PX_RESTRICT solverContact = reinterpret_cast<PxcSolverContact*>(p);
				p += pointStride;

				const Vec3V point = V3LoadU(contact.point);
				const FloatV separation = FLoad(contact.separation);
				const FloatV cTargetVel = V3Dot(normal, V3LoadA(contact.targetVel));

				const Vec3V ra = V3Sub(point, bodyFrame0p);
				const Vec3V rb = V3Sub(point, bodyFrame1p);
				const FloatV maxImpulse = FLoad(contact.maxImpulse);

				const Vec3V raXn = V3Cross(ra, normal);
				const Vec3V rbXn = V3Cross(rb, normal);

				const Vec3V delAngVel0 = M33MulV3(invInertia0_dom0, raXn);
				const Vec3V delAngVel1 = M33MulV3(invInertia1_dom1, rbXn);

				const FloatV resp0 = FAdd(invMass0_dom0fV, V3Dot(raXn, delAngVel0));
				const FloatV resp1 = FAdd(invMass1_dom1fV, V3Dot(rbXn, delAngVel1));

				const FloatV unitResponse = FSub(resp0, resp1);

				const FloatV vrel1 = FAdd(normalVel0, V3Dot(raXn, angVel0));
				const FloatV vrel2 = FAdd(normalVel1, V3Dot(rbXn, angVel1));
				const FloatV vrel = FSub(vrel1, vrel2);

				const FloatV velMultiplier = FSel(FIsGrtr(unitResponse, zero), FRecip(unitResponse), zero);

				const FloatV penetration = FSub(separation, restDistance);

				const FloatV penetrationInvDtPt8 = FMax(maxPenBias, FMul(penetration, invDtp8));

				FloatV scaledBias = FMul(velMultiplier, penetrationInvDtPt8);

				const FloatV penetrationInvDt = FMul(penetration, invDt);

				solverContact->delAngVel0_InvMassADom = V4SetW(Vec4V_From_Vec3V(delAngVel0), invMass0_dom0fV);
				solverContact->delAngVel1_InvMassBDom = V4SetW(Vec4V_From_Vec3V(delAngVel1), invMass1_dom1fV);

				const BoolV isGreater2 = BAnd(BAnd(FIsGrtr(restitution, zero), FIsGrtr(bounceThreshold, vrel)), 
					FIsGrtr(FNeg(vrel), penetrationInvDt));

				scaledBias = FSel(isGreater2, zero, scaledBias);
				const FloatV targetVelocity = FAdd(FSel(isGreater2, FMul(FNeg(vrel), restitution), zero), cTargetVel);

				solverContact->raXnXYZ_appliedForceW = V4SetW(Vec4V_From_Vec3V(raXn), zero);
				solverContact->rbXnXYZ_velMultiplierW = V4SetW(Vec4V_From_Vec3V(rbXn), velMultiplier);

				solverContact->scaledBiasX_targetVelocityY_maxImpulseZ = V3Merge(scaledBias, targetVelocity, maxImpulse);

			}			
			ptr = p;
		}
	}

	//construct all the frictions

	PxU8* PX_RESTRICT ptr2 = workspace;

	bool hasFriction = false;
	for(PxU32 i=0;i< frictionPatchCount;i++)
	{
		const PxU32 contactCount = c.frictionPatchContactCounts[i];
		if(contactCount == 0)
			continue;

		const Gu::ContactPoint* contactBase0 = buffer.contacts + c.contactPatches[c.correlationListHeads[i]].start;

		PxcSolverContactCoulombHeader* header = reinterpret_cast<PxcSolverContactCoulombHeader*>(ptr2); 
		header->frictionOffset = PxU16(ptr - ptr2);// + sizeof(PxcSolverFrictionHeader);
		ptr2 += sizeof(PxcSolverContactCoulombHeader) + header->numNormalConstr * pointStride;

		const PxReal staticFriction = contactBase0->staticFriction;
		const PxU32 disableStrongFriction = contactBase0->internalFaceIndex1 & PxMaterialFlag::eDISABLE_FRICTION;
		const bool haveFriction = (disableStrongFriction == 0);
	
		PxcSolverFrictionHeader* frictionHeader = (PxcSolverFrictionHeader*)ptr;
		frictionHeader->numNormalConstr = Ps::to8(c.frictionPatchContactCounts[i]);
		frictionHeader->numFrictionConstr = Ps::to8(haveFriction ? c.frictionPatches[i].numConstraints : 0);
		ptr += sizeof(PxcSolverFrictionHeader);
		ptr += frictionHeader->getAppliedForcePaddingSize(c.frictionPatchContactCounts[i]);
		Ps::prefetchLine(ptr, 128);
		Ps::prefetchLine(ptr, 256);
		Ps::prefetchLine(ptr, 384);

		const Vec3V normal = V3LoadU(buffer.contacts[c.contactPatches[c.correlationListHeads[i]].start].normal);

		const FloatV normalX = V3GetX(normal);
		const FloatV normalY = V3GetY(normal);
		const FloatV normalZ = V3GetZ(normal);
		
		const Vec3V t0Fallback1 = V3Merge(zero, FNeg(normalZ), normalY);
		const Vec3V t0Fallback2 = V3Merge(FNeg(normalY), normalX, zero) ;

		const BoolV con = FIsGrtr(orthoThreshold, FAbs(normalX));
		const Vec3V tFallback1 = V3Sel(con, t0Fallback1, t0Fallback2);

		const Vec3V vrel = V3Sub(linVel0, linVel1);
		const Vec3V t0_ = V3Sub(vrel, V3Scale(normal, V3Dot(normal, vrel)));
		const FloatV sqDist = V3Dot(t0_,t0_);
		const BoolV con1 = FIsGrtr(sqDist, eps);
		const Vec3V tDir0 =V3Normalize(V3Sel(con1, t0_, tFallback1));
		const Vec3V tDir1 = V3Cross(tDir0, normal);

		Vec3V tFallback = tDir0;
		Vec3V tFallbackAlt = tDir1;

		if(haveFriction)
		{
			//frictionHeader->setStaticFriction(n.staticFriction);
			frictionHeader->setStaticFriction(staticFriction);
			frictionHeader->setDominance0(n.dominance0);
			frictionHeader->setDominance1(n.dominance1);
			frictionHeader->type			= frictionHeaderType;
			
			PxU32 totalPatchContactCount = 0;
		
			for(PxU32 patch=c.correlationListHeads[i]; 
				patch!=PxcCorrelationBuffer::LIST_END; 
				patch = c.contactPatches[patch].next)
			{
				const PxU32 count = c.contactPatches[patch].count;
				const PxU32 start = c.contactPatches[patch].start;
				const Gu::ContactPoint* contactBase = buffer.contacts + start;
					
				PxU8* p = ptr;
				for(PxU32 j =0; j < count; j++)
				{
					hasFriction = true;
					const PxU32 contactId = totalPatchContactCount + j;
					const Gu::ContactPoint& contact = contactBase[j];
					const Vec3V point = V3LoadU(contact.point);
					const Vec3V ra = V3Sub(point, bodyFrame0p);
					const Vec3V rb = V3Sub(point, bodyFrame1p);
					const Vec3V targetVel = V3LoadU(contact.targetVel);

					for(PxU32 k = 0; k < frictionPerPointCount; ++k)
					{
						const Vec3V t0 = tFallback;
						tFallback = tFallbackAlt;
						tFallbackAlt = t0;

						PxcSolverFriction* PX_RESTRICT f0 = reinterpret_cast<PxcSolverFriction*>(p);
						p += frictionStride;
						f0->contactIndex = contactId;

						const Vec3V raXn = V3Cross(ra, t0);
						const Vec3V rbXn = V3Cross(rb, t0);

						const Vec3V delAngVel0 = M33MulV3(invInertia0_dom0, raXn);
						const Vec3V delAngVel1 = M33MulV3(invInertia1_dom1, rbXn);

						const FloatV resp0 = FAdd(invMass0_dom0fV, V3Dot(raXn, delAngVel0));
						const FloatV resp1 = FAdd(invMass1_dom1fV, V3Dot(rbXn, delAngVel1));
						const FloatV resp = FSub(resp0, resp1);

						const FloatV velMultiplier = FNeg(FSel(FIsGrtr(resp, zero), FRecip(resp), zero));

						f0->normalXYZ_appliedForceW = V4SetW(Vec4V_From_Vec3V(t0), zero);
						f0->raXnXYZ_velMultiplierW = V4SetW(Vec4V_From_Vec3V(raXn), velMultiplier);
						f0->rbXnXYZ_targetVelocityW = V4SetW(Vec4V_From_Vec3V(rbXn), V3Dot(targetVel, t0));
						f0->delAngVel0_InvMassADom = V4SetW(Vec4V_From_Vec3V(delAngVel0), invMass0_dom0fV);
						f0->delAngVel1_InvMassBDom = V4SetW(Vec4V_From_Vec3V(delAngVel1), invMass1_dom1fV);
					}
				}

				totalPatchContactCount += c.contactPatches[patch].count;
				
				ptr = p;	
			}
		}
	}
	*ptr = 0;
	return hasFriction;
}



#endif
