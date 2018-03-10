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
#include "PsMathUtils.h"
#include "PxcSolverContact.h"
#include "PxcSolverContactPF.h"
#include "PxcSolverConstraintTypes.h"
#include "PxcNpWorkUnit.h"
#include "PxcNpThreadContext.h"
#include "PxcNpContactPrep.h"
#include "PxvGeometry.h"
#include "PxvDynamics.h"
#include "PxcCorrelationBuffer.h"
#include "PxcSolverConstraintDesc.h"
#include "PxcSolverBody.h"
#include "PxcSolverContact4.h"
#include "PxcSolverContactPF4.h"


#include "PsVecMath.h"
#include "PxContactModifyCallback.h"
#include "PxsMaterialManager.h"
#include "PxsMaterialCombiner.h"
#include "PxsSolverExt.h"
#include "PxsSolverContact.h"
#include "PxsSolverConstraintExt.h"

using namespace physx;
using namespace Gu;
using namespace Ps::aos;

#ifdef __SPU__
#include "..\..\ps3\include\spu\SpuNpMemBlock.h"
#include "PxsResourceManager.h"

namespace physx
{
	extern SpuNpMemBlock gMemBlock;
	extern SpuNpMemBlock gMemBlockFrictionPatches;
}
#endif


static void computeBlockStreamByteSizesCoulomb(const PxcNpWorkUnit& n, PxcNpThreadContext& threadContext, const PxcCorrelationBufferCoulomb& c,
													 const bool forceContacts, PxU32& _solverConstraintByteSize,
													 PxU32& _axisConstraintCount)
{
	PX_ASSERT(0 == _solverConstraintByteSize);
	PX_ASSERT(0 == _axisConstraintCount);
	PX_UNUSED(forceContacts);
	PX_UNUSED(threadContext);

	// PT: use local vars to remove LHS
	PxU32 solverConstraintByteSize = 0;
	PxU32 numFrictionPatches = 0;
	PxU32 axisConstraintCount = 0;

	//Unpack the thread context to get the structures that we will need to compute the reservation sizes and make the reservations.
	//const PxcCorrelationBuffer& c = threadContext.mCorrelationBuffer;
	//const PxcCorrelationBufferCoulomb& c = threadContext.mCorrelationBufferPointFriction;
	//const ContactBuffer& buffer = threadContext.mContactBuffer;

	//We can compute contactForceByteSize and frictionPatchBytByteSize and solverConstraintSize in the following loop.
	const bool useExtContacts = (n.flags & (PxcNpWorkUnitFlag::eARTICULATION_BODY0|PxcNpWorkUnitFlag::eARTICULATION_BODY1)) != 0;
	//const bool haveFriction = PX_IR(n.staticFriction) > 0 || PX_IR(n.dynamicFriction) > 0;

	for(PxU32 i = 0; i < c.frictionPatchCount; i++)
	{
		//Friction patches.
		if(c.correlationListHeads[i] != PxcCorrelationBuffer::LIST_END)
			numFrictionPatches++;


		const PxcFrictionPatchCoulomb& frictionPatch = c.frictionPatches[i];
		const bool haveFriction = (frictionPatch.materialFlags & PxMaterialFlag::eDISABLE_FRICTION) == 0;

		//Solver constraint data.
		if(c.frictionPatchContactCounts[i]!=0)
		{
			solverConstraintByteSize += sizeof(PxcSolverContactCoulombHeader);
			
			solverConstraintByteSize += useExtContacts ? c.frictionPatchContactCounts[i] * sizeof(PxcSolverContactExt) 
				: c.frictionPatchContactCounts[i] * sizeof(PxcSolverContact);

			axisConstraintCount += c.frictionPatchContactCounts[i];

			//We always need the friction headers to write the accumulated 
			if(haveFriction)
			{
				//4 bytes
				solverConstraintByteSize += sizeof(PxcSolverFrictionHeader);
				//buffer to store applied forces in
				//const PxU32 paddedFrictionPatchCount =((4 * ((numNormalConstr + 3)/4));//(4 * ((c.frictionPatchContactCounts[i] + 4)/4)) - 1;
				//solverConstraintByteSize += sizeof(PxF32) * paddedFrictionPatchCount; //4 bytes * number of contacts (padded to make multiple of 16
				solverConstraintByteSize += PxcSolverFrictionHeader::getAppliedForcePaddingSize(c.frictionPatchContactCounts[i]);

				solverConstraintByteSize += useExtContacts ? c.frictionPatches[i].numConstraints * sizeof(PxcSolverFrictionExt)
					: c.frictionPatches[i].numConstraints * sizeof(PxcSolverFriction);
				axisConstraintCount += c.frictionPatches[i].numConstraints;
			}
			else
			{
				//reserve buffers for storing accumulated impulses
				solverConstraintByteSize += sizeof(PxcSolverFrictionHeader);
				solverConstraintByteSize += PxcSolverFrictionHeader::getAppliedForcePaddingSize(c.frictionPatchContactCounts[i]);
			}
		}
	}  

	//Work out if we really need to create a solverConstraint buffer.
	if(!(n.flags & (PxcNpWorkUnitFlag::eOUTPUT_CONSTRAINTS | PxcNpWorkUnitFlag::eMODIFIABLE_CONTACT)))
	{
		solverConstraintByteSize =0;
	}

	_axisConstraintCount = axisConstraintCount;

	//16-byte alignment.
	_solverConstraintByteSize =  ((solverConstraintByteSize + 0x0f) & ~0x0f);
	PX_ASSERT(0 == (_solverConstraintByteSize & 0x0f));
}

static bool reserveBlockStreamsCoulomb(const PxcNpWorkUnit& n, PxcNpThreadContext& threadContext, const PxcCorrelationBufferCoulomb& c,
											 const bool forceContacts,
											 PxU8*& solverConstraint,
#ifdef __SPU__
						uintptr_t& eaSolverConstraint,
#endif
						PxU32& solverConstraintByteSize,
						PxU32& axisConstraintCount, PxsConstraintBlockManager& constraintBlockManager)
{
	PX_ASSERT(NULL == solverConstraint);
#ifdef __SPU__
	PX_ASSERT(0 == eaSolverConstraint);
	PX_ASSERT(0 == eaFrictionPatches);
#endif
	PX_ASSERT(0 == solverConstraintByteSize);
	PX_ASSERT(0 == axisConstraintCount);

#ifdef __SPU__
	SpuNpMemBlock& constraintBlockStream = gMemBlock;
#else
	PxcConstraintBlockStream& constraintBlockStream = threadContext.mConstraintBlockStream;
#endif

	//From constraintBlockStream we need to reserve contact points, contact forces, and a char buffer for the solver constraint data (already have a variable for this).
	//From frictionPatchStream we just need to reserve a single buffer.

	//Compute the sizes of all the buffers.
	computeBlockStreamByteSizesCoulomb(
		n, threadContext, c, forceContacts,
		solverConstraintByteSize,
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

	//Return true if neither of the two block reservations failed.
	return ((0==constraintBlockByteSize || constraintBlock));
}

//The persistent friction patch correlation/allocation will already have happenned as this is per-pair.
//This function just computes the size of the combined solve data.
void computeBlockStreamByteSizesCoulomb4(PxcSolverContact4Desc* descs,
								  PxcNpThreadContext& threadContext, const PxcCorrelationBufferCoulomb& c,
								PxU32& _solverConstraintByteSize, PxU32* _axisConstraintCount, PxU32& _numContactPoints4)
{
	PX_ASSERT(0 == _solverConstraintByteSize);
	PX_UNUSED(threadContext);

	PxU32 maxPatches = 0;
	PxU32 maxContactCount[PxcCorrelationBuffer::MAX_FRICTION_PATCHES];
	PxU32 maxFrictionCount[PxcCorrelationBuffer::MAX_FRICTION_PATCHES];
	PxMemZero(maxContactCount, sizeof(maxContactCount));
	PxMemZero(maxFrictionCount, sizeof(maxFrictionCount));
	for(PxU32 a = 0; a < 4; ++a)
	{
		PxU32 axisConstraintCount = 0;

		for(PxU32 i = 0; i < descs[a].numFrictionPatches; i++)
		{
			PxU32 ind = i + descs[a].startFrictionPatchIndex;

			const PxcFrictionPatchCoulomb& frictionPatch = c.frictionPatches[ind];

			const bool haveFriction = (frictionPatch.materialFlags & PxMaterialFlag::eDISABLE_FRICTION) == 0;
			//Solver constraint data.
			if(c.frictionPatchContactCounts[ind]!=0)
			{
				maxContactCount[i] = PxMax(c.frictionPatchContactCounts[ind], maxContactCount[i]);
				axisConstraintCount += c.frictionPatchContactCounts[ind];

				if(haveFriction)
				{
					const PxU32 fricCount = c.frictionPatches[ind].numConstraints;
					maxFrictionCount[i] = PxMax(fricCount, maxFrictionCount[i]);
					axisConstraintCount += fricCount;
				}
			}
		}
		maxPatches = PxMax(descs[a].numFrictionPatches, maxPatches);
		_axisConstraintCount[a] = axisConstraintCount;
	}

	PxU32 totalContacts = 0, totalFriction = 0;
	for(PxU32 a = 0; a < maxPatches; ++a)
	{
		totalContacts += maxContactCount[a];
		totalFriction += maxFrictionCount[a];
	}

	_numContactPoints4 = totalContacts;


	//OK, we have a given number of friction patches, contact points and friction constraints so we can calculate how much memory we need

	const bool isStatic = (((descs[0].unit->flags | descs[1].unit->flags | descs[2].unit->flags | descs[3].unit->flags) & PxcNpWorkUnitFlag::eDYNAMIC_BODY1) == 0);

	const PxU32 headerSize = (sizeof(PxcSolverContactCoulombHeader4) + sizeof(PxcSolverFrictionHeader4)) * maxPatches;
	//Add on 1 Vec4V per contact for the applied force buffer
	const PxU32 constraintSize = isStatic ? ((sizeof(PxcSolverContact4Base) + sizeof(Vec4V)) * totalContacts) + ( sizeof(PxcSolverFriction4Base) * totalFriction) : 
		((sizeof(PxcSolverContact4Dynamic) + sizeof(Vec4V)) * totalContacts) + (sizeof(PxcSolverFriction4Dynamic) * totalFriction);

	_solverConstraintByteSize =  ((constraintSize + headerSize + 0x0f) & ~0x0f);
	PX_ASSERT(0 == (_solverConstraintByteSize & 0x0f));
}


static PxcSolverConstraintPrepState::Enum reserveBlockStreamsCoulomb4(PxcSolverContact4Desc* descs, PxcNpThreadContext& threadContext, const PxcCorrelationBufferCoulomb& c,
											 PxU8*& solverConstraint,
#ifdef __SPU__
						uintptr_t& eaSolverConstraint,
#endif
						PxU32& solverConstraintByteSize,
						PxU32* axisConstraintCount, PxU32& numContactPoints4, PxsConstraintBlockManager& constraintBlockManager)
{
	PX_ASSERT(NULL == solverConstraint);
#ifdef __SPU__
	PX_ASSERT(0 == eaSolverConstraint);
#endif
	PX_ASSERT(0 == solverConstraintByteSize);
#ifdef __SPU__
	SpuNpMemBlock& constraintBlockStream = gMemBlock;
#else
	PxcConstraintBlockStream& constraintBlockStream = threadContext.mConstraintBlockStream;
#endif

	//From constraintBlockStream we need to reserve contact points, contact forces, and a char buffer for the solver constraint data (already have a variable for this).
	//From frictionPatchStream we just need to reserve a single buffer.

	//Compute the sizes of all the buffers.
	computeBlockStreamByteSizesCoulomb4(
		descs, threadContext, c, solverConstraintByteSize,
		axisConstraintCount, numContactPoints4);

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

	//Return true if neither of the two block reservations failed.
	return ((0==constraintBlockByteSize || constraintBlock)) ? PxcSolverConstraintPrepState::eSUCCESS : PxcSolverConstraintPrepState::eOUT_OF_MEMORY;
}


PxcSolverConstraintPrepState::Enum physx::createFinalizeSolverContacts4Coulomb(PxcNpWorkUnit** n,
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
								 PxReal frictionOffsetThreshold,
								 PxReal correlationDistance,
								 PxsConstraintBlockManager& constraintBlockManager,
								 PxFrictionType::Enum frictionType)
{
	PX_UNUSED(frictionOffsetThreshold);
	PX_UNUSED(correlationDistance);

	for(PxU32 i = 0; i < 4; ++i)
	{
		desc[i]->constraintLengthOver16 = 0;
	}

	PX_ASSERT(n[0]->contactCount && n[1]->contactCount && n[2]->contactCount && n[3]->contactCount);
	PX_ASSERT(!(n[0]->flags & (PxcNpWorkUnitFlag::eARTICULATION_BODY0|PxcNpWorkUnitFlag::eARTICULATION_BODY1))); //KS - we can't batch articulation contacts - not enough space on SPU

	//This represents each block 
	PxcSolverContact4Desc blockDescs[4];

	Gu::ContactBuffer& buffer = threadContext.mContactBuffer;

	buffer.count = 0;

	PxU32 numContacts = 0;

	PxTransform idt = PxTransform(PxIdentity);

	PxcCorrelationBufferCoulomb c;

	c.frictionPatchCount = 0;
	c.contactPatchCount = 0;

	PxU32 numFrictionPerPoint = PxU32(frictionType == PxFrictionType::eONE_DIRECTIONAL ? 1 : 2);

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
		if(!iter.forceNoResponse)
		{
			invMassScale0[a] = iter.getInvMassScale0();
			invMassScale1[a] = iter.getInvMassScale1();
			invInertiaScale0[a] = iter.getInvInertiaScale0();
			invInertiaScale1[a] = iter.getInvInertiaScale1();
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
						hasMaxImpulse = hasMaxImpulse || maxImpulse != PX_MAX_F32;
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
	
		blockDesc.startFrictionPatchIndex = c.frictionPatchCount;
		blockDesc.startContactPatchIndex = c.contactPatchCount;

		createContactPatchesCoulomb(c, buffer, PXC_SAME_NORMAL, blockDesc.startContactIndex);	
		
		bool overflow = correlatePatchesCoulomb(c, buffer, *blockDesc.bodyFrame0, *blockDesc.bodyFrame1, PXC_SAME_NORMAL, numFrictionPerPoint, blockDesc.startContactPatchIndex,
			blockDesc.startFrictionPatchIndex);
		if(overflow)
			return PxcSolverConstraintPrepState::eUNBATCHABLE;

		blockDesc.numContactPatches = c.contactPatchCount - blockDesc.startContactPatchIndex;
		blockDesc.numFrictionPatches = c.frictionPatchCount - blockDesc.startFrictionPatchIndex;

	}

	//OK, now we need to work out how much memory to allocate, allocate it and then block-create the constraints...

	PxU8* solverConstraint = NULL;
	PxU32 solverConstraintByteSize = 0;
	PxU32 axisConstraintCount[4];
	PxU32 numContactPoints4 = 0;

#ifdef __SPU__
	uintptr_t eaSolverConstraint = 0;
#endif

	PxcSolverConstraintPrepState::Enum state = reserveBlockStreamsCoulomb4(blockDescs, threadContext, c,
											 solverConstraint,
#ifdef __SPU__
											 eaSolverConstraint,
#endif
												solverConstraintByteSize,
												axisConstraintCount, numContactPoints4, constraintBlockManager);

	if(state != PxcSolverConstraintPrepState::eSUCCESS)
		return state;

	//OK, we allocated the memory, now let's create the constraints

	for(PxU32 a = 0; a < 4; ++a)
	{
		
#ifdef __SPU__
		n[a]->solverConstraintPointer = (PxU8*)eaSolverConstraint;
		desc[a]->constraint = (PxU8*)eaSolverConstraint;
#else
		n[a]->solverConstraintPointer = solverConstraint;
		desc[a]->constraint = solverConstraint;
#endif

		n[a]->solverConstraintSize = solverConstraintByteSize;
		n[a]->axisConstraintCount += Ps::to16(axisConstraintCount[a]);

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


	bool hasFriction = setupFinalizeSolverConstraintsCoulomb4(blockDescs, threadContext, solverConstraint, 
											invDtF32, bounceThresholdF32, c, numFrictionPerPoint, numContactPoints4, solverConstraintByteSize,
											iMassScale0, iInertiaScale0, iMassScale1, iInertiaScale1);

	*((PxU32*)(solverConstraint + solverConstraintByteSize)) = 0;
	*((PxU32*)(solverConstraint + solverConstraintByteSize + 4)) = hasFriction ? 0xFFFFFFFF : 0;


	return PxcSolverConstraintPrepState::eSUCCESS;
}



bool physx::createFinalizeSolverContactsCoulomb(PxcNpWorkUnit& n,
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
	PX_UNUSED(frictionOffsetThreshold);
	PX_UNUSED(correlationDistance);

	//CM_PROFILE_ZONE(context.getContext()->getEventProfiler(),Cm::ProfileEventId::Sim::createFinalize);
	desc.constraintLengthOver16 = 0;
	
	ContactBuffer& buffer = threadContext.mContactBuffer;

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
		return true;
	}

	Ps::prefetchLine(sBody0);
	Ps::prefetchLine(sBody1);
	Ps::prefetchLine(solverBodyData0);
	Ps::prefetchLine(solverBodyData1);

	PxcCorrelationBufferCoulomb c;// = threadContext.mCorrelationBuffer;

	createContactPatchesCoulomb(c, buffer, PXC_SAME_NORMAL, 0);	

	PxU32 numContactsPerPatch = PxU32(frictionType == PxFrictionType::eONE_DIRECTIONAL ? 1 : 2);
	
	bool overflow = correlatePatchesCoulomb(c, buffer, *bodyFrame0, *bodyFrame1, PXC_SAME_NORMAL, numContactsPerPatch, 0, 0);
	PX_UNUSED(overflow);
#ifdef __SPU__
	if(overflow)
	{
		overflowed = overflow;
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

	//PX_ASSERT(patchCount == c.frictionPatchCount);

	PxU8* solverConstraint = NULL;
#ifdef __SPU__
	uintptr_t eaSolverConstraint = 0;
#endif
	PxU32 solverConstraintByteSize = 0;
	PxU32 axisConstraintCount = 0;

	const bool successfulReserve = reserveBlockStreamsCoulomb(
		n, threadContext, c, forceContacts,
		solverConstraint,
#ifdef __SPU__
		eaSolverConstraint,
#endif
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
		n.solverConstraintPointer = (PxU8*)eaSolverConstraint;
#else
		n.solverConstraintPointer = solverConstraint;
#endif
		n.contactCount = Ps::to16(numContacts);
		n.solverConstraintSize = solverConstraintByteSize;

		//Initialise solverConstraint buffer.
		if(solverConstraint)
		{
			bool hasFriction = false;
			Ps::invalidateCache(solverConstraint, (PxI32)solverConstraintByteSize);
			if(n.flags & (PxcNpWorkUnitFlag::eARTICULATION_BODY0|PxcNpWorkUnitFlag::eARTICULATION_BODY1))
			{
				const PxcSolverBodyData& data0 = *solverBodyData0;
				const PxcSolverBodyData& data1 = *solverBodyData1;

				PxsSolverExtBody b0((void*)sBody0, (void*)&data0, desc.linkIndexA);
				PxsSolverExtBody b1((void*)sBody1, (void*)&data1, desc.linkIndexB);

				hasFriction = setupFinalizeExtSolverConstraintsCoulomb(n, buffer, c, *bodyFrame0, *bodyFrame1, perPointFriction, solverConstraint,
					invDtF32, bounceThresholdF32, b0, b1, numContactsPerPatch,
					invMassScale0, invInertiaScale0, invMassScale1, invInertiaScale1);
			}
			else
			{
				const PxcSolverBodyData& data0 = *solverBodyData0;
				const PxcSolverBodyData& data1 = *solverBodyData1;

				hasFriction = setupFinalizeSolverConstraintsCoulomb(n, buffer, c, *bodyFrame0, *bodyFrame1, perPointFriction, solverConstraint,
					*sBody0, *sBody1, data0, data1, invDtF32, bounceThresholdF32, numContactsPerPatch,
					invMassScale0, invInertiaScale0, invMassScale1, invInertiaScale1);
			}
			*((PxU32*)(solverConstraint + solverConstraintByteSize)) = 0;
			*((PxU32*)(solverConstraint + solverConstraintByteSize + 4)) = hasFriction ? 0xFFFFFFFF : 0;
		}
	}

	return successfulReserve;
}


