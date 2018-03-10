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
#include "PxSceneDesc.h"
#include "PsVecMath.h"
#include "PsMathUtils.h"
#include "PxcSolverContact.h"
#include "PxcSolverContact4.h"
#include "PxcSolverConstraintTypes.h"
#include "PxcNpWorkUnit.h"
#include "PxcNpThreadContext.h"
#include "PxcNpContactPrep.h"
#include "PxvGeometry.h"
#include "PxvDynamics.h"
#include "PxcCorrelationBuffer.h"
#include "PxsDynamics.h"
//#include "PxsContext.h"
#include "PxsSolverConstraintExt.h"

using namespace physx;
using namespace Gu;


#include "PsVecMath.h"
#include "PxContactModifyCallback.h"
#include "PxsMaterialManager.h"
#include "PxsMaterialCombiner.h"

using namespace physx;
using namespace Ps::aos;

#define PX_ENABLE_AVG_POINT 1

#ifdef __SPU__
#include "..\..\ps3\include\spu\SpuNpMemBlock.h"
#include "PxsResourceManager.h"

namespace physx
{
	extern SpuNpMemBlock gMemBlock;
	extern SpuNpMemBlock gMemBlockFrictionPatches;
}
#endif

PxcCreateFinalizeSolverContactMethod physx::createFinalizeMethods[3] =
{
	createFinalizeSolverContacts,
	createFinalizeSolverContactsCoulomb,
	createFinalizeSolverContactsCoulomb
};

PxcCreateFinalizeSolverContactMethod4 physx::createFinalizeMethods4[3] = 
{
	createFinalizeSolverContacts4,
	createFinalizeSolverContacts4Coulomb,
	createFinalizeSolverContacts4Coulomb
};



//// Transform points then test against threshold
//static PX_FORCE_INLINE bool pointsAreClose(const PxTransform& bodyFrame0, const PxTransform& bodyFrame1,
//									const PxVec3& localAnchor0, const PxVec3& localAnchor1,
//									const PxVec3& axis, float correlDist)
//{
//	const PxVec3 worldPatchPoint0 = bodyFrame0.transform(localAnchor0);
//	const PxVec3 worldPatchPoint1 = bodyFrame1.transform(localAnchor1);
//
//    return PxAbs((worldPatchPoint0 - worldPatchPoint1).dot(axis))<correlDist;
//}

static PX_FORCE_INLINE bool pointsAreClose(const PxTransform& body1ToBody0,
									const PxVec3& localAnchor0, const PxVec3& localAnchor1,
									const PxVec3& axis, float correlDist)
{
	const PxVec3 worldPatchPoint0 = localAnchor0;
	const PxVec3 worldPatchPoint1 = body1ToBody0.transform(localAnchor1);

	return PxAbs((worldPatchPoint0 - worldPatchPoint1).dot(axis))<correlDist;

	/*const PxVec3 dif = (worldPatchPoint0 - worldPatchPoint1);
	const PxVec3 tangentialDif = dif - (axis * dif.dot(axis));

	return tangentialDif.magnitudeSquared()<correlDist;*/
}


static bool getFrictionPatches(PxcFrictionPatchStreamPair& streams,
						PxcCorrelationBuffer& c,
						const PxU8* frictionCookie,
						PxU32 frictionPatchCount,
						const PxTransform& bodyFrame0,
						const PxTransform& bodyFrame1,
						PxReal correlationDistance)
{
	if(frictionCookie == NULL)
		return true;

	//KS - this is now DMA'd inside the shader so we don't need to immediate DMA it here
//#ifdef __SPU__
//	const uintptr_t eaFrictionCookie=(uintptr_t)frictionCookie;
//	PX_ASSERT(0==((uintptr_t)c.frictionPatches & 0x0f));
//	PX_ASSERT(0==(eaFrictionCookie & 0x0f));
//	Cm::memFetchAlignedAsync((Cm::MemFetchPtr)c.frictionPatches, (Cm::MemFetchPtr)eaFrictionCookie, CELL_ALIGN_SIZE_16(frictionPatchCount*sizeof(PxcFrictionPatch)), 1);
//	Cm::memFetchWait(1);
//	const PxcFrictionPatch* patches = c.frictionPatches;
//#else
//	const PxcFrictionPatch* patches = streams.findInputPatches<PxcFrictionPatch>(frictionCookie);
//#endif
	const PxcFrictionPatch* patches = streams.findInputPatches<PxcFrictionPatch>(frictionCookie);

	//Try working out relative transforms!
	const PxTransform body1ToBody0 = bodyFrame0.transformInv(bodyFrame1);

	while(frictionPatchCount--)
	{
		Ps::prefetchLine(patches,128);
		const PxcFrictionPatch& patch = *patches++;
		/*if(patch.broken)
			continue;*/
		

		// if the eDISABLE_STRONG_FRICTION flag is there we need to blow away the previous frame's friction correlation, so
		// that we can associate each friction anchor with a target velocity. So we lose strong friction.
		//const PxsMaterialCore* data0 = materialManager->getMaterial(patch.materialIndex0);
		//const PxsMaterialCore* data1 = materialManager->getMaterial(patch.materialIndex1);

		const bool disableStrongFriction = (patch.materialFlags & PxMaterialFlag::eDISABLE_STRONG_FRICTION) == 1;

		PX_ASSERT (patch.broken == 0 || patch.broken == 1);
		//if(!patch.broken)//if(!disableStrongFriction)
		if(!patch.broken && !disableStrongFriction && patch.anchorCount != 0)
		{
			PX_ASSERT(patch.anchorCount <= 2);

			/*const PxVec3 worldNormal = bodyFrame0.rotate(patch.body0Normal);
			if(worldNormal.dot(bodyFrame1.rotate(patch.body1Normal)) > PXC_SAME_NORMAL)*/
			const PxVec3 worldNormal = patch.body0Normal;
			if(worldNormal.dot(body1ToBody0.rotate(patch.body1Normal)) > PXC_SAME_NORMAL)
			{
				//const bool anchor1Sep = !pointsAreClose(bodyFrame0, bodyFrame1, patch.body0Anchors[0], patch.body1Anchors[0], worldNormal, correlationDistance);
				const bool anchor1Sep = !pointsAreClose(body1ToBody0, patch.body0Anchors[0], patch.body1Anchors[0], worldNormal, correlationDistance);
				if(!anchor1Sep)
				{
					/*const bool anchor2Sep = patch.anchorCount == 2 &&
									  !pointsAreClose(bodyFrame0, bodyFrame1, patch.body0Anchors[1], patch.body1Anchors[1], worldNormal, correlationDistance);*/
					const bool anchor2Sep = patch.anchorCount == 2 &&
									  !pointsAreClose(body1ToBody0, patch.body0Anchors[1], patch.body1Anchors[1], worldNormal, correlationDistance);
					if(!anchor2Sep)
					{
						// if the eDISABLE_STRONG_FRICTION flag is there we need to blow away the previous frame's friction correlation, so
						// that we can associate each friction anchor with a target velocity. So we lose strong friction.
						//This should put in the outerloop.........
						/*const PxsMaterialCore* data0 = materialManager->getMaterial(patch.materialIndex0);
						const PxsMaterialCore* data1 = materialManager->getMaterial(patch.materialIndex1);

						PxU32 flags = (data0->flags | data1->flags);
						const bool disableStrongFriction = (flags & PxMaterialFlag::eDISABLE_STRONG_FRICTION) == 1;*/
						//if(!disableStrongFriction)
						if(c.frictionPatchCount == PxcCorrelationBuffer::MAX_FRICTION_PATCHES)
							return false;
						{
							c.contactID[c.frictionPatchCount][0] = 0xffff;
							c.contactID[c.frictionPatchCount][1] = 0xffff;
							c.frictionPatches[c.frictionPatchCount++] = patch;
						}
					}
				}
			}
		}
	}
	return true;
}

PX_FORCE_INLINE void computeBlockStreamByteSizes(const PxcNpWorkUnit& n, PxcNpThreadContext& threadContext, const bool forceContacts,
								PxU32& _solverConstraintByteSize, PxU32& _frictionPatchByteSize, PxU32& _numFrictionPatches,
								PxU32& _axisConstraintCount)
{
	PX_ASSERT(0 == _solverConstraintByteSize);
	PX_ASSERT(0 == _frictionPatchByteSize);
	PX_ASSERT(0 == _numFrictionPatches);
	PX_ASSERT(0 == _axisConstraintCount);
	PX_UNUSED(forceContacts);

	// PT: use local vars to remove LHS
	PxU32 solverConstraintByteSize = 0;
	PxU32 numFrictionPatches = 0;
	PxU32 axisConstraintCount = 0;

	//Unpack the thread context to get the structures that we will need to compute the reservation sizes and make the reservations.
	const PxcCorrelationBuffer& c = threadContext.mCorrelationBuffer;
	//const ContactBuffer& buffer = threadContext.mContactBuffer;

	//We can compute contactForceByteSize and frictionPatchByteSize and solverConstraintSize in the following loop.
	const bool useExtContacts = (n.flags & (PxcNpWorkUnitFlag::eARTICULATION_BODY0|PxcNpWorkUnitFlag::eARTICULATION_BODY1)) != 0;
	
	//const bool haveFriction = PX_IR(n.staticFriction) > 0 || PX_IR(n.dynamicFriction) > 0;

	for(PxU32 i = 0; i < c.frictionPatchCount; i++)
	{
		//Friction patches.
		if(c.correlationListHeads[i] != PxcCorrelationBuffer::LIST_END)
			numFrictionPatches++;

		const PxcFrictionPatch& frictionPatch = c.frictionPatches[i];
		/*const PxsMaterialData& data0 = *n.materialManager->getMaterial(frictionPatch.materialIndex0);
		const PxsMaterialData& data1 = *n.materialManager->getMaterial(frictionPatch.materialIndex1);
		const PxU32 flag = (data0.flags | data1.flags);
		const bool haveFriction = ((flag & PxMaterialFlag::eDISABLE_FRICTION) == 0);*/

		const bool haveFriction = (frictionPatch.materialFlags & PxMaterialFlag::eDISABLE_FRICTION) == 0;


		//Solver constraint data.
		if(c.frictionPatchContactCounts[i]!=0)
		{
			solverConstraintByteSize += sizeof(PxcSolverContactHeader);
			solverConstraintByteSize += useExtContacts ? c.frictionPatchContactCounts[i] * sizeof(PxcSolverContactPointExt) 
				: c.frictionPatchContactCounts[i] * sizeof(PxcSolverContactPoint);

			axisConstraintCount += c.frictionPatchContactCounts[i];

			if(haveFriction)
			{
				solverConstraintByteSize += useExtContacts ? c.frictionPatches[i].anchorCount * 2 * sizeof(PxcSolverContactFrictionExt)
					: c.frictionPatches[i].anchorCount * 2 * sizeof(PxcSolverContactFriction);
				axisConstraintCount += c.frictionPatches[i].anchorCount * 2;

			}
		}
	}
	PxU32 frictionPatchByteSize = numFrictionPatches*sizeof(PxcFrictionPatch);

	//Work out if we really need to create a solverConstraint buffer.
	if(!(n.flags & (PxcNpWorkUnitFlag::eOUTPUT_CONSTRAINTS | PxcNpWorkUnitFlag::eMODIFIABLE_CONTACT)))
	{
		solverConstraintByteSize =0;
	}

	_numFrictionPatches = numFrictionPatches;
	_axisConstraintCount = axisConstraintCount;

	//16-byte alignment.
	_frictionPatchByteSize = ((frictionPatchByteSize + 0x0f) & ~0x0f);
	_solverConstraintByteSize =  ((solverConstraintByteSize + 0x0f) & ~0x0f);
	PX_ASSERT(0 == (_solverConstraintByteSize & 0x0f));
	PX_ASSERT(0 == (_frictionPatchByteSize & 0x0f));
}

static bool reserveBlockStreams(const PxcNpWorkUnit& n, PxcNpThreadContext& threadContext, const bool forceContacts,
						PxU8*& solverConstraint,
						PxcFrictionPatch*& _frictionPatches,
#ifdef __SPU__
						uintptr_t& eaSolverConstraint,
						uintptr_t& eaFrictionPatches,
#endif
						PxU32& numFrictionPatches, PxU32& solverConstraintByteSize,
						PxU32& axisConstraintCount, PxsConstraintBlockManager& constraintBlockManager)
{
	PX_ASSERT(NULL == solverConstraint);
	PX_ASSERT(NULL == _frictionPatches);
#ifdef __SPU__
	PX_ASSERT(0 == eaSolverConstraint);
	PX_ASSERT(0 == eaFrictionPatches);
#endif
	PX_ASSERT(0 == numFrictionPatches);
	PX_ASSERT(0 == solverConstraintByteSize);
	PX_ASSERT(0 == axisConstraintCount);

#ifdef __SPU__
	SpuNpMemBlock& constraintBlockStream = gMemBlock;
	SpuNpMemBlock& frictionPatchStream = gMemBlockFrictionPatches;
#else
	PxcConstraintBlockStream& constraintBlockStream = threadContext.mConstraintBlockStream;
	PxcFrictionPatchStreamPair& frictionPatchStream = threadContext.mFrictionPatchStreamPair;
#endif

	//From frictionPatchStream we just need to reserve a single buffer.
	PxU32 frictionPatchByteSize = 0;
	//Compute the sizes of all the buffers.
	computeBlockStreamByteSizes(
		n, threadContext, forceContacts,
		solverConstraintByteSize, frictionPatchByteSize, numFrictionPatches,
		axisConstraintCount);

	//Reserve the buffers.

	//First reserve the accumulated buffer size for the constraint block.
	PxU8* constraintBlock = NULL;
#ifdef __SPU__
	uintptr_t eaConstraintBlock = NULL;
#endif
	const PxU32 constraintBlockByteSize = solverConstraintByteSize;
	if(constraintBlockByteSize > 0)
	{
#ifdef __SPU__
		constraintBlock = constraintBlockStream.reserve(constraintBlockByteSize+16u, &eaConstraintBlock);
#else
		constraintBlock = constraintBlockStream.reserve(constraintBlockByteSize+16u, constraintBlockManager);
#endif
		if(0==constraintBlock || ((PxU8*)(-1))==constraintBlock)
		{
			if(0==constraintBlock)
			{
				PX_WARN_ONCE(true,
					"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for narrow phase. "
					"Either accept dropped contacts or increase buffer size allocated for narrow phase by increasing PxSceneDesc::maxNbContactDataBlocks.");
			}
			else
			{
				PX_WARN_ONCE(true,
					"Attempting to allocate more than 16K of contact data for a single contact pair in narrowphase. "
					"Either accept dropped contacts or simplify collision geometry.");
				constraintBlock=NULL;
			}
		}
	}

	PxcFrictionPatch* frictionPatches = NULL;
	//If the constraint block reservation didn't fail then reserve the friction buffer too.
	if(frictionPatchByteSize >0 && (0==constraintBlockByteSize || constraintBlock))
	{
#ifdef __SPU__
		frictionPatches = (PxcFrictionPatch*)frictionPatchStream.reserve(frictionPatchByteSize, &eaFrictionPatches);
#else
		frictionPatches = frictionPatchStream.reserve<PxcFrictionPatch>(frictionPatchByteSize);
#endif
		if(0==frictionPatches || ((PxcFrictionPatch*)(-1))==frictionPatches)
		{
			if(0==frictionPatches)
			{
				PX_WARN_ONCE(true,
					"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for narrow phase. "
					"Either accept dropped contacts or increase buffer size allocated for narrow phase by increasing PxSceneDesc::maxNbContactDataBlocks.");
			}
			else
			{
				PX_WARN_ONCE(true,
					"Attempting to allocate more than 16K of friction data for a single contact pair in narrowphase. "
					"Either accept dropped contacts or simplify collision geometry.");
				frictionPatches=NULL;
			}
		}
	}

	_frictionPatches = frictionPatches;

	//Patch up the individual ptrs to the buffer returned by the constraint block reservation (assuming the reservation didn't fail).
	if(0==constraintBlockByteSize || constraintBlock)
	{
		if(solverConstraintByteSize)
		{
			solverConstraint = constraintBlock;
			PX_ASSERT(0==((uintptr_t)solverConstraint & 0x0f));
#ifdef __SPU__
			eaSolverConstraint = eaConstraintBlock;
#endif
		}
	}
	threadContext.mConstraintSize += (constraintBlockByteSize);

	//Return true if neither of the two block reservations failed.
	return ((0==constraintBlockByteSize || constraintBlock) && (0==frictionPatchByteSize || frictionPatches));
}


bool physx::createFinalizeSolverContacts(PxcNpWorkUnit& n,
								 PxcNpThreadContext& threadContext,
								 bool forceContacts,
								 bool perPointFriction,
								 PxcSolverConstraintDesc& desc,
								 const PxcSolverBody* sBody0,
								 const PxcSolverBody* sBody1,
								 const PxcSolverBodyData* solverBodyData0,
								 const PxcSolverBodyData* solverBodyData1,
								 const PxsRigidCore* body0,
								 const PxsRigidCore* body1,
								 const PxReal invDtF32,
								 PxReal bounceThresholdF32,
								 PxReal frictionOffsetThreshold,
								 PxReal correlationDistance,
								 PxsConstraintBlockManager& constraintBlockManager,
								 PxFrictionType::Enum frictionType
#ifdef __SPU__
								 , bool& overflowed
#endif
								 )
{
	PX_UNUSED(frictionType);

	//CM_PROFILE_ZONE(context.getContext()->getEventProfiler(),Cm::ProfileEventId::Sim::createFinalize);
	ContactBuffer& buffer = threadContext.mContactBuffer;

	desc.constraintLengthOver16 = 0;

	buffer.count = 0;

	PxU32 numContacts = 0;
	PxU32 patchCount = 0;

	// We pull the friction patches out of the cache to remove the dependency on how
	// the cache is organized. Remember original addrs so we can write them back 
	// efficiently.

	Ps::prefetchLine(n.frictionDataPtr);

		// PT: those transforms have already been accessed by the calling code so maybe we could avoid fetching them again
	// PT: this form is cleaner but has LHS...
//	const PxTransform& bodyFrame0 = body0 ? body0->body2World : PxTransform(PxIdentity);
//	const PxTransform& bodyFrame1 = body1 ? body1->body2World : PxTransform(PxIdentity);
	// PT: ...while the code below has none
	PxTransform idt = PxTransform(PxIdentity);
	const PxTransform* bodyFrame0;
	if(n.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY0)
	{
		bodyFrame0 = &body0->body2World;
	}
	else
	{
		bodyFrame0 = &idt;
	}

	const PxTransform* bodyFrame1;
	if(n.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1)
	{
		bodyFrame1 = &body1->body2World;
	}
	else
	{
		bodyFrame1 = &idt;
	}

	Ps::prefetchLine(bodyFrame0);
	Ps::prefetchLine(bodyFrame1);

	PxReal invMassScale0 = 1.f;
	PxReal invMassScale1 = 1.f;
	PxReal invInertiaScale0 = 1.f;
	PxReal invInertiaScale1 = 1.f;

	if(n.compressedContacts)
	{
		PxContactStreamIterator iter(n.compressedContacts, n.compressedContactSize);
		
		if(!iter.forceNoResponse)
		{
			invMassScale0 = iter.getInvMassScale0();
			invMassScale1 = iter.getInvMassScale1();
			invInertiaScale0 = iter.getInvInertiaScale0();
			invInertiaScale1 = iter.getInvInertiaScale1();
			while(iter.hasNextPatch())
			{
				iter.nextPatch();
				++patchCount;
				while(iter.hasNextContact())
				{
					iter.nextContact();
					Ps::prefetchLine(iter.currPtr, 128);
					Ps::prefetchLine(&buffer.contacts[numContacts], 128);
					PxReal maxImpulse = iter.getMaxImpulse();
					if(maxImpulse != 0.f)
					{
						PX_ASSERT(numContacts < ContactBuffer::MAX_CONTACTS);
						buffer.contacts[numContacts].normal = iter.getContactNormal();
						buffer.contacts[numContacts].point = iter.getContactPoint();
						buffer.contacts[numContacts].separation = iter.getSeparation();
						//KS - we use the face indices to cache the material indices and flags - avoids bloating the PxContact structure
						buffer.contacts[numContacts].internalFaceIndex0 = PxU32(iter.getMaterialIndex0() | (iter.getMaterialIndex1() << 16));
						buffer.contacts[numContacts].internalFaceIndex1 = iter.getMaterialFlags();
						buffer.contacts[numContacts].maxImpulse = maxImpulse;
						buffer.contacts[numContacts].staticFriction = iter.getStaticFriction();
						buffer.contacts[numContacts].dynamicFriction = iter.getDynamicFriction();
						buffer.contacts[numContacts].restitution = iter.getRestitution();
						const PxVec3& targetVel = iter.getTargetVel();
						buffer.contacts[numContacts].targetVel = targetVel;
						perPointFriction |= iter.contactsWereModifiable && !targetVel.isZero();
						++numContacts;
					}
				}
			}

			buffer.count = numContacts;
		}
	}

	if(numContacts == 0)
	{
		n.frictionDataPtr = NULL;
		n.frictionPatchCount = 0;
		n.contactCount = 0;
		desc.constraint = NULL;
		return true;
	}

	Ps::prefetchLine(sBody0);
	Ps::prefetchLine(sBody1);
	Ps::prefetchLine(solverBodyData0);
	Ps::prefetchLine(solverBodyData1);

	PxcCorrelationBuffer& c = threadContext.mCorrelationBuffer;

	c.frictionPatchCount = 0;
	c.contactPatchCount = 0;
	if(!(perPointFriction || n.flags & PxcNpWorkUnitFlag::eDISABLE_STRONG_FRICTION))
	{
		getFrictionPatches(threadContext.mFrictionPatchStreamPair, c,  n.frictionDataPtr, n.frictionPatchCount, *bodyFrame0, *bodyFrame1, correlationDistance);
	}

	bool overflow = !createContactPatches(threadContext, 0, PXC_SAME_NORMAL);		
	overflow = correlatePatches(c, buffer, *bodyFrame0, *bodyFrame1, PXC_SAME_NORMAL, 0, 0) || overflow;
	PX_UNUSED(overflow);

#ifdef __SPU__
	if(overflow)
	{
		overflowed = true;
	}
#else
#ifdef PX_CHECKED
	if(overflow)
	{
		Ps::getFoundation().error(physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
				"Dropping contacts in solver because we exceeded limit of 32 friction patches.");
	}
#endif
#endif
	growPatches(c, buffer, *bodyFrame0, *bodyFrame1, correlationDistance, 0, frictionOffsetThreshold);

	//PX_ASSERT(patchCount == c.frictionPatchCount);

	PxcFrictionPatch* frictionPatches = NULL;
	PxU8* solverConstraint = NULL;
#ifdef __SPU__
	uintptr_t eaFrictionPatches = 0;
	uintptr_t eaSolverConstraint = 0;
#endif
	PxU32 numFrictionPatches = 0;
	PxU32 solverConstraintByteSize = 0;
	PxU32 axisConstraintCount = 0;

	const bool successfulReserve = reserveBlockStreams(
		n, threadContext, forceContacts,
		solverConstraint, frictionPatches,
#ifdef __SPU__
		eaSolverConstraint, eaFrictionPatches,
#endif
		numFrictionPatches,
		solverConstraintByteSize,
		axisConstraintCount,
		constraintBlockManager);
	// initialise the work unit's ptrs to the various buffers.

	n.frictionDataPtr = NULL;
	n.solverConstraintPointer = NULL;
	n.contactCount = 0;
	n.frictionPatchCount = 0;
	n.solverConstraintSize = 0;
	n.axisConstraintCount += Ps::to16(axisConstraintCount);

	// patch up the work unit with the reserved buffers and set the reserved buffer data as appropriate.

	if(successfulReserve)
	{
#ifdef __SPU__
		n.frictionDataPtr = (PxU8*)eaFrictionPatches;
		n.solverConstraintPointer = (PxU8*)eaSolverConstraint;
#else
		n.frictionDataPtr = (PxU8*)frictionPatches;
		n.solverConstraintPointer = solverConstraint;
#endif
		n.contactCount = Ps::to16(numContacts);
		n.frictionPatchCount = numFrictionPatches;
		n.solverConstraintSize = solverConstraintByteSize;

		//Initialise friction buffer.
		if(frictionPatches)
		{
			// PT: TODO: revisit this... not very satisfying
			//const PxU32 maxSize = numFrictionPatches*sizeof(PxcFrictionPatch);
			Ps::prefetchLine(frictionPatches);
			Ps::prefetchLine(frictionPatches, 128);
			Ps::prefetchLine(frictionPatches, 256);

			for(PxU32 i=0;i<c.frictionPatchCount;i++)
			{
				//if(c.correlationListHeads[i]!=PxcCorrelationBuffer::LIST_END)
				if(c.frictionPatchContactCounts[i])
				{
					*frictionPatches++ = c.frictionPatches[i];
					Ps::prefetchLine(frictionPatches, 256);
				}
			}
		}

		//Initialise solverConstraint buffer.
		if(solverConstraint)
		{
			//CM_PROFILE_ZONE(context.getContext()->getEventProfiler(),Cm::ProfileEventId::Sim::setupFinalize);
			Ps::invalidateCache(solverConstraint, (PxI32)solverConstraintByteSize);
			if(n.flags & (PxcNpWorkUnitFlag::eARTICULATION_BODY0|PxcNpWorkUnitFlag::eARTICULATION_BODY1))
			{
				const PxcSolverBodyData& data0 = *solverBodyData0;
				const PxcSolverBodyData& data1 = *solverBodyData1;

				PxsSolverExtBody b0((void*)sBody0, (void*)&data0, desc.linkIndexA);
				PxsSolverExtBody b1((void*)sBody1, (void*)&data1, desc.linkIndexB);

				setupFinalizeExtSolverConstraints(n, buffer, c, *bodyFrame0, *bodyFrame1, perPointFriction, solverConstraint,
							b0, b1, data0, data1, invDtF32, bounceThresholdF32,
							invMassScale0, invInertiaScale0, invMassScale1, invInertiaScale1);
			}
			else
			{
				const PxcSolverBodyData& data0 = *solverBodyData0;
				const PxcSolverBodyData& data1 = *solverBodyData1;
				setupFinalizeSolverConstraints(n, buffer, c, *bodyFrame0, *bodyFrame1, perPointFriction, solverConstraint,
					*sBody0, *sBody1, data0, data1, invDtF32, bounceThresholdF32,
					invMassScale0, invInertiaScale0, invMassScale1, invInertiaScale1);
			}
			//KS - set to 0 so we have a counter for the number of times we solved the constraint
			//only going to be used on SPU but might as well set on all platforms because this code is shared
			*((PxU32*)(solverConstraint + solverConstraintByteSize)) = 0;
		}
	}

	return successfulReserve;
}

PX_FORCE_INLINE void computeBlockStreamFrictionByteSizes(const PxcNpWorkUnit&, PxcNpThreadContext& threadContext,
														 PxU32& _frictionPatchByteSize, PxU32& _numFrictionPatches,
														 PxU32 frictionPatchStartIndex, PxU32 frictionPatchEndIndex)
{
	/*PX_ASSERT(0 == _frictionPatchByteSize);
	PX_ASSERT(0 == _numFrictionPatches);*/

	// PT: use local vars to remove LHS
	PxU32 numFrictionPatches = 0;

	//Unpack the thread context to get the structures that we will need to compute the reservation sizes and make the reservations.
	const PxcCorrelationBuffer& c = threadContext.mCorrelationBuffer;

	for(PxU32 i = frictionPatchStartIndex; i < frictionPatchEndIndex; i++)
	{
		//Friction patches.
		if(c.correlationListHeads[i] != PxcCorrelationBuffer::LIST_END)
			numFrictionPatches++;
	}
	PxU32 frictionPatchByteSize = numFrictionPatches*sizeof(PxcFrictionPatch);

	_numFrictionPatches = numFrictionPatches;

	//16-byte alignment.
	_frictionPatchByteSize = ((frictionPatchByteSize + 0x0f) & ~0x0f);
	PX_ASSERT(0 == (_frictionPatchByteSize & 0x0f));
}

static bool reserveFrictionBlockStreams(const PxcNpWorkUnit& n, PxcNpThreadContext& threadContext, PxU32 frictionPatchStartIndex, PxU32 frictionPatchEndIndex,
						PxcFrictionPatch*& _frictionPatches,
#ifdef __SPU__
						uintptr_t& eaFrictionPatches,
#endif
						PxU32& numFrictionPatches)
{
//	PX_ASSERT(NULL == _frictionPatches);
//#ifdef __SPU__
//	PX_ASSERT(0 == eaFrictionPatches);
//#endif
//	PX_ASSERT(0 == numFrictionPatches);
#ifdef __SPU__
	SpuNpMemBlock& frictionPatchStream = gMemBlockFrictionPatches;
#else
	PxcFrictionPatchStreamPair& frictionPatchStream = threadContext.mFrictionPatchStreamPair;
#endif

	//From frictionPatchStream we just need to reserve a single buffer.
	PxU32 frictionPatchByteSize = 0;
	//Compute the sizes of all the buffers.

	computeBlockStreamFrictionByteSizes(n, threadContext, frictionPatchByteSize, numFrictionPatches, frictionPatchStartIndex, frictionPatchEndIndex);

	PxcFrictionPatch* frictionPatches = NULL;
	//If the constraint block reservation didn't fail then reserve the friction buffer too.
	if(frictionPatchByteSize > 0)
	{
#ifdef __SPU__
		frictionPatches = (PxcFrictionPatch*)frictionPatchStream.reserve(frictionPatchByteSize, &eaFrictionPatches);
#else
		frictionPatches = frictionPatchStream.reserve<PxcFrictionPatch>(frictionPatchByteSize);
#endif
		if(0==frictionPatches || ((PxcFrictionPatch*)(-1))==frictionPatches)
		{
			if(0==frictionPatches)
			{
				PX_WARN_ONCE(true,
					"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for constraint prep. "
					"Either accept dropped contacts or increase buffer size allocated for narrow phase by increasing PxSceneDesc::maxNbContactDataBlocks.");
			}
			else
			{
				PX_WARN_ONCE(true,
					"Attempting to allocate more than 16K of friction data for a single contact pair in constraint prep. "
					"Either accept dropped contacts or simplify collision geometry.");
				frictionPatches=NULL;
			}
		}
	}

	_frictionPatches = frictionPatches;

	//Return true if neither of the two block reservations failed.
	return (0==frictionPatchByteSize || frictionPatches);
}

//The persistent friction patch correlation/allocation will already have happenned as this is per-pair.
//This function just computes the size of the combined solve data.
void computeBlockStreamByteSizes4(PxcSolverContact4Desc* descs,
								PxU32& _solverConstraintByteSize, PxU32* _axisConstraintCount,
								PxcNpThreadContext& threadContext)
{
	PX_ASSERT(0 == _solverConstraintByteSize);

	//Unpack the thread context to get the structures that we will need to compute the reservation sizes and make the reservations.
	const PxcCorrelationBuffer& c = threadContext.mCorrelationBuffer;
	//const PxcContactBuffer& buffer = threadContext.mContactBuffer;
	
	//const bool haveFriction = PX_IR(n.staticFriction) > 0 || PX_IR(n.dynamicFriction) > 0;

	PxU32 maxPatches = 0;
	PxU32 maxFrictionPatches = 0;
	PxU32 maxContactCount[PxcCorrelationBuffer::MAX_FRICTION_PATCHES];
	PxU32 maxFrictionCount[PxcCorrelationBuffer::MAX_FRICTION_PATCHES];
	PxMemZero(maxContactCount, sizeof(maxContactCount));
	PxMemZero(maxFrictionCount, sizeof(maxFrictionCount));
	bool hasMaxImpulse = false;
	bool hasTargetVelocity = false;

	for(PxU32 a = 0; a < 4; ++a)
	{
		PxU32 axisConstraintCount = 0;
		hasMaxImpulse = hasMaxImpulse || descs[a].hasMaxImpulse;
		hasTargetVelocity = hasTargetVelocity || descs[a].hasTargetVelocity;
		for(PxU32 i = 0; i < descs[a].numFrictionPatches; i++)
		{
			PxU32 ind = i + descs[a].startFrictionPatchIndex;

			const PxcFrictionPatch& frictionPatch = c.frictionPatches[ind];

			const bool haveFriction = (frictionPatch.materialFlags & PxMaterialFlag::eDISABLE_FRICTION) == 0
				&& frictionPatch.anchorCount != 0;
			//Solver constraint data.
			if(c.frictionPatchContactCounts[ind]!=0)
			{
				maxContactCount[i] = PxMax(c.frictionPatchContactCounts[ind], maxContactCount[i]);
				axisConstraintCount += c.frictionPatchContactCounts[ind];

				if(haveFriction)
				{
					const PxU32 fricCount = (PxU32)c.frictionPatches[ind].anchorCount * 2;
					maxFrictionCount[i] = PxMax(fricCount, maxFrictionCount[i]);
					axisConstraintCount += fricCount;
				}
			}
		}
		maxPatches = PxMax(descs[a].numFrictionPatches, maxPatches);
		_axisConstraintCount[a] = axisConstraintCount;
	}

	for(PxU32 a = 0; a < maxPatches; ++a)
	{
		if(maxFrictionCount[a] > 0)
			maxFrictionPatches++;
	}


	PxU32 totalContacts = 0, totalFriction = 0;
	for(PxU32 a = 0; a < maxPatches; ++a)
	{
		totalContacts += maxContactCount[a];
		totalFriction += maxFrictionCount[a];
	}

	//OK, we have a given number of friction patches, contact points and friction constraints so we can calculate how much memory we need

	const bool isStatic = (((descs[0].unit->flags | descs[1].unit->flags | descs[2].unit->flags | descs[3].unit->flags) & PxcNpWorkUnitFlag::eDYNAMIC_BODY1) == 0);

	const PxU32 headerSize = sizeof(PxcSolverContactHeader4) * maxPatches + sizeof(PxcSolverFrictionSharedData4) * maxFrictionPatches;
	PxU32 constraintSize = isStatic ? (sizeof(PxcSolverContactBatchPointBase4) * totalContacts) + ( sizeof(PxcSolverContactFrictionBase4) * totalFriction) : 
		(sizeof(PxcSolverContactBatchPointDynamic4) * totalContacts) + (sizeof(PxcSolverContactFrictionDynamic4) * totalFriction);

	//Space for the appliedForce buffer
	constraintSize += sizeof(Vec4V)*(totalContacts+totalFriction);

	//If we have max impulse, reserve a buffer for it
	if(hasMaxImpulse)
		constraintSize += sizeof(Ps::aos::Vec4V) * totalContacts;
	//Else don't :-)
	//If we have target velocities, reserve a buffer for it
	if(hasTargetVelocity)
		constraintSize += sizeof(Ps::aos::Vec4V) * totalFriction;
	//Else don't :-)

	_solverConstraintByteSize =  ((constraintSize + headerSize + 0x0f) & ~0x0f);
	PX_ASSERT(0 == (_solverConstraintByteSize & 0x0f));
}

static PxcSolverConstraintPrepState::Enum reserveBlockStreams4(PxcSolverContact4Desc* descs, PxcNpThreadContext& threadContext,
						PxU8*& solverConstraint, PxU32* axisConstraintCount,
#ifdef __SPU__
						uintptr_t& eaSolverConstraint,
#endif
						PxU32& solverConstraintByteSize, 
						PxsConstraintBlockManager& constraintBlockManager)
{
	PX_ASSERT(NULL == solverConstraint);
#ifdef __SPU__
	PX_ASSERT(0 == eaSolverConstraint);
#endif
	PX_ASSERT(0 == solverConstraintByteSize);
	//PX_ASSERT(0 == axisConstraintCount);

#ifdef __SPU__
	SpuNpMemBlock& constraintBlockStream = gMemBlock;
#else
	PxcConstraintBlockStream& constraintBlockStream = threadContext.mConstraintBlockStream;
#endif

	//Compute the sizes of all the buffers.
	computeBlockStreamByteSizes4(descs, 
		solverConstraintByteSize, axisConstraintCount,
		threadContext);

	//Reserve the buffers.

	//First reserve the accumulated buffer size for the constraint block.
	PxU8* constraintBlock = NULL;
#ifdef __SPU__
	uintptr_t eaConstraintBlock = NULL;
#endif
	const PxU32 constraintBlockByteSize = solverConstraintByteSize;
	if(constraintBlockByteSize > 0)
	{
		if((constraintBlockByteSize + 16u) > 16384)
			return PxcSolverConstraintPrepState::eUNBATCHABLE;

#ifdef __SPU__
		constraintBlock = constraintBlockStream.reserve(constraintBlockByteSize+16u, &eaConstraintBlock);
#else
		constraintBlock = constraintBlockStream.reserve(constraintBlockByteSize+16u, constraintBlockManager);
#endif
		if(0==constraintBlock || ((PxU8*)(-1))==constraintBlock)
		{
			if(0==constraintBlock)
			{
				PX_WARN_ONCE(true,
					"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for narrow phase. "
					"Either accept dropped contacts or increase buffer size allocated for narrow phase by increasing PxSceneDesc::maxNbContactDataBlocks.");
			}
			else
			{
				PX_WARN_ONCE(true,
					"Attempting to allocate more than 16K of contact data for a single contact pair in narrowphase. "
					"Either accept dropped contacts or simplify collision geometry.");
				constraintBlock=NULL;
			}
		}
	}

	//Patch up the individual ptrs to the buffer returned by the constraint block reservation (assuming the reservation didn't fail).
	if(0==constraintBlockByteSize || constraintBlock)
	{
		if(solverConstraintByteSize)
		{
			solverConstraint = constraintBlock;
			PX_ASSERT(0==((uintptr_t)solverConstraint & 0x0f));
#ifdef __SPU__
			eaSolverConstraint = eaConstraintBlock;
#endif
		}
	}
	threadContext.mConstraintSize += (constraintBlockByteSize);

	return ((0==constraintBlockByteSize || constraintBlock)) ? PxcSolverConstraintPrepState::eSUCCESS : PxcSolverConstraintPrepState::eOUT_OF_MEMORY;
}

//This returns 1 of 3 states: success, unbatchable or out-of-memory. If the constraint is unbatchable, we must fall back on 4 separate constraint
//prep calls
PxcSolverConstraintPrepState::Enum physx::createFinalizeSolverContacts4(PxcNpWorkUnit** n,
								 PxcNpThreadContext& threadContext,
								 PxcSolverConstraintDesc** desc,
								 const PxcSolverBody** sBody0,
								 const PxcSolverBody** sBody1,
								 const PxcSolverBodyData** solverBodyData0,
								 const PxcSolverBodyData** solverBodyData1,
								 const PxsRigidCore** body0,
								 const PxsRigidCore** body1,
								 const PxReal invDtF32,
								 PxReal bounceThresholdF32,
								 PxReal	frictionOffsetThreshold,
								 PxReal correlationDistance,
								 PxsConstraintBlockManager& constraintBlockManager,
								 PxFrictionType::Enum frictionType)
{	
	PX_UNUSED(frictionType);
	//CM_PROFILE_ZONE(context.getContext()->getEventProfiler(),Cm::ProfileEventId::Sim::createFinalize);

	for(PxU32 a = 0; a < 4; ++a)
	{
		desc[a]->constraintLengthOver16 = 0;
	}

	PX_ASSERT(n[0]->contactCount && n[1]->contactCount && n[2]->contactCount && n[3]->contactCount);
	PX_ASSERT(!(n[0]->flags & (PxcNpWorkUnitFlag::eARTICULATION_BODY0|PxcNpWorkUnitFlag::eARTICULATION_BODY1))); //KS - we can't batch articulation contacts - not enough space on SPU
	//PxU32 totalContactCount = n[0]->contactCount + n[1]->contactCount + n[2]->contactCount + n[3]->contactCount;

	//This represents each block 
	PxcSolverContact4Desc blockDescs[4];

	Gu::ContactBuffer& buffer = threadContext.mContactBuffer;	

	buffer.count = 0;

	PxU32 numContacts = 0;

	PxTransform idt = PxTransform(PxIdentity);

	PxcCorrelationBuffer& c = threadContext.mCorrelationBuffer;

	c.frictionPatchCount = 0;
	c.contactPatchCount = 0;


	PX_ALIGN(16, PxReal invMassScale0[4]);
	PX_ALIGN(16, PxReal invMassScale1[4]);
	PX_ALIGN(16, PxReal invInertiaScale0[4]);
	PX_ALIGN(16, PxReal invInertiaScale1[4]);

	for(PxU32 a = 0; a < 4; ++a)
	{
		PxcSolverContact4Desc& blockDesc = blockDescs[a];
		if(n[a]->flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY0)
		{
			Ps::prefetchLine(&body0[a]->body2World);
			blockDesc.bodyFrame0 = &body0[a]->body2World;
		}
		else
		{
			blockDesc.bodyFrame0 = &idt;
		}

		if(n[a]->flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1)
		{
			Ps::prefetchLine(&body1[a]->body2World);
			blockDesc.bodyFrame1 = &body1[a]->body2World;
		}
		else
		{
			blockDesc.bodyFrame1 = &idt;
		}
		
		blockDesc.unit = n[a];
		blockDesc.startContactIndex = numContacts;
		blockDesc.b0 = sBody0[a];
		blockDesc.b1 = sBody1[a];
		blockDesc.data0 = solverBodyData0[a];
		blockDesc.data1 = solverBodyData1[a];

		Ps::prefetchLine(sBody0[a]);
		Ps::prefetchLine(sBody1[a]);
		Ps::prefetchLine(solverBodyData0[a]);
		Ps::prefetchLine(solverBodyData1[a]);

		
		if((numContacts + n[a]->contactCount) > 64)
		{
			return PxcSolverConstraintPrepState::eUNBATCHABLE;
		}

		bool perPointFriction = false;
		PxContactStreamIterator iter(n[a]->compressedContacts, n[a]->compressedContactSize);	

		bool hasMaxImpulse = false;		
		bool hasTargetVelocity = false;
		if(!iter.forceNoResponse)
		{
			invMassScale0[a] = iter.getInvMassScale0();
			invMassScale1[a] = iter.getInvMassScale1();
			invInertiaScale0[a] = iter.getInvInertiaScale0();
			invInertiaScale1[a] = iter.getInvInertiaScale1();
			hasMaxImpulse = (iter.header->flags & PxContactHeader::eHAS_MAX_IMPULSE) != 0;
			hasTargetVelocity = (iter.header->flags & PxContactHeader::eHAS_TARGET_VELOCITY) != 0;

			while(iter.hasNextPatch())
			{
				iter.nextPatch();
				while(iter.hasNextContact())
				{
					iter.nextContact();
					Ps::prefetchLine(iter.currPtr, 128);
					Ps::prefetchLine(&buffer.contacts[numContacts], 128);
					PxReal maxImpulse = iter.getMaxImpulse();
					if(maxImpulse != 0.f)
					{
						PX_ASSERT(numContacts < Gu::ContactBuffer::MAX_CONTACTS);
						buffer.contacts[numContacts].normal = iter.getContactNormal();
						buffer.contacts[numContacts].point = iter.getContactPoint();
						buffer.contacts[numContacts].separation = iter.getSeparation();
						//KS - we use the face indices to cache the material indices and flags - avoids bloating the PxContact structure
						buffer.contacts[numContacts].internalFaceIndex0 = PxU32(iter.getMaterialIndex0() | (iter.getMaterialIndex1() << 16));
						buffer.contacts[numContacts].internalFaceIndex1 = iter.getMaterialFlags();
						buffer.contacts[numContacts].maxImpulse = maxImpulse;
						buffer.contacts[numContacts].staticFriction = iter.getStaticFriction();
						buffer.contacts[numContacts].dynamicFriction = iter.getDynamicFriction();
						buffer.contacts[numContacts].restitution = iter.getRestitution();
						const PxVec3& targetVel = iter.getTargetVel();
						buffer.contacts[numContacts].targetVel = targetVel;
						perPointFriction |= iter.contactsWereModifiable && !targetVel.isZero();
						++numContacts;
					}
				}
			}
		}
		const PxU32 contactCount = numContacts - blockDesc.startContactIndex;
		buffer.count = numContacts;

		if(contactCount == 0)
			return PxcSolverConstraintPrepState::eUNBATCHABLE;

		blockDesc.numContacts = contactCount;
		blockDesc.perPointFriction = perPointFriction;
		blockDesc.hasMaxImpulse = hasMaxImpulse;
		blockDesc.hasTargetVelocity = hasTargetVelocity;
		

		//OK...do the correlation here as well...
		Ps::prefetchLine(n[a]->frictionDataPtr);
	
		blockDesc.startFrictionPatchIndex = c.frictionPatchCount;
		if(!(perPointFriction || n[a]->flags & PxcNpWorkUnitFlag::eDISABLE_STRONG_FRICTION))
		{
			bool valid = getFrictionPatches(threadContext.mFrictionPatchStreamPair, c,  n[a]->frictionDataPtr, n[a]->frictionPatchCount, 
				*blockDesc.bodyFrame0, *blockDesc.bodyFrame1, correlationDistance);
			if(!valid)
				return PxcSolverConstraintPrepState::eUNBATCHABLE;
		}
		//Create the contact patches
		blockDesc.startContactPatchIndex = threadContext.mCorrelationBuffer.contactPatchCount;
		if(!createContactPatches(threadContext, blockDesc.startContactIndex, PXC_SAME_NORMAL))
			return PxcSolverConstraintPrepState::eUNBATCHABLE;
		blockDesc.numContactPatches = threadContext.mCorrelationBuffer.contactPatchCount - blockDesc.startContactPatchIndex;

		bool overflow = correlatePatches(c, buffer, *blockDesc.bodyFrame0, *blockDesc.bodyFrame1, PXC_SAME_NORMAL, 
			blockDesc.startContactPatchIndex, blockDesc.startFrictionPatchIndex);

		if(overflow)
			return PxcSolverConstraintPrepState::eUNBATCHABLE;

		growPatches(c, buffer, *blockDesc.bodyFrame0, *blockDesc.bodyFrame1, correlationDistance, blockDesc.startFrictionPatchIndex,
			frictionOffsetThreshold);

		//Remove the empty friction patches - do we actually need to do this?
		for(PxU32 p = c.frictionPatchCount; p > blockDesc.startFrictionPatchIndex; --p)
		{
			if(c.correlationListHeads[p-1] == 0xffff)
			{
				//We have an empty patch...need to bin this one...
				for(PxU32 p2 = p; p2 < c.frictionPatchCount; ++p2)
				{
					c.correlationListHeads[p2-1] = c.correlationListHeads[p2];
					c.frictionPatchContactCounts[p2-1] = c.frictionPatchContactCounts[p2];
				}
				c.frictionPatchCount--;
			}
		}

		PxU32 numFricPatches = c.frictionPatchCount - blockDesc.startFrictionPatchIndex;
		blockDesc.numFrictionPatches = numFricPatches;
	}

	PxcFrictionPatch* frictionPatchArray[4];
	PxU32 frictionPatchCounts[4];
#ifdef __SPU__
	uintptr_t eaFrictionPatches[4];
#endif


	for(PxU32 a = 0; a < 4; ++a)
	{
		PxcSolverContact4Desc& blockDesc = blockDescs[a];

		const bool successfulReserve =  reserveFrictionBlockStreams(*n[a], threadContext, blockDesc.startFrictionPatchIndex, blockDesc.numFrictionPatches + blockDesc.startFrictionPatchIndex,
						frictionPatchArray[a],
#ifdef __SPU__
						eaFrictionPatches[a],
#endif
						frictionPatchCounts[a]);

		//KS - TODO - how can we recover if we failed to allocate this memory?
		if(!successfulReserve)
		{
			return PxcSolverConstraintPrepState::eOUT_OF_MEMORY;
		}
	}
	//At this point, all the friction data has been calculated, the correlation has been done. Provided this was all successful, 
	//we are ready to create the batched constraints

	PxU8* solverConstraint = NULL;
#ifdef __SPU__
	uintptr_t eaSolverConstraint = 0;
#endif
	PxU32 solverConstraintByteSize = 0;

	

	{
		PxU32 axisConstraintCount[4];
		PxcSolverConstraintPrepState::Enum state = reserveBlockStreams4(blockDescs, threadContext,
						solverConstraint, axisConstraintCount,
#ifdef __SPU__
						eaSolverConstraint,
#endif
						solverConstraintByteSize, 
						constraintBlockManager);

		if(state != PxcSolverConstraintPrepState::eSUCCESS)
			return state;


		for(PxU32 a = 0; a < 4; ++a)
		{

			PxcFrictionPatch* frictionPatches = frictionPatchArray[a];

			PxcSolverContact4Desc& blockDesc = blockDescs[a];
#ifdef __SPU__
			n[a]->frictionDataPtr = (PxU8*)eaFrictionPatches[a];
#else
			n[a]->frictionDataPtr = (PxU8*)frictionPatches;
#endif
			n[a]->frictionPatchCount = frictionPatchCounts[a];
			
			//Initialise friction buffer.
			if(frictionPatches)
			{
				// PT: TODO: revisit this... not very satisfying
				//const PxU32 maxSize = numFrictionPatches*sizeof(PxcFrictionPatch);
				Ps::prefetchLine(frictionPatches);
				Ps::prefetchLine(frictionPatches, 128);
				Ps::prefetchLine(frictionPatches, 256);

				for(PxU32 i=0;i<blockDesc.numFrictionPatches;i++)
				{
					if(c.correlationListHeads[blockDesc.startFrictionPatchIndex + i]!=PxcCorrelationBuffer::LIST_END)
					{
						*frictionPatches++ = c.frictionPatches[blockDesc.startFrictionPatchIndex + i];
						Ps::prefetchLine(frictionPatches, 256);
					}
				}
			}

			
#ifdef __SPU__
			n[a]->solverConstraintPointer = (PxU8*)eaSolverConstraint;
#else
			n[a]->solverConstraintPointer = solverConstraint;
#endif

			n[a]->solverConstraintSize = solverConstraintByteSize;
			n[a]->axisConstraintCount += Ps::to16(axisConstraintCount[a]);
#ifdef __SPU__
			desc[a]->constraint = (PxU8*)eaSolverConstraint;
#else
			desc[a]->constraint = solverConstraint;
#endif
			desc[a]->constraintLengthOver16 = Ps::to16(solverConstraintByteSize/16);
			PxU32 writeBackLength;
			void* writeBack = PxcNpWorkUnitGetContactForcesDC_Safe(*n[a], writeBackLength);
			desc[a]->writeBack = writeBack;
			setWritebackLength(*desc[a], writeBackLength);
		}

		const Vec4V iMassScale0 = V4LoadA(invMassScale0); 
		const Vec4V iInertiaScale0 = V4LoadA(invInertiaScale0);
		const Vec4V iMassScale1 = V4LoadA(invMassScale1); 
		const Vec4V iInertiaScale1 = V4LoadA(invInertiaScale1);

		setupFinalizeSolverConstraints4(blockDescs, threadContext, solverConstraint, invDtF32, bounceThresholdF32,
			iMassScale0, iInertiaScale0, iMassScale1, iInertiaScale1);

		PX_ASSERT((*solverConstraint == PXS_SC_TYPE_BLOCK_RB_CONTACT) || (*solverConstraint == PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT));

		*((PxU32*)(solverConstraint + solverConstraintByteSize)) = 0;
	}
	return PxcSolverConstraintPrepState::eSUCCESS;
}


PxU32 physx::writeCompressedContact(const Gu::ContactPoint* const PX_RESTRICT contactPoints, const PxU32 numContactPoints, PxcNpThreadContext& threadContext,
									PxU16& writtenContactCount, PxU8*& compressedContacts, PxU32& compressedContactSize, PxReal*& contactForces, PxU32 contactForceByteSize,
									PxU16& statusFlag, const PxsMaterialManager* materialManager, bool hasModifiableContacts, bool forceNoResponse, PxsMaterialInfo* PX_RESTRICT pMaterial,
									PxU32 additionalHeaderSize, bool useConstraintData, PxcNpThreadContext* islandContext, bool insertAveragePoint)
{
#if !PX_ENABLE_AVG_POINT
	PX_UNUSED(insertAveragePoint);
#endif
	if(numContactPoints == 0)
	{
		writtenContactCount = 0;
		compressedContacts = NULL;
		compressedContactSize = 0;
		contactForces = NULL;
		statusFlag &= ~PxcNpWorkUnitStatusFlag::eHAS_CONTACT_FORCES;
		return 0;
	}
	//Calculate the size of the contact buffer...
	PX_ALLOCA(strHeader, PxU16, numContactPoints);
	PxU16* strideHeader = &strHeader[0];
	PxU32 numStrideHeaders = 1;

	const bool hasInternalFaceIndex = contactPoints[0].internalFaceIndex0 != PXC_CONTACT_NO_FACE_INDEX ||
										contactPoints[0].internalFaceIndex1 != PXC_CONTACT_NO_FACE_INDEX;
	const bool isModifiable = !forceNoResponse && hasModifiableContacts;

	const PxReal closeNormalThresh = PXC_SAME_NORMAL;
	//Go through and tag how many patches we have...
	PxVec3 normal = contactPoints[0].normal;
	PxU16 mat0 = pMaterial[0].mMaterialIndex0;
	PxU16 mat1 = pMaterial[0].mMaterialIndex1;

	PxU32 totalContactPoints = numContactPoints;
#if !PX_ENABLE_AVG_POINT
	if(!isModifiable)
#endif
	{
		PxU32 strideStart = 0;
		for(PxU32 a = 1; a < numContactPoints; ++a)
		{
			if(normal.dot(contactPoints[a].normal) < closeNormalThresh || 
				pMaterial[a].mMaterialIndex0 != mat0 || pMaterial[a].mMaterialIndex1 != mat1)
			{
				normal = contactPoints[a].normal;
				strideHeader[numStrideHeaders-1] = (PxU16)a;
				mat0 = pMaterial[a].mMaterialIndex0;
				mat1 = pMaterial[a].mMaterialIndex1;
#if PX_ENABLE_AVG_POINT
				totalContactPoints = insertAveragePoint && (a - strideStart) > 1 ? totalContactPoints + 1 : totalContactPoints;
#endif
				strideStart = a;
				numStrideHeaders++;
			}
		}
#if PX_ENABLE_AVG_POINT
		totalContactPoints = insertAveragePoint &&(numContactPoints - strideStart) > 1 ? totalContactPoints + 1 : totalContactPoints;
		contactForceByteSize = insertAveragePoint && contactForceByteSize != 0 ? contactForceByteSize + sizeof(PxF32) * (totalContactPoints - numContactPoints) : contactForceByteSize;
#endif
	}

	strideHeader[numStrideHeaders - 1] = (PxU16)numContactPoints;

	//Calculate the number of patches/points required

	const PxU32 patchHeaderSize = isModifiable ? sizeof(PxContactPatchBase) : numStrideHeaders * sizeof(PxContactPatch);
	const PxU32 pointSize = totalContactPoints * (isModifiable ? sizeof(PxModifiableContact) : hasInternalFaceIndex ? sizeof(PxFeatureContact) :	sizeof(PxSimpleContact));
	const PxU32 headerSize = (isModifiable ? sizeof(PxModifyContactHeader) : sizeof(PxContactHeader)) + additionalHeaderSize;

	PxU32 requiredSize = headerSize + patchHeaderSize + pointSize;
	PxU32 alignedRequiredSize = (requiredSize + 0xf) & 0xfffffff0;
	PxU32 totalSize = alignedRequiredSize + contactForceByteSize;

#ifdef __SPU__
	uintptr_t eaCompressedBuffer = 0;
	PxU8* PX_RESTRICT data = gMemBlock.reserve(totalSize, &eaCompressedBuffer);
	compressedContacts = (PxU8* PX_RESTRICT)eaCompressedBuffer;
#else
	PxU8* PX_RESTRICT data = useConstraintData ? threadContext.mConstraintBlockStream.reserve(totalSize, islandContext->mConstraintBlockManager) : 
		threadContext.mContactBlockStream.reserve(totalSize);
	compressedContacts = data;
#endif
	Ps::prefetchLine(data);
	Ps::prefetchLine(data,128);

	if(data == NULL)
	{
		writtenContactCount = 0;
		compressedContacts = NULL;
		compressedContactSize = 0;
		contactForces = NULL;
		statusFlag &= ~PxcNpWorkUnitStatusFlag::eHAS_CONTACT_FORCES;
		return 0;
	}

	threadContext.mCompressedCacheSize += requiredSize;
	compressedContactSize = requiredSize;

	PxU8* PX_RESTRICT currPtr = NULL;

	PxU32 startIndex = 0;

	//Extract first material
	PxU16 origMatIndex0 = pMaterial[startIndex].mMaterialIndex0;
	PxU16 origMatIndex1 = pMaterial[startIndex].mMaterialIndex1;

	PxReal staticFriction, dynamicFriction, combinedRestitution;
	PxU32 flags;
	{
		const PxsMaterialData& data0 = *materialManager->getMaterial(origMatIndex0);
		const PxsMaterialData& data1 = *materialManager->getMaterial(origMatIndex1);

		combinedRestitution = PxsMaterialCombiner::combineRestitution(data0, data1);
		PxsMaterialCombiner combiner(1.0f, 1.0f);
		PxsMaterialCombiner::PxsCombinedMaterial combinedMat = combiner.combineIsotropicFriction(data0, data1);
		staticFriction  = combinedMat.staFriction;
		dynamicFriction = combinedMat.dynFriction;
		flags = combinedMat.flags;
	}


	PxU8* PX_RESTRICT dataPlusOffset = data + additionalHeaderSize;
	if(isModifiable)
	{
		PxModifyContactHeader* PX_RESTRICT header = (PxModifyContactHeader*)dataPlusOffset;
		header->flags = PxU16((hasInternalFaceIndex ? PxContactHeader::eHAS_FACE_INDICES : 0) | (isModifiable ? PxContactHeader::eMODIFIABLE : 0) |
			(forceNoResponse ? PxContactHeader::eFORCE_NO_RESPONSE : 0));
		header->totalContactCount = (PxU16)totalContactPoints;
		header->invMassScale0 = 1.f;
		header->invMassScale1 = 1.f;
		header->invInertiaScale0 = 1.f;
		header->invInertiaScale1 = 1.f;

		currPtr = dataPlusOffset + sizeof(PxModifyContactHeader);		

		PxContactPatchBase* PX_RESTRICT patch = (PxContactPatchBase*)currPtr;
		currPtr += sizeof(PxContactPatchBase);
		Ps::prefetchLine(currPtr, 128);
		//
		
		PxModifiableContact* PX_RESTRICT point = (PxModifiableContact*)currPtr;

	

		for(PxU32 a = 0; a < numStrideHeaders; ++a)
		{

			const PxU16 matIndex0 = pMaterial[startIndex].mMaterialIndex0;
			const PxU16 matIndex1 = pMaterial[startIndex].mMaterialIndex1;
			if(matIndex0 != origMatIndex0 || matIndex1 != origMatIndex1)
			{
				const PxsMaterialData& data0 = *materialManager->getMaterial(matIndex0);
				const PxsMaterialData& data1 = *materialManager->getMaterial(matIndex1);

				combinedRestitution = PxsMaterialCombiner::combineRestitution(data0, data1);
				PxsMaterialCombiner combiner(1.0f, 1.0f);
				PxsMaterialCombiner::PxsCombinedMaterial combinedMat = combiner.combineIsotropicFriction(data0, data1);
				staticFriction = combinedMat.staFriction;
				dynamicFriction = combinedMat.dynFriction;
				flags = combinedMat.flags;
				origMatIndex0 = matIndex0;
				origMatIndex1 = matIndex1;
			}

			patch->nbContacts = Ps::to16(totalContactPoints);
			patch->flags = Ps::to16(flags);

			const PxU32 endIndex = strideHeader[a];
#if PX_ENABLE_AVG_POINT
			if(insertAveragePoint && (endIndex - startIndex) > 1)
			{
				//patch->nbContacts++;
				PxVec3 avgPt(0.f);
				PxF32 avgPen(0.f);
				PxF32 recipCount = 1.f/((PxF32)(endIndex-startIndex));
				for(PxU32 b = startIndex; b < endIndex; ++b)
				{
					avgPt += contactPoints[b].point;
					avgPen += contactPoints[b].separation;
				}

				point->contact = avgPt * recipCount;
				point->separation = avgPen * recipCount;
				point->internalFaceIndex0 = contactPoints[0].internalFaceIndex0;
				point->internalFaceIndex1 = contactPoints[0].internalFaceIndex1;
				point->normal = contactPoints[0].normal;
				point->maxImpulse = PX_MAX_REAL;
				point->targetVel = PxVec3(0.f);
				point->staticFriction = staticFriction;
				point->dynamicFriction = dynamicFriction;
				point->restitution = combinedRestitution;
				point->flags = flags;
				point->materialIndex0 = matIndex0;
				point->materialIndex1 = matIndex1;
				point++;
				Ps::prefetchLine(point, 128);
			}
#endif

			for(PxU32 b = startIndex; b < endIndex; ++b)
			{
				point->contact = contactPoints[b].point;
				point->separation = contactPoints[b].separation;
				point->internalFaceIndex0 = contactPoints[b].internalFaceIndex0;
				point->internalFaceIndex1 = contactPoints[b].internalFaceIndex1;
				point->normal = contactPoints[b].normal;
				point->maxImpulse = PX_MAX_REAL;
				point->targetVel = PxVec3(0.f);
				point->staticFriction = staticFriction;
				point->dynamicFriction = dynamicFriction;
				point->restitution = combinedRestitution;
				point->flags = flags;
				point->materialIndex0 = matIndex0;
				point->materialIndex1 = matIndex1;
				point++;
				Ps::prefetchLine(point, 128);
			}
			currPtr = (PxU8*)point;
			startIndex = strideHeader[a];
		}
	}
	else 
	{
		PxContactHeader* PX_RESTRICT header = (PxContactHeader*)dataPlusOffset;
		header->flags = PxU16((hasInternalFaceIndex ? PxContactHeader::eHAS_FACE_INDICES : 0) | (isModifiable ? PxContactHeader::eMODIFIABLE : 0) |
			(forceNoResponse ? PxContactHeader::eFORCE_NO_RESPONSE : 0));
		header->totalContactCount = (PxU16)totalContactPoints;
		currPtr = dataPlusOffset + sizeof(PxContactHeader);
		if(hasInternalFaceIndex)
		{
			for(PxU32 a = 0; a < numStrideHeaders; ++a)
			{
				const PxU16 matIndex0 = pMaterial[startIndex].mMaterialIndex0;
				const PxU16 matIndex1 = pMaterial[startIndex].mMaterialIndex1;
				if(matIndex0 != origMatIndex0 || matIndex1 != origMatIndex1)
				{
					const PxsMaterialData& data0 = *materialManager->getMaterial(matIndex0);
					const PxsMaterialData& data1 = *materialManager->getMaterial(matIndex1);

					combinedRestitution = PxsMaterialCombiner::combineRestitution(data0, data1);
					PxsMaterialCombiner combiner(1.0f, 1.0f);
					PxsMaterialCombiner::PxsCombinedMaterial combinedMat = combiner.combineIsotropicFriction(data0, data1);
					staticFriction = combinedMat.staFriction;
					dynamicFriction = combinedMat.dynFriction;
					flags = combinedMat.flags;
					origMatIndex0 = matIndex0;
					origMatIndex1 = matIndex1;
				}

				PxContactPatch* PX_RESTRICT patch = (PxContactPatch*)currPtr;
				currPtr += sizeof(PxContactPatch);
				Ps::prefetchLine(currPtr, 128);
				const PxU32 endIndex = strideHeader[a];
				patch->normal = contactPoints[startIndex].normal;
				patch->nbContacts = Ps::to16(endIndex - startIndex);
				patch->staticFriction = staticFriction;
				patch->dynamicFriction = dynamicFriction;
				patch->restitution = combinedRestitution;
				patch->flags = Ps::to16(flags);
				patch->materialIndex0 = matIndex0;
				patch->materialIndex1 = matIndex1;
				PxFeatureContact* PX_RESTRICT point = (PxFeatureContact*)currPtr;
#if PX_ENABLE_AVG_POINT
				if(insertAveragePoint && (endIndex - startIndex) > 1)
				{
					patch->nbContacts++;
					PxVec3 avgPt(0.f);
					PxF32 avgPen(0.f);
					PxF32 recipCount = 1.f/((PxF32)(endIndex-startIndex));
					for(PxU32 b = startIndex; b < endIndex; ++b)
					{
						avgPt += contactPoints[b].point;
						avgPen += contactPoints[b].separation;
					}

					point->contact = avgPt * recipCount;
					point->separation = avgPen * recipCount;
					point->internalFaceIndex0 = contactPoints[0].internalFaceIndex0;
					point->internalFaceIndex1 = contactPoints[0].internalFaceIndex1;
					point++;
					Ps::prefetchLine(point, 128);
				}
#endif
				for(PxU32 b = startIndex; b < endIndex; ++b)
				{
					point->contact = contactPoints[b].point;
					point->separation = contactPoints[b].separation;
					point->internalFaceIndex0 = contactPoints[b].internalFaceIndex0;
					point->internalFaceIndex1 = contactPoints[b].internalFaceIndex1;
					point++;
					Ps::prefetchLine(point, 128);
				}
				currPtr = (PxU8*)point;
				startIndex = strideHeader[a];
			}
		}
		else
		{
			for(PxU32 a = 0; a < numStrideHeaders; ++a)
			{
				const PxU16 matIndex0 = pMaterial[startIndex].mMaterialIndex0;
				const PxU16 matIndex1 = pMaterial[startIndex].mMaterialIndex1;
				if(matIndex0 != origMatIndex0 || matIndex1 != origMatIndex1)
				{
					const PxsMaterialData& data0 = *materialManager->getMaterial(matIndex0);
					const PxsMaterialData& data1 = *materialManager->getMaterial(matIndex1);

					combinedRestitution = PxsMaterialCombiner::combineRestitution(data0, data1);
					PxsMaterialCombiner combiner(1.0f, 1.0f);
					PxsMaterialCombiner::PxsCombinedMaterial combinedMat = combiner.combineIsotropicFriction(data0, data1);
					staticFriction = combinedMat.staFriction;
					dynamicFriction = combinedMat.dynFriction;
					flags = combinedMat.flags;
					origMatIndex0 = matIndex0;
					origMatIndex1 = matIndex1;
				}

				PxContactPatch* PX_RESTRICT patch = (PxContactPatch*)currPtr;
				currPtr += sizeof(PxContactPatch);
				Ps::prefetchLine(currPtr, 128);
				const PxU32 endIndex = strideHeader[a];
				patch->normal = contactPoints[startIndex].normal;
				patch->nbContacts = Ps::to16(endIndex - startIndex);
				//KS - we could probably compress this further into the header but the complexity might not be worth it
				patch->staticFriction = staticFriction;
				patch->dynamicFriction = dynamicFriction;
				patch->restitution = combinedRestitution;
				patch->materialIndex0 = matIndex0;
				patch->materialIndex1 = matIndex1;
				patch->flags = Ps::to16(flags);
				PxSimpleContact* PX_RESTRICT point = (PxSimpleContact*)currPtr;
#if PX_ENABLE_AVG_POINT
				if(insertAveragePoint && (endIndex - startIndex) > 1)
				{
					patch->nbContacts++;
					PxVec3 avgPt(0.f);
					PxF32 avgPen(0.f);
					PxF32 recipCount = 1.f/((PxF32)(endIndex-startIndex));
					for(PxU32 b = startIndex; b < endIndex; ++b)
					{
						avgPt += contactPoints[b].point;
						avgPen += contactPoints[b].separation;
					}

					point->contact = avgPt * recipCount;
					point->separation = avgPen * recipCount;
					point++;
					Ps::prefetchLine(point, 128);
				}
#endif
				for(PxU32 b = startIndex; b < endIndex; ++b)
				{
					point->contact = contactPoints[b].point;
					point->separation = contactPoints[b].separation;
					point++;
					Ps::prefetchLine(point, 128);
				}
				currPtr = (PxU8*)point;
				startIndex = strideHeader[a];
			}
		}

	}
	PX_ASSERT(PxU32(currPtr - data) == requiredSize);

	if(contactForceByteSize)
	{
		PxU8* forces = (data + alignedRequiredSize);
		statusFlag |= PxcNpWorkUnitStatusFlag::eHAS_CONTACT_FORCES;
#ifdef __SPU__
		contactForces = (PxReal*)((PxU8* PX_RESTRICT)eaCompressedBuffer + alignedRequiredSize);
#else
		contactForces = (PxReal*)forces;
#endif
		PxMemZero(forces, contactForceByteSize);
	}

	writtenContactCount = (PxU16)totalContactPoints;

	return requiredSize;
}


bool physx::finishContacts(PxcNpWorkUnit& n, PxcNpThreadContext& threadContext, PxsMaterialInfo* PX_RESTRICT pMaterials)
{
	ContactBuffer& buffer = threadContext.mContactBuffer;

	PX_ASSERT((n.statusFlags & PxcNpWorkUnitStatusFlag::eTOUCH_KNOWN) != PxcNpWorkUnitStatusFlag::eTOUCH_KNOWN);
	PxU16 statusFlags = (PxU16)(n.statusFlags & (~(PxcNpWorkUnitStatusFlag::eTOUCH_KNOWN | PxcNpWorkUnitStatusFlag::eHAS_CONTACT_FORCES)));
	const PxU16 flags = n.flags;	// PT: don't read the flags *after* touching the 'touch' member, as it creates an LHS
	if (buffer.count != 0)
		statusFlags |= PxcNpWorkUnitStatusFlag::eHAS_TOUCH;
	else
		statusFlags |= PxcNpWorkUnitStatusFlag::eHAS_NO_TOUCH;

	if(n.frictionDataPtr==0 && buffer.count==0)
	{
		n.statusFlags = statusFlags;
		return true;
	}
	
	n.contactCount = Ps::to16(buffer.count);

	if (((flags & (PxcNpWorkUnitFlag::eOUTPUT_CONSTRAINTS | PxcNpWorkUnitFlag::eMODIFIABLE_CONTACT)) != 0) && buffer.count > 0)
		statusFlags |= PxcNpWorkUnitStatusFlag::eHAS_SOLVER_CONSTRAINTS;
	else
		statusFlags &= ~PxcNpWorkUnitStatusFlag::eHAS_SOLVER_CONSTRAINTS;

	n.statusFlags = statusFlags;
	n.frictionDataPtr = buffer.count == 0 ? NULL : n.frictionDataPtr;

	PxU32 contactForceByteSize = buffer.count * sizeof(PxReal);

	const bool createReports = 
		n.flags & PxcNpWorkUnitFlag::eOUTPUT_CONTACTS
		|| threadContext.mCreateContactStream
		|| (n.flags & PxcNpWorkUnitFlag::eFORCE_THRESHOLD
		&& ((n.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY0 && PX_IR(static_cast<const PxsBodyCore*>(n.rigidCore0)->contactReportThreshold) != 0x7f7fffff)	// PX_MAX_REAL
		|| (n.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1 && PX_IR(static_cast<const PxsBodyCore*>(n.rigidCore1)->contactReportThreshold) != 0x7f7fffff)));	// PX_MAX_REAL

	if(!buffer.count || !createReports)
	{
		contactForceByteSize = 0;
	}

	//Regardless of the flags, we need to now record the compressed contact stream
	PxReal* contactForces;
	return (writeCompressedContact(buffer.contacts, buffer.count, threadContext, n.contactCount, n.compressedContacts, n.compressedContactSize,
									contactForces, contactForceByteSize, n.statusFlags, n.materialManager, ((n.flags & PxcNpWorkUnitFlag::eMODIFIABLE_CONTACT) != 0), 
									false, pMaterials, 0, false, NULL, threadContext.mCreateAveragePoint) != 0) || (buffer.count == 0);
}
  
