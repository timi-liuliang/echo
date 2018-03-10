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
#include "PxsSolverConstraintExt.h"
#include "PxcSolverConstraintDesc.h"
#include "PxcSolverConstraint1D.h"
#include "PxsSolverContact.h"
#include "PxcArticulationHelper.h"
#include "PxcNpWorkUnit.h"
#include "PxsMaterialManager.h"
#include "PxsMaterialCombiner.h"

using namespace physx;
using namespace Gu;

// constraint-gen only, since these use getVelocityFast methods
// which aren't valid during the solver phase

PxReal PxsSolverExtBody::projectVelocity(const PxVec3& linear, const PxVec3& angular) const
{
	if(mLinkIndex == PxcSolverConstraintDesc::NO_LINK)
		return mBody->projectVelocity(linear, angular);
	else
		return FStore(PxcArticulationHelper::getVelocityFast(*mFsData, mLinkIndex).dot(Cm::SpatialVector(linear, angular)));
}

PxVec3 PxsSolverExtBody::getLinVel() const
{
	if(mLinkIndex == PxcSolverConstraintDesc::NO_LINK)
		return mBody->linearVelocity;
	else
	{
		PxVec3 result;
		V3StoreU(PxcArticulationHelper::getVelocityFast(*mFsData, mLinkIndex).linear, result);
		return result;
	}
}

PxVec3 PxsSolverExtBody::getAngVel() const
{
	if(mLinkIndex == PxcSolverConstraintDesc::NO_LINK)
		return mBody->angularVelocity;
	else
	{
		PxVec3 result;
		V3StoreU(PxcArticulationHelper::getVelocityFast(*mFsData, mLinkIndex).angular, result);
		return result;
	}
}

namespace
{

}
PxReal physx::getImpulseResponse(const PxsSolverExtBody& b0, const Cm::SpatialVector& impulse0, Cm::SpatialVector& deltaV0, PxReal dom0, PxReal angDom0,
								 const PxsSolverExtBody& b1, const Cm::SpatialVector& impulse1, Cm::SpatialVector& deltaV1, PxReal dom1, PxReal angDom1,
								 bool /*allowSelfCollision*/)
{
	//	allowSelfCollision = true;
	// right now self-collision with contacts crashes the solver
	
	//KS - knocked this out to save some space on SPU
	//if(allowSelfCollision && b0.mLinkIndex!=PxcSolverConstraintDesc::NO_LINK && b0.mFsData == b1.mFsData)
	//{
	//	PxcArticulationHelper::getImpulseSelfResponse(*b0.mFsData,b0.mLinkIndex, impulse0, deltaV0, 
	//												  b1.mLinkIndex, impulse1, deltaV1);
	//	//PxReal response = impulse0.dot(deltaV0*dom0) + impulse1.dot(deltaV1*dom1);
	//	PX_ASSERT(PxAbs(impulse0.dot(deltaV0*dom0) + impulse1.dot(deltaV1*dom1))>0);
	//}
	//else 
	{
		if(b0.mLinkIndex == PxcSolverConstraintDesc::NO_LINK)
			b0.mBody->getResponse(impulse0.linear * dom0, impulse0.angular * angDom0, deltaV0.linear, deltaV0.angular, *b0.mBodyData);
		else
			PxcArticulationHelper::getImpulseResponse(*b0.mFsData, b0.mLinkIndex, impulse0.scale(dom0, angDom0), deltaV0);

		if(b1.mLinkIndex == PxcSolverConstraintDesc::NO_LINK)
			b1.mBody->getResponse(impulse1.linear * dom1, impulse1.angular * angDom1, deltaV1.linear, deltaV1.angular , *b1.mBodyData);
		else
			PxcArticulationHelper::getImpulseResponse(*b1.mFsData, b1.mLinkIndex, impulse1.scale(dom1, angDom1), deltaV1);
	}

	PxReal response = impulse0.dot(deltaV0) + impulse1.dot(deltaV1);

	return response;
}


PxReal physx::getImpulseResponse(const PxsSolverExtBody& b0, const Cm::SpatialVector& impulse0, Cm::SpatialVector& deltaV0, PxReal dom0,
								 const PxsSolverExtBody& b1, const Cm::SpatialVector& impulse1, Cm::SpatialVector& deltaV1, PxReal dom1,
								 bool /*allowSelfCollision*/)
{
	//	allowSelfCollision = true;
	// right now self-collision with contacts crashes the solver
	
	//KS - knocked this out to save some space on SPU
	//if(allowSelfCollision && b0.mLinkIndex!=PxcSolverConstraintDesc::NO_LINK && b0.mFsData == b1.mFsData)
	//{
	//	PxcArticulationHelper::getImpulseSelfResponse(*b0.mFsData,b0.mLinkIndex, impulse0, deltaV0, 
	//												  b1.mLinkIndex, impulse1, deltaV1);
	//	//PxReal response = impulse0.dot(deltaV0*dom0) + impulse1.dot(deltaV1*dom1);
	//	PX_ASSERT(PxAbs(impulse0.dot(deltaV0*dom0) + impulse1.dot(deltaV1*dom1))>0);
	//}
	//else 
	{
		if(b0.mLinkIndex == PxcSolverConstraintDesc::NO_LINK)
			b0.mBody->getResponse(impulse0.linear, impulse0.angular, deltaV0.linear, deltaV0.angular, *b0.mBodyData);
		else
			PxcArticulationHelper::getImpulseResponse(*b0.mFsData,b0.mLinkIndex, impulse0, deltaV0);

		if(b1.mLinkIndex == PxcSolverConstraintDesc::NO_LINK)
			b1.mBody->getResponse(impulse1.linear, impulse1.angular, deltaV1.linear, deltaV1.angular, *b1.mBodyData);
		else
			PxcArticulationHelper::getImpulseResponse(*b1.mFsData, b1.mLinkIndex, impulse1, deltaV1);
	}

	PxReal response = impulse0.dot(deltaV0) * dom0 + impulse1.dot(deltaV1) * dom1;

	deltaV0 = deltaV0 * dom0;
	deltaV1 = deltaV1 * dom1;

	return response;
}


namespace physx
{


	void setupFinalizeExtSolverConstraints(PxcNpWorkUnit& n,
						    const ContactBuffer& buffer,
							const PxcCorrelationBuffer& c,
							const PxTransform& bodyFrame0,
							const PxTransform& bodyFrame1,
							bool perPointFriction,
							PxU8* workspace,
							const PxsSolverExtBody& b0,
							const PxsSolverExtBody& b1,
							const PxcSolverBodyData& /*data0*/,
							const PxcSolverBodyData& /*data1*/,
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

	PX_ASSERT(n.frictionDataPtr);

	/*const bool haveFriction = PX_IR(n.staticFriction) > 0 || PX_IR(n.dynamicFriction) > 0;*/

	PxU8* PX_RESTRICT ptr = workspace;

	const FloatV zero=FZero();

	//KS - TODO - this should all be done in SIMD to avoid LHS
	const PxF32 maxPenBias0 = b0.mLinkIndex == PxcSolverConstraintDesc::NO_LINK ? b0.mBodyData->penBiasClamp : getMaxPenBias(*b0.mFsData)[b0.mLinkIndex];
	const PxF32 maxPenBias1 = b1.mLinkIndex == PxcSolverConstraintDesc::NO_LINK ? b1.mBodyData->penBiasClamp : getMaxPenBias(*b1.mFsData)[b1.mLinkIndex];

	const FloatV maxPenBias = FLoad(PxMax(maxPenBias0, maxPenBias1));
	
	const PxReal d0 = n.dominance0 * invMassScale0;
	const PxReal d1 = n.dominance1 * invMassScale1;

	const PxReal angD0 = n.dominance0 * invInertiaScale0;
	const PxReal angD1 = n.dominance1 * invInertiaScale1;

	Vec4V staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W = V4Zero();
	staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, FLoad(d0));
	staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, FLoad(d1));

	const FloatV restDistance = FLoad(n.restDistance); 

	PxU32 frictionPatchWritebackAddrIndex = 0;
	PxU32 contactWritebackCount = 0;

	Ps::prefetchLine(c.contactID);
	Ps::prefetchLine(c.contactID, 128);

	const FloatV invDt = FLoad(invDtF32);
	const FloatV p8 = FLoad(0.8f);
	const FloatV bounceThreshold = FLoad(bounceThresholdF32);

	const FloatV invDtp8 = FMul(invDt, p8);

	bool useExtContacts = (n.flags & (PxcNpWorkUnitFlag::eARTICULATION_BODY0|PxcNpWorkUnitFlag::eARTICULATION_BODY1))!=0;

	for(PxU32 i=0;i<c.frictionPatchCount;i++)
	{
		PxU32 contactCount = c.frictionPatchContactCounts[i];
		if(contactCount == 0)
			continue;

		const PxcFrictionPatch& frictionPatch = c.frictionPatches[i];
		PX_ASSERT(frictionPatch.anchorCount <= 2);  //0==anchorCount is allowed if all the contacts in the manifold have a large offset. 

		const Gu::ContactPoint* contactBase0 = buffer.contacts + c.contactPatches[c.correlationListHeads[i]].start;
		const PxReal combinedRestitution = contactBase0->restitution;

		const PxReal staticFriction = contactBase0->staticFriction;
		const PxReal dynamicFriction = contactBase0->dynamicFriction;
		const PxU32 disableStrongFriction = contactBase0->internalFaceIndex1 & PxMaterialFlag::eDISABLE_FRICTION;
		staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, FLoad(staticFriction));
		staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, FLoad(dynamicFriction));
	
		PxcSolverContactHeader* PX_RESTRICT header = reinterpret_cast<PxcSolverContactHeader*>(ptr);
		ptr += sizeof(PxcSolverContactHeader);		


		Ps::prefetchLine(ptr + 128);
		Ps::prefetchLine(ptr + 256);
		Ps::prefetchLine(ptr + 384);
		
		const bool haveFriction = (disableStrongFriction == 0) ;//PX_IR(n.staticFriction) > 0 || PX_IR(n.dynamicFriction) > 0;
		header->numNormalConstr		= Ps::to8(contactCount);
		header->numFrictionConstr	= Ps::to8(haveFriction ? frictionPatch.anchorCount*2 : 0);
	
		header->type				= Ps::to8(useExtContacts ? PXS_SC_TYPE_EXT_CONTACT 
			                                         : header->numFrictionConstr == 0 ? PXS_SC_TYPE_NOFRICTION_RB_CONTACT 
													 : (n.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1) == 0 ? PXS_SC_TYPE_STATIC_CONTACT 
									                 : PXS_SC_TYPE_RB_CONTACT);

		const FloatV restitution = FLoad(combinedRestitution);
	
		header->staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W = staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W;

		header->angDom0 = angD0;
		header->angDom1 = angD1;
	
		PxU32 pointStride = useExtContacts ? sizeof(PxcSolverContactPointExt) : sizeof(PxcSolverContactPoint);
		PxU32 frictionStride = useExtContacts ? sizeof(PxcSolverContactFrictionExt) : sizeof(PxcSolverContactFriction);

		
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

				PxcSolverContactPointExt* PX_RESTRICT solverContact = reinterpret_cast<PxcSolverContactPointExt*>(p);
				p += pointStride;

				//const Vec3V targetVel = Vec3V_From_PxVec3(contact.targetVel);
				const Vec3V normal = V3LoadU(contact.normal);
				const FloatV separation = FLoad(contact.separation);

				const FloatV penetration = FSub(separation, restDistance);

				const PxVec3 ra = contact.point - bodyFrame0.p;
				const PxVec3 rb = contact.point - bodyFrame1.p;

				const PxVec3 raXn = ra.cross(contact.normal);
				const PxVec3 rbXn = rb.cross(contact.normal);

				Cm::SpatialVector deltaV0, deltaV1;

				const FloatV unitResponse = FLoad(getImpulseResponse(b0, Cm::SpatialVector(contact.normal, raXn), deltaV0, d0, angD0,
																	 b1, Cm::SpatialVector(-contact.normal, -rbXn), deltaV1, d1, angD1));

				const FloatV vrel = FLoad(b0.projectVelocity(contact.normal, raXn)
								  - b1.projectVelocity(contact.normal, rbXn));

				FloatV velMultiplier = FSel(FIsEq(unitResponse, zero), zero, FRecip(unitResponse));
				FloatV scaledBias = FMul(velMultiplier, FMax(maxPenBias, FMul(penetration, invDtp8)));
				const FloatV penetrationInvDt = FMul(penetration, invDt);

				const BoolV isGreater2 = BAnd(BAnd(FIsGrtr(restitution, zero), FIsGrtr(bounceThreshold, vrel)), FIsGrtr(FNeg(vrel), penetrationInvDt));

				scaledBias = FSel(isGreater2, zero, scaledBias);
				FloatV targetVelocity = FSel(isGreater2, FMul(FNeg(vrel), restitution), zero);

				solverContact->normalXYZ_velMultiplierW = V4SetW(Vec4V_From_Vec3V(normal), velMultiplier);
				solverContact->raXnXYZ_appliedForceW = V4SetW(Vec4V_From_Vec3V(V3LoadU(raXn)), zero);
				solverContact->rbXnXYZ_scaledBiasW = V4SetW(Vec4V_From_Vec3V(V3LoadU(rbXn)), scaledBias);
				solverContact->delAngVel0_targetVelocityW = V4SetW(Vec4V_From_Vec3V(V3LoadU(deltaV0.angular)), targetVelocity);
				solverContact->delAngVel1_maxImpulseW = V4SetW(Vec4V_From_Vec3V(V3LoadU(deltaV1.angular)), FLoad(contact.maxImpulse));
				solverContact->linDeltaVA = Vec4V_From_Vec3V(V3LoadU(deltaV0.linear));
				solverContact->linDeltaVB = Vec4V_From_Vec3V(V3LoadU(deltaV1.linear));
			}

			ptr = p;
		}
		contactWritebackCount += contactCount;

		if(haveFriction)
		{
			//const Vec3V normal = Vec3V_From_PxVec3(buffer.contacts[c.contactPatches[c.correlationListHeads[i]].start].normal);
			PxVec3 normalS = buffer.contacts[c.contactPatches[c.correlationListHeads[i]].start].normal;

			PxVec3 t0, t1;
			computeFrictionTangents(b0.getLinVel() - b1.getLinVel(), normalS, t0, t1);

			Vec3V vT0 = V3LoadU(t0);
			Vec3V vT1 = V3LoadU(t1);
			
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
				PxcSolverContactFrictionExt* PX_RESTRICT f0 = reinterpret_cast<PxcSolverContactFrictionExt*>(ptr);
				ptr += frictionStride;
				PxcSolverContactFrictionExt* PX_RESTRICT f1 = reinterpret_cast<PxcSolverContactFrictionExt*>(ptr);
				ptr += frictionStride;

				f0->frictionBrokenWritebackByte = writeback;
				f1->frictionBrokenWritebackByte = writeback;

				PxVec3 ra = bodyFrame0.q.rotate(frictionPatch.body0Anchors[j]);
				PxVec3 rb = bodyFrame1.q.rotate(frictionPatch.body1Anchors[j]);
				PxVec3 error = (ra + bodyFrame0.p) - (rb + bodyFrame1.p);

				{
					const PxVec3 raXn = ra.cross(t0);
					const PxVec3 rbXn = rb.cross(t0);

					Cm::SpatialVector deltaV0, deltaV1;

					FloatV resp = FLoad(getImpulseResponse(b0, Cm::SpatialVector(t0, raXn), deltaV0, d0, angD0,
															 b1, Cm::SpatialVector(-t0, -rbXn), deltaV1, d1, angD1));

					const FloatV velMultiplier = FSel(FIsGrtr(resp, zero), FMul(p8, FRecip(resp)), zero);

					PxU32 index = perPointFriction ? c.contactID[i][j] : c.contactPatches[c.correlationListHeads[i]].start;
					const PxF32 targetVel = buffer.contacts[index].targetVel.dot(t0);

					f0->normalXYZ_appliedForceW = V4SetW(vT0, zero);
					f0->raXnXYZ_velMultiplierW = V4SetW(V3LoadU(raXn), velMultiplier);
					f0->rbXnXYZ_biasW = V4SetW(V3LoadU(rbXn), FLoad(t0.dot(error) * invDtF32));
					f0->delAngVel0_BrokenW = V4SetW(Vec4V_From_Vec3V(V3LoadU(deltaV0.angular)), FZero());
					f0->delAngVel1_TargetVelW = V4SetW(Vec4V_From_Vec3V(V3LoadU(deltaV1.angular)), FLoad(targetVel));
					f0->linDeltaVA = Vec4V_From_Vec3V(V3LoadU(deltaV0.linear));
					f0->linDeltaVB = Vec4V_From_Vec3V(V3LoadU(deltaV1.linear));
				}

				{

					const PxVec3 raXn = ra.cross(t1);
					const PxVec3 rbXn = rb.cross(t1);

					Cm::SpatialVector deltaV0, deltaV1;

					FloatV resp = FLoad(getImpulseResponse(b0, Cm::SpatialVector(t1, raXn), deltaV0, d0, angD0,
														   b1, Cm::SpatialVector(-t1, -rbXn), deltaV1, d1, angD1));

					const FloatV velMultiplier = FSel(FIsGrtr(resp, zero), FMul(p8, FRecip(resp)), zero);

					PxU32 index = perPointFriction ? c.contactID[i][j] : c.contactPatches[c.correlationListHeads[i]].start;
					const PxF32 targetVel = buffer.contacts[index].targetVel.dot(t0);

					f1->normalXYZ_appliedForceW = V4SetW(vT1, zero);
					f1->raXnXYZ_velMultiplierW = V4SetW(V3LoadU(raXn), velMultiplier);
					f1->rbXnXYZ_biasW = V4SetW(V3LoadU(rbXn), FLoad(t1.dot(error) * invDtF32));
					f1->delAngVel0_BrokenW = V4SetW(Vec4V_From_Vec3V(V3LoadU(deltaV0.angular)), FZero());
					f1->delAngVel1_TargetVelW = V4SetW(Vec4V_From_Vec3V(V3LoadU(deltaV1.angular)), FLoad(targetVel));
					f1->linDeltaVA = Vec4V_From_Vec3V(V3LoadU(deltaV0.linear));
					f1->linDeltaVB = Vec4V_From_Vec3V(V3LoadU(deltaV1.linear));
				}
			}
		}

		frictionPatchWritebackAddrIndex++;
	}

	PX_ASSERT(PxU32(ptr - workspace) == n.solverConstraintSize);
}


}
