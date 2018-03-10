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
           
#include "ScShapeInstancePairLL.h"
//#include "ScMaterialCombiner.h"
#include "ScPhysics.h"
#include "PxsContext.h"
#include "PxsMaterialCombiner.h"
#include "GuTriangleMesh.h"
#include "ScStaticSim.h"

using namespace physx;

Sc::ShapeInstancePairLL::ShapeInstancePairLL(ShapeSim& s1, ShapeSim& s2, ActorPair& aPair, PxPairFlags pairFlags) :
	RbElementInteraction	(s1, s2, PX_INTERACTION_TYPE_OVERLAP, PX_INTERACTION_FLAG_RB_ELEMENT|PX_INTERACTION_FLAG_FILTERABLE|PX_INTERACTION_FLAG_SIP),
	mContactReportStamp		(PX_INVALID_U32),
	mFlags					(0),
	mActorPair				(aPair),
	mReportPairIndex		(INVALID_REPORT_PAIR_ID),
	mManager				(NULL),
	mReportStreamIndex		(0)
{
	// The PxPairFlags get stored in the SipFlag, make sure any changes get noticed
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eSOLVE_CONTACT == (1<<0));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eMODIFY_CONTACTS == (1<<1));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_FOUND == (1<<2));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_PERSISTS == (1<<3));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_LOST == (1<<4));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_CCD == (1<<5));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND == (1<<6));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS == (1<<7));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST == (1<<8));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_CONTACT_POINTS == (1<<9));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eDETECT_DISCRETE_CONTACT == (1<<10));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eDETECT_CCD_CONTACT == (1<<11));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::ePRE_SOLVER_VELOCITY == (1<<12));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::ePOST_SOLVER_VELOCITY == (1<<13));
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eCONTACT_EVENT_POSE == (1<<14));
	PX_COMPILE_TIME_ASSERT((PAIR_FLAGS_MASK & PxPairFlag::eSOLVE_CONTACT) == PxPairFlag::eSOLVE_CONTACT);
	PX_COMPILE_TIME_ASSERT((PxPairFlag::eSOLVE_CONTACT | PAIR_FLAGS_MASK) == PAIR_FLAGS_MASK);
	PX_COMPILE_TIME_ASSERT((PAIR_FLAGS_MASK & PxPairFlag::eCONTACT_EVENT_POSE) == PxPairFlag::eCONTACT_EVENT_POSE);
	PX_COMPILE_TIME_ASSERT((PxPairFlag::eCONTACT_EVENT_POSE | PAIR_FLAGS_MASK) == PAIR_FLAGS_MASK);

	setPairFlags(pairFlags);

	// sizeof(ShapeInstancePairLL): 84 => 80 bytes

	PX_ASSERT(!mLLIslandHook.isManaged()); 
}


void Sc::ShapeInstancePairLL::visualize(Cm::RenderOutput& out)
{
	if (mManager)  // sleeping pairs have no contact points -> do not visualize
	{
		Scene& scene = getScene();
		PxReal scale = scene.getVisualizationScale();

		const PxReal flipNormal = (&mActorPair.getActorA() == &getShape0().getRbSim()) ? 1.0f : -1.0f;

		PxU32 offset;
		PxU32 nextOffset = 0;
		do
		{
			const void* contactData;
			PxU32 contactDataSize;
			PxU32 contactPointCount;
			const PxReal* impulses;

			offset = nextOffset;
			nextOffset = getContactPointData(contactData, contactDataSize, contactPointCount, impulses, offset);

			const PxReal param_contactForce = scene.getVisualizationParameter(PxVisualizationParameter::eCONTACT_FORCE);
			const PxReal param_contactNormal = scene.getVisualizationParameter(PxVisualizationParameter::eCONTACT_NORMAL);
			const PxReal param_contactError = scene.getVisualizationParameter(PxVisualizationParameter::eCONTACT_ERROR);
			const PxReal param_contactPoint = scene.getVisualizationParameter(PxVisualizationParameter::eCONTACT_POINT);

			PxContactStreamIterator iter((PxU8*)contactData, contactDataSize);

			PxU32 i = 0;
			while(iter.hasNextPatch())
			{
				iter.nextPatch();
				while(iter.hasNextContact())
				{
					iter.nextContact();

					PxReal length = 0;
					PxU32 color = 0;

					if ((param_contactForce != 0.0f) && impulses)
					{
						length = scale * param_contactForce * impulses[i];
						color = 0xff0000;
					}
					else if (param_contactNormal != 0.0f)
					{
						length = scale * param_contactNormal;
						color = 0x0000ff;
					}
					else if (param_contactError != 0.0f)
					{
						length = PxAbs(scale * param_contactError * iter.getSeparation());
						color = 0xffff00;
					}

					if (length != 0)
						out << Cm::RenderOutput::LINES << color << iter.getContactPoint() << iter.getContactPoint() + iter.getContactNormal() * length * flipNormal;

					if (param_contactPoint != 0)
					{
						PxReal s = scale * 0.1f;
						PxVec3 point = iter.getContactPoint();

						if(0) //temp debug to see identical contacts
							point.x += scale * 0.01f * (contactPointCount - i + 1);

						out << Cm::RenderOutput::LINES << PxU32(PxDebugColor::eARGB_RED);
						out << point + PxVec3(-s,0,0) << point + PxVec3(s,0,0);
						out << point + PxVec3(0,-s,0) << point + PxVec3(0,s,0);
						out << point + PxVec3(0,0,-s) << point + PxVec3(0,0,s);

					}
				}
			}
		}
		while(nextOffset != offset);
	}
}


PX_FORCE_INLINE	void Sc::ShapeInstancePairLL::processReportPairOnActivate()
{
	PX_ASSERT(isReportPair());
	PX_ASSERT(mReportPairIndex == INVALID_REPORT_PAIR_ID);

	if (readIntFlag(WAS_IN_PERSISTENT_EVENT_LIST))
	{
		getScene().getNPhaseCore()->addToPersistentContactEventPairs(this);
		mFlags &= ~WAS_IN_PERSISTENT_EVENT_LIST;
	}
}


PX_FORCE_INLINE	void Sc::ShapeInstancePairLL::processReportPairOnDeactivate()
{
	PX_ASSERT(isReportPair());
	PX_ASSERT(mReportPairIndex != INVALID_REPORT_PAIR_ID);
	PX_COMPILE_TIME_ASSERT(IS_IN_PERSISTENT_EVENT_LIST == (WAS_IN_PERSISTENT_EVENT_LIST >> 1));
	PX_ASSERT(!(readIntFlag(WAS_IN_PERSISTENT_EVENT_LIST)));

	PxU32 wasInPersList = (mFlags & IS_IN_PERSISTENT_EVENT_LIST) << 1;
	mFlags |= wasInPersList;

	removeFromReportPairList();
}


void Sc::ShapeInstancePairLL::setContactReportPostSolverVelocity(ContactStreamManager& cs)
{
	Scene& scene = getScene();
	NPhaseCore* npcore = scene.getNPhaseCore();
	PxU8* stream = npcore->getContactReportPairData(cs.bufferIndex);
	
	cs.setContactReportPostSolverVelocity(stream, mActorPair.getActorA(), mActorPair.getActorB());
}


void Sc::ShapeInstancePairLL::processUserNotification(PxU32 contactEvent, PxU16 infoFlags, bool touchLost, const PxU32 ccdPass)
{
	PX_ASSERT(hasTouch());

	contactEvent = (!ccdPass) ? contactEvent : (contactEvent | PxPairFlag::eNOTIFY_TOUCH_CCD);

	if(mManager)
		Ps::prefetchLine(mManager);

	Scene& scene = getScene();
	NPhaseCore* npcore = scene.getNPhaseCore();

	PxU32 pairFlags = getPairFlags();
	PX_ASSERT(pairFlags & contactEvent);
	const PxU32 extraDataFlags = pairFlags & CONTACT_REPORT_EXTRA_DATA;
	
	// make sure shape A and shape B are the same way round as the actors (in compounds they may be swapped)
	// TODO: make "unswapped" a SIP flag and set it in updateState()
	const bool unswapped = &mActorPair.getActorA() == &getShape0().getRbSim();
	const Sc::ShapeSim& shapeA = unswapped ? getShape0() : getShape1();
	const Sc::ShapeSim& shapeB = unswapped ? getShape1() : getShape0();

	if(!mActorPair.isInContactReportActorPairSet())
	{
		mActorPair.setInContactReportActorPairSet();
		npcore->addToContactReportActorPairSet(&mActorPair);
		mActorPair.incRefCount();
	}

	// Prepare user notification
	PxU32 timeStamp = scene.getTimeStamp();
	PxU32 shapePairTimeStamp = scene.getReportShapePairTimeStamp();
	
	PxU8* stream = NULL;
	ContactShapePair* pairStream = NULL;
	ContactStreamManager& cs = mActorPair.getContactStreamManager();
	if(mActorPair.streamResetStamp(timeStamp))
	{
		PX_ASSERT(mContactReportStamp != shapePairTimeStamp);  // actor pair and shape pair timestamps must both be out of sync in this case

		PxU16 maxCount;
		if (cs.maxPairCount != 0)
			maxCount = cs.maxPairCount;  // use value from previous report
		else
		{
			// TODO: Use some kind of heuristic
			maxCount = 2;
			cs.maxPairCount = maxCount;
		}

		PxU32 maxExtraDataSize;
		if (!extraDataFlags || touchLost)
		{
			maxExtraDataSize = 0;
			cs.setMaxExtraDataSize(maxExtraDataSize);
		}
		else
		{
			PxU32 currentMaxExtraDataSize = cs.getMaxExtraDataSize();
			maxExtraDataSize = ContactStreamManager::computeContactReportExtraDataSize(extraDataFlags, true);
			PX_ASSERT(maxExtraDataSize > 0);
			if (maxExtraDataSize <= currentMaxExtraDataSize)
				maxExtraDataSize = currentMaxExtraDataSize;  // use value from previous report
			else
				cs.setMaxExtraDataSize(maxExtraDataSize);
		}

		stream = npcore->reserveContactReportPairData(maxCount, maxExtraDataSize, cs.bufferIndex);

		if (!maxExtraDataSize)  // this is the usual case, so set it first for branch prediction
			cs.reset();
		else if (stream)
		{
			cs.reset();
			PX_ASSERT(extraDataFlags);
			PX_ASSERT(!touchLost);

			cs.fillInContactReportExtraData(stream, extraDataFlags, mActorPair.getActorA(), mActorPair.getActorB(), ccdPass, 0, sizeof(ContactStreamHeader));
			if ((extraDataFlags & PxPairFlag::ePOST_SOLVER_VELOCITY) && (pairFlags & PxPairFlag::eDETECT_CCD_CONTACT))
				scene.setPostSolverVelocityNeeded();
		}
	}
	else
	{
		const PxU32 currentPairCount = cs.currentPairCount;
		if(currentPairCount != 0)
		{
			PxU8* tmpStreamPtr = npcore->getContactReportPairData(cs.bufferIndex);
			if (!extraDataFlags)
				stream = tmpStreamPtr;  // this is the usual case, so set it first for branch prediction
			else
			{
				if (!touchLost)
				{
					// - the first few shape pair events might not request extra data
					// - the events so far were due to touch lost
					// - multiple reports due to CCD multiple passes
					// Hence, the extra data has to be created/extended now.
					//
					PxU16 oldExtraDataSize = cs.extraDataSize;
					PxI32 lastContactPass;
					if (oldExtraDataSize)
					{
						ContactStreamHeader* strHeader = reinterpret_cast<ContactStreamHeader*>(tmpStreamPtr);
						lastContactPass = strHeader->contactPass;
					}
					else
						lastContactPass = -1;

					if ((PxI32)ccdPass > lastContactPass)  // do not send extra data mulitple times for the same contact pass
					{
						PxU16 extraDataSize = (PxU16)(oldExtraDataSize + ContactStreamManager::computeContactReportExtraDataSize(extraDataFlags, (oldExtraDataSize == 0)));
						PxU8* strPtr;
						if (extraDataSize <= cs.getMaxExtraDataSize())
							strPtr = tmpStreamPtr;
						else
							strPtr = npcore->resizeContactReportPairData(currentPairCount < cs.maxPairCount ? cs.maxPairCount : (PxU32)(cs.maxPairCount+1), extraDataSize, cs);
							// the check for max pair count is there to avoid another potential allocation further below

						if (strPtr)
						{
							stream = strPtr;
							PxU32 sizeOffset;
							if (oldExtraDataSize)
								sizeOffset = oldExtraDataSize;
							else
								sizeOffset = sizeof(ContactStreamHeader);
							cs.fillInContactReportExtraData(strPtr, extraDataFlags, mActorPair.getActorA(), mActorPair.getActorB(), ccdPass, currentPairCount, sizeOffset);
							if ((extraDataFlags & PxPairFlag::ePOST_SOLVER_VELOCITY) && (pairFlags & PxPairFlag::eDETECT_CCD_CONTACT))
								scene.setPostSolverVelocityNeeded();
						}
						else
						{
							stream = tmpStreamPtr;
							cs.raiseFlags(ContactStreamManagerFlag::eINCOMPLETE_STREAM);
						}
					}
					else
						stream = tmpStreamPtr;
				}
				else
					stream = tmpStreamPtr;
			}
		}
	}

	if(stream)
		pairStream = cs.getShapePairs(stream);
	else
	{
		cs.raiseFlags(ContactStreamManagerFlag::eINVALID_STREAM);
		return;
	}

	ContactShapePair* cp;
	if (mContactReportStamp != shapePairTimeStamp)
	{
		// this shape pair is not in the contact notification stream yet

		if (cs.currentPairCount < cs.maxPairCount)
			cp = pairStream + cs.currentPairCount;
		else
		{
			PxU32 newSize = PxU32(cs.currentPairCount + (cs.currentPairCount >> 1) + 1);
			stream = npcore->resizeContactReportPairData(newSize, cs.getMaxExtraDataSize(), cs);
			if(stream)
			{
				pairStream = cs.getShapePairs(stream);
				cp = pairStream + cs.currentPairCount;
			}
			else
			{
				cs.raiseFlags(ContactStreamManagerFlag::eINCOMPLETE_STREAM);
				return;
			}
		}

		//!!! why is alignment important here? Looks almost like some refactor nonsense
		PX_ASSERT(0==((uintptr_t)stream & 0x0f));  // check 16Byte alignment
		
		mReportStreamIndex = cs.currentPairCount;
		cp->shapes[0] = shapeA.getPxShape();
		cp->shapes[1] = shapeB.getPxShape();
		cp->contactStream = NULL;
		cp->contactCount = 0;
		cp->constraintStreamSize = 0;
		cp->requiredBufferSize = 0;
		cp->flags = infoFlags;
		PX_ASSERT(contactEvent <= 0xffff);
		cp->events = (PxU16)contactEvent;
		cp->shapeID[0] = shapeA.getID();
		cp->shapeID[1] = shapeB.getID();

		cs.currentPairCount++;

		mContactReportStamp = shapePairTimeStamp;
	}
	else
	{
		// this shape pair is in the contact notification stream already but there is a second event (can happen with force threshold reports, for example).
		
		PX_ASSERT(mReportStreamIndex < cs.currentPairCount);
		cp = &pairStream[mReportStreamIndex];
		cp->events |= contactEvent;
		cp->flags |= infoFlags;
	}

	if ((getPairFlags() & PxPairFlag::eNOTIFY_CONTACT_POINTS) && mManager && (!cp->contactStream) && !(contactEvent & (PxU32)(PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST)))
	{
		const void* contactData;
		PxU32 contactDataSize;
		const PxReal* contactImpulses;
		const PxsCCDContactHeader* ccdContactData;
		PxU32 nbPoints = mManager->getContactPointData(contactData, contactDataSize, contactImpulses, ccdContactData);

		bool isCCDPass = (ccdPass != 0);
		if ((contactDataSize && !isCCDPass) || (ccdContactData && (!ccdContactData->isFromPreviousPass) && isCCDPass))
		{
			const PxU8* cData;
			PxU32 cDataSize;
			PxU32 alignedContactDataSize;
			const PxReal* impulses;
			if (!isCCDPass)
			{
				PX_ASSERT(0==((uintptr_t)contactData & 0x0f));  // check 16Byte alignment
				cData = reinterpret_cast<const PxU8*>(contactData);
				cDataSize = contactDataSize;
				alignedContactDataSize = (cDataSize + 0xf) & 0xfffffff0;
				impulses = contactImpulses;
			}
			else
			{
				PX_ASSERT(0==((uintptr_t)ccdContactData & 0x0f));  // check 16Byte alignment
				cData = reinterpret_cast<const PxU8*>(ccdContactData) + sizeof(PxsCCDContactHeader);
				cDataSize = ccdContactData->contactStreamSize - sizeof(PxsCCDContactHeader);
				PxU32 tmpAlignedSize = (ccdContactData->contactStreamSize + 0xf) & 0xfffffff0;
				alignedContactDataSize = tmpAlignedSize - sizeof(PxsCCDContactHeader);
				impulses = reinterpret_cast<const PxReal*>(cData + alignedContactDataSize);
				nbPoints = 1;
			}

			infoFlags = cp->flags;
			infoFlags |= unswapped ? 0 : PxContactPairFlag::eINTERNAL_CONTACTS_ARE_FLIPPED;
			infoFlags |= readIntFlag(FACE_INDEX_REPORT_PAIR) ? PxContactPairFlag::eINTERNAL_HAS_FACE_INDICES : 0;

			PX_ASSERT(0==((uintptr_t)impulses & 0x0f));

			PX_ASSERT((impulses == NULL) || ((cData + alignedContactDataSize) == (const PxU8*)(impulses)));	// make sure contacts and impulses use consecutive memory space.
																												// Note: There are no impulses if collision response has been disabled.
			PxU32 impulseSize = impulses ? (nbPoints * sizeof(PxReal)) : 0;
			if (impulseSize)
				infoFlags |= PxContactPairFlag::eINTERNAL_HAS_IMPULSES;
			cp->contactStream = cData;
			cp->contactCount = Ps::to16(nbPoints);
			cp->constraintStreamSize = (PxU16)cDataSize;
			cp->requiredBufferSize = alignedContactDataSize + impulseSize;

			cp->flags = infoFlags;
		}
	}
}

PxU32 Sc::ShapeInstancePairLL::getContactPointData(const void*& contactData, PxU32& contactDataSize, PxU32& contactPointCount, const PxReal*& impulses, PxU32 startOffset)
{
	// Process LL generated contacts
	if (mManager != NULL)
	{
		const void* dcdContactStream;
		PxU32 dcdContactStreamSize;
		const PxReal* dcdImpulses;
		const PxsCCDContactHeader* ccdContactStream;
		PxU32 dcdContactCount = mManager->getContactPointData(dcdContactStream, dcdContactStreamSize, dcdImpulses, ccdContactStream);

		PX_ASSERT(((dcdContactCount == 0) && (!ccdContactStream)) || ((dcdContactCount > 0) && hasTouch()) || (ccdContactStream && hasCCDTouch()));

		PxU32 idx = 0;
		if (dcdContactCount)
		{
			if (startOffset == 0)
			{
				contactData = dcdContactStream;
				contactDataSize = dcdContactStreamSize;
				contactPointCount = dcdContactCount;
				impulses = dcdImpulses;

				if (!ccdContactStream)
					return startOffset;
				else
					return (startOffset + 1);
			}

			idx++;
		}

		while(ccdContactStream)
		{
			if (startOffset == idx)
			{
				const PxU8* stream = reinterpret_cast<const PxU8*>(ccdContactStream);
				PxU16 streamSize = ccdContactStream->contactStreamSize;
				contactData = stream + sizeof(PxsCCDContactHeader);
				contactDataSize = streamSize - sizeof(PxsCCDContactHeader);
				contactPointCount = 1;
				impulses = reinterpret_cast<const PxReal*>(stream + ((streamSize + 0xf) & 0xfffffff0));

				if (!ccdContactStream->nextStream)
					return startOffset;
				else
					return (startOffset + 1);
			}
			
			idx++;
			ccdContactStream = ccdContactStream->nextStream;
		}
	}

	contactData = NULL;
	contactDataSize = 0;
	contactPointCount = 0;
	impulses = NULL;
	return startOffset;
}


// Note that LL will not send end touch events for managers that are destroyed while having contact
void Sc::ShapeInstancePairLL::managerNewTouch(const PxU32 ccdPass, bool adjustCounters)
{
	if (readIntFlag(HAS_TOUCH))
		return; // Do not count the touch twice (for instance when recreating a manager with touch)
	// We have contact this frame
    setHasTouch();

	if (adjustCounters)
		adjustCountersOnNewTouch();

	if(!readIntFlag(CONTACTS_RESPONSE_DISABLED))
	{
		PxsIslandManager& islandManager = getScene().getInteractionScene().getLLIslandManager();
		islandManager.setEdgeConnected(mLLIslandHook);
	}
	
	if(!isReportPair())
		return;
	else
	{
		PX_ASSERT(hasTouch());
		PX_ASSERT(!readIntFlag(IS_IN_PERSISTENT_EVENT_LIST));
		PX_ASSERT(!readIntFlag(IS_IN_FORCE_THRESHOLD_EVENT_LIST));

		PxU32 pairFlags = getPairFlags();
		if (pairFlags & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			PxU16 infoFlag = 0;
			if (mActorPair.getTouchCount() == 1)  // this code assumes that the actor pair touch count does get incremented beforehand
			{
				infoFlag = PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH;
			}

			processUserNotification(PxPairFlag::eNOTIFY_TOUCH_FOUND, infoFlag, false, ccdPass);
		}

		if (pairFlags & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
		{
			getScene().getNPhaseCore()->addToPersistentContactEventPairsDelayed(this);  // to make sure that from now on, the pairs are tested for persistent contact events
		}
		else if (pairFlags & ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS)
		{
			// new touch -> need to start checking for force threshold events
			// Note: this code assumes that it runs before the pairs get tested for force threshold exceeded
			getScene().getNPhaseCore()->addToForceThresholdContactEventPairs(this);
		}
	}
}


bool Sc::ShapeInstancePairLL::managerLostTouch(const PxU32 ccdPass, bool adjustCounters)
{
	if (!readIntFlag(HAS_TOUCH))
		return false;

	PxsIslandManager& islandManager = getScene().getInteractionScene().getLLIslandManager();
	PX_ASSERT(mLLIslandHook.isManaged());
	islandManager.setEdgeUnconnected(mLLIslandHook);

	// We do not have LL contacts this frame and also we lost LL contact this frame

	if (!isReportPair())
	{
		setHasNoTouch();
	}
	else
	{
		PX_ASSERT(hasTouch());

		sendLostTouchReport(false, ccdPass);

		if (readIntFlag(IS_IN_CONTACT_EVENT_LIST))
		{
			// don't need to worry about persistent/force-threshold contact events until next new touch

			if (readIntFlag(IS_IN_FORCE_THRESHOLD_EVENT_LIST))
			{
				getScene().getNPhaseCore()->removeFromForceThresholdContactEventPairs(this);
			}
			else
			{
				PX_ASSERT(readIntFlag(IS_IN_PERSISTENT_EVENT_LIST));
				getScene().getNPhaseCore()->removeFromPersistentContactEventPairs(this);
			}

			clearFlag(FORCE_THRESHOLD_EXCEEDED_FLAGS);
		}

		setHasNoTouch();
	}

	if (adjustCounters)
		adjustCountersOnLostTouch();

	BodySim* body0 = getShape0().getBodySim();
	BodySim* body1 = getShape1().getBodySim();

	if (!body0 || !body1)
	{
		if (body0)
		{
			body0->internalWakeUp();
		}
		if (body1)
		{
			body1->internalWakeUp();
		}

		return false;
	}
	return true;
}


void Sc::ShapeInstancePairLL::updateState(bool removeFromDirtyList)
{
	const PxU32 oldContactState = getManagerContactState();

	// Copy dirty flags before they get cleared below
	const PxU16 dirtyFlags = getDirtyFlags();
	CoreInteraction::updateState(removeFromDirtyList);
	Scene& scene = getScene();
	InteractionScene& iscene = scene.getInteractionScene();

	// Update notification status
	if (dirtyFlags & (CIF_DIRTY_NOTIFICATION | CIF_DIRTY_VISUALIZATION))
	{
		PxU32 pairFlags = getPairFlags();

		// Check if collision response is disabled
		Sc::BodySim* bs0 = getShape0().getBodySim();
		Sc::BodySim* bs1 = getShape1().getBodySim();
		bool wasEnabled = (oldContactState & CONTACTS_RESPONSE_DISABLED) == 0;
		bool enabled = (bs0 && !bs0->isKinematic()) || (bs1 && !bs1->isKinematic());  // If the pair has no dynamic body then disable response
		enabled = enabled && (pairFlags & PxPairFlag::eSOLVE_CONTACT);
		setFlag(CONTACTS_RESPONSE_DISABLED, !enabled);

		if (wasEnabled != enabled)
		{
			if (enabled)
			{
				PX_ASSERT(!scene.getInteractionScene().getLLIslandManager().getIsEdgeConnected(mLLIslandHook));
				scene.getInteractionScene().getLLIslandManager().setEdgeConnected(mLLIslandHook);
			}
			else
				scene.getInteractionScene().getLLIslandManager().setEdgeUnconnected(mLLIslandHook);
		}

		// Check if contact points needed
		setFlag(CONTACTS_COLLECT_POINTS, ((pairFlags & PxPairFlag::eNOTIFY_CONTACT_POINTS) ||
										  (pairFlags & PxPairFlag::eMODIFY_CONTACTS) || 
										  scene.getVisualizationParameter(PxVisualizationParameter::eCONTACT_POINT) ||
										  scene.getVisualizationParameter(PxVisualizationParameter::eCONTACT_NORMAL) ||
										  scene.getVisualizationParameter(PxVisualizationParameter::eCONTACT_ERROR) ||
										  scene.getVisualizationParameter(PxVisualizationParameter::eCONTACT_FORCE)) );
	}

	if (readDirtyFlag(dirtyFlags, CIF_DIRTY_BODY_KINEMATIC))
	{
		// Check kinematic shapes
		Sc::BodySim* bs0 = getShape0().getBodySim();
		Sc::BodySim* bs1 = getShape1().getBodySim();
		setFlag(SHAPE0_IS_KINEMATIC, (bs0 && (bs0->isKinematic()!=0)));
		setFlag(SHAPE1_IS_KINEMATIC, (bs1 && (bs1->isKinematic()!=0)));
	}

	// Check if the sip has contact reports requested
	if (isReportPair())
	{
		PX_ASSERT((mReportPairIndex == INVALID_REPORT_PAIR_ID) || (!readIntFlag(WAS_IN_PERSISTENT_EVENT_LIST)));

		if (mReportPairIndex == INVALID_REPORT_PAIR_ID && scene.getInteractionScene().isActiveInteraction(this))
		{
			PX_ASSERT(!readIntFlag(WAS_IN_PERSISTENT_EVENT_LIST));  // sanity check: an active pair should never have this flag set

			// the code below is to cover the case where the pair flags for a pair changed
			if (hasTouch() && (getPairFlags() & ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS))
				scene.getNPhaseCore()->addToForceThresholdContactEventPairs(this);
		}

		if ((getPairFlags() & PxPairFlag::eNOTIFY_CONTACT_POINTS) &&
			((getShape0().getGeometryType() == PxGeometryType::eTRIANGLEMESH) || (getShape0().getGeometryType() == PxGeometryType::eHEIGHTFIELD) ||
			(getShape1().getGeometryType() == PxGeometryType::eTRIANGLEMESH) || (getShape1().getGeometryType() == PxGeometryType::eHEIGHTFIELD)))
			raiseFlag(FACE_INDEX_REPORT_PAIR);
		else
			clearFlag(FACE_INDEX_REPORT_PAIR);
	}
	PX_ASSERT(isReportPair() || (!readIntFlag(WAS_IN_PERSISTENT_EVENT_LIST)));  // pairs without contact reports should never have this flag set.

	PxU32 newContactState = getManagerContactState();
	bool recreateManager = (oldContactState != newContactState);

	// No use in updating manager properties if the manager is going to be re-created or does not exist yet
	if ((!recreateManager) && (mManager != 0))
	{
		ShapeSim& shapeSim0 = getShape0();
		ShapeSim& shapeSim1 = getShape1();

		// Update dominance
		if (readDirtyFlag(dirtyFlags, CIF_DIRTY_DOMINANCE))
		{
			Sc::BodySim* bs0 = shapeSim0.getBodySim();
			Sc::BodySim* bs1 = shapeSim1.getBodySim();

			// Static actors are in dominance group zero and must remain there
			const PxDominanceGroup dom0 = bs0 ? bs0->getActorCore().getDominanceGroup() : (PxDominanceGroup)0;
			const PxDominanceGroup dom1 = bs1 ? bs1->getActorCore().getDominanceGroup() : (PxDominanceGroup)0;

			const PxDominanceGroupPair cdom = getScene().getDominanceGroupPair(dom0, dom1);
			mManager->setDominance0(cdom.dominance0);
			mManager->setDominance1(cdom.dominance1);
		}

		// Update skin width
		if (readDirtyFlag(dirtyFlags, CIF_DIRTY_REST_OFFSET))
		{
			mManager->setRestDistance(shapeSim0.getRestOffset() + shapeSim1.getRestOffset());
		}

		// Activate, create managers as needed
		updateManager();
	}
	else if (iscene.isActiveInteraction(this))  // only re-create the manager if the pair is active
	{
		// A) This is a newly created pair
		//
		// B) The contact notification or processing state has changed.
		//    All existing managers need to be deleted and recreated with the correct flag set
		//    These flags can only be set at creation in LL
		resetManager();
	}

	if (mManager)	//TODO: this looks misplaced here!
		setSweptProperties();
}


void Sc::ShapeInstancePairLL::initialize()
{
	//Add a fresh edge to the island manager.
	PxsIslandManager& islandManager = getScene().getInteractionScene().getLLIslandManager();
	Sc::BodySim* bs0 = getShape0().getBodySim();
	Sc::BodySim* bs1 = getShape1().getBodySim();
	PxsIslandManagerNodeHook islandManagerHookA(PxsIslandManagerNodeHook::INVALID);
	PxsIslandManagerNodeHook islandManagerHookB(PxsIslandManagerNodeHook::INVALID);
	const PxU32 actorTypeA = bs0 ? bs0->getActorType() : PxActorType::eRIGID_STATIC;
	const PxU32 actorTypeB = bs1 ? bs1->getActorType() : PxActorType::eRIGID_STATIC;
	if(PxActorType::eRIGID_DYNAMIC == actorTypeA || PxActorType::eARTICULATION_LINK==actorTypeA)
	{
		islandManagerHookA = bs0->getLLIslandManagerNodeHook();
		PX_ASSERT(islandManagerHookA.isManaged());
	}
	if(PxActorType::eRIGID_DYNAMIC == actorTypeB || PxActorType::eARTICULATION_LINK==actorTypeB)
	{
		islandManagerHookB = bs1->getLLIslandManagerNodeHook();
		PX_ASSERT(islandManagerHookB.isManaged());
	}
	PX_ASSERT(!mLLIslandHook.isManaged());
	islandManager.addEdge(PxsIslandManager::EDGE_TYPE_CONTACT_MANAGER,islandManagerHookA,islandManagerHookB,mLLIslandHook);

	// If the pair will be asleep initially, we want the island manager to build an island. This is to make sure that
	// when an awake object starts touching an island of objects which were added asleep, it will still wake the whole island
	// up AND make sure all the edges of that island will generate solver constraints immediately if there is touch.
	// Note that this will not trigger narrowphase automatically for sleeping pairs. Only if the island manager detects that
	// an awake object was touching the sleeping island, the second narrowphase kicks in and computes the collision information.
	if (!activeManagerAllowed())
	{
		islandManager.setEdgeConnected(mLLIslandHook);
	}

	RbElementInteraction::initialize();
	PX_ASSERT((&getShape0()) && (&getShape1()));

	mActorPair.incRefCount();
}

void Sc::ShapeInstancePairLL::destroy()
{
	destroyManager();

	getScene().getInteractionScene().getLLIslandManager().removeEdge(PxsIslandManager::EDGE_TYPE_CONTACT_MANAGER, mLLIslandHook);

	// This will remove the interaction from the actors list, which will prevent
	// update calls to this actor because of Body::wakeUp below.
	RbElementInteraction::destroy();

	if (mReportPairIndex != INVALID_REPORT_PAIR_ID)
	{
		removeFromReportPairList();
	}

	if (hasTouch())
	{
		// The SIP is removed explicitly because we still have contact
		BodySim* body0 = getShape0().getBodySim();
		BodySim* body1 = getShape1().getBodySim();
		if (body0)
			body0->decrementBodyConstraintCounter();
		if (body1)
			body1->decrementBodyConstraintCounter();
		mActorPair.decTouchCount();
	}

	mActorPair.decRefCount();
}


bool Sc::ShapeInstancePairLL::onActivate(PxU32 infoFlag)
{
	PX_ASSERT(!(infoFlag & Actor::AS_PART_OF_ISLAND_GEN_PASS_2));  // second pass of island gen should never wake objects up, it might only put them asleep again.

	RbElementInteraction::onActivate(infoFlag);

	if (isReportPair() && !(infoFlag & Actor::AS_PART_OF_ISLAND_GEN_PASS_1))
	{
		// for pairs that go through a second island pass, there is the possibility that they get put to sleep again after the second pass.
		// So we do not want to check for re-insertion into the persistent report pair list yet.

		processReportPairOnActivate();
	}

	if (updateManager())
	{
		//
		// recorded touch status   |     edge status     |   real touch status   |               onActivate event             |   narrowphase event   |   
		//------------------------------------------------------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------------------------------------------------------
		// unknown (=no touch)     | connected           | touch                 | unconnect edge                             | newTouch              | 
		// (for newly added pair)  |                     |                       | (will cause unnecessary entries but no     | -> connect edge       |
		//                         |                     |                       |  other impact)                             |                       |
		//------------------------------------------------------------------------------------------------------------------------------------------------
		// "                       | "                   | no touch              | unconnect edge                             | -                     | (*)
		//------------------------------------------------------------------------------------------------------------------------------------------------
		// touch                   | connected           | touch                 | connect edge                               | -                     |
		//------------------------------------------------------------------------------------------------------------------------------------------------
		// "                       | "                   | no touch              | connect edge                               | lostTouch             |
		//                         |                     |                       |                                            | -> unconnect edge     |
		//------------------------------------------------------------------------------------------------------------------------------------------------
		// no touch                | connected           | touch                 | unconnect edge                             | newTouch              |
		//                         | (due to user        |                       | (will cause unnecessary entries but no     | -> connect edge       |
		//                         | deactivate/move..)  |                       |  other impact)                             |                       |
		//------------------------------------------------------------------------------------------------------------------------------------------------
		// "                       | "                   | no touch              | unconnect edge                             | -                     | (*)
		//------------------------------------------------------------------------------------------------------------------------------------------------
		// "                       | unconnected         | no touch              | unconnect edge                             | -                     |
		//                         | (no potential touch |                       |                                            |                       |
		//                         |  status change since|                       |                                            |                       |
		//                         |  put to sleep by    |                       |                                            |                       |
		//                         |  island gen)        |                       |                                            |                       |
		//------------------------------------------------------------------------------------------------------------------------------------------------
		//
		// This table tries to explain why it is necessary to set the edge connectivity if the recorded touch status is "no touch". Basically, the
		// cases marked with (*) would fail without this logic.
		//
		// Some combinations are missing because they can not occur. For example, if a pair had touch, the edge will always be connected on wake up because
		// nothing can unconnect the edge while the pair is not active (same for pairs added asleep). Similarly, a pair that had no touch and was not marked 
		// dirty by any operation can not lose touch while being inactive.
		//
		if (!hasTouch())
			getScene().getInteractionScene().getLLIslandManager().setEdgeUnconnected(mLLIslandHook);
		else
		{
			PX_ASSERT(readIntFlag(CONTACTS_RESPONSE_DISABLED) || getScene().getInteractionScene().getLLIslandManager().getIsEdgeConnected(mLLIslandHook));
		}

		setSweptProperties();
		return true;
	}
	else
		return false;
}


bool Sc::ShapeInstancePairLL::onDeactivate(PxU32 infoFlag)
{
#ifdef _DEBUG
	if (infoFlag & Actor::AS_PART_OF_ISLAND_GEN)
	{
		bool isEdgeConnected = getScene().getInteractionScene().getLLIslandManager().getIsEdgeConnected(mLLIslandHook);
		PX_ASSERT((!mManager) || (mManager->getTouchStatus() && isEdgeConnected) || (!mManager->getTouchStatus() && !isEdgeConnected));
		PX_ASSERT((!mManager) || mManager->touchStatusKnown());
	}
#endif

	if (infoFlag & Actor::AS_PART_OF_ISLAND_GEN_PASS_2)
	{
		// This is necessary if deactivation gets called as part of the second island gen pass logic for pairs that got
		// woken up after first island gen pass and were put to sleep again after the second island pass (because it turned out they
		// are part of a subisland that can be put to sleep again). Since we do not want to send reports for such pairs, we need to delay
		// the corresponding work. That causes the buffered HAS_TOUCH flag to potentially become out of snyc with the real touch status
		// and this needs to get corrected here.

		PX_ASSERT(mManager);
		PX_ASSERT(mManager->touchStatusKnown());

		bool touchStatus = (mManager->getTouchStatus() > 0);
		bool bufferedTouchStatus = (hasTouch() > 0);

		if (touchStatus != bufferedTouchStatus)
		{
			PX_ASSERT(mReportPairIndex == INVALID_REPORT_PAIR_ID);
			PX_ASSERT(!readIntFlag(IS_IN_PERSISTENT_EVENT_LIST));
			PX_ASSERT(!readIntFlag(IS_IN_FORCE_THRESHOLD_EVENT_LIST));

			PxU32 pairFlags = getPairFlags();

			if (touchStatus)
			{
				setHasTouch();

				// did not have touch -> activation did not add it to the list of persistent report pairs

				PX_ASSERT(!readIntFlag(WAS_IN_PERSISTENT_EVENT_LIST));

				if (pairFlags & (PxPairFlag::eNOTIFY_TOUCH_PERSISTS | CONTACT_FORCE_THRESHOLD_PAIRS))
				{
					// set flag as if the pair had been in the list of report pairs
					raiseFlag(WAS_IN_PERSISTENT_EVENT_LIST);
				}

				// we need to discuss whether to send a touch found event here if requested
			}
			else
			{
				// did have touch, however, for second island pass pairs, activation should not add the pair 
				// to the persistent report pair list immediately, hence, the old state should still be visible.

				if (pairFlags & (PxPairFlag::eNOTIFY_TOUCH_PERSISTS | CONTACT_FORCE_THRESHOLD_PAIRS))
				{
					PX_ASSERT(readIntFlag(WAS_IN_PERSISTENT_EVENT_LIST));

					// set flag as if the pair had not been in the list of report pairs
					clearFlag(WAS_IN_PERSISTENT_EVENT_LIST);
				}
				else
				{
					PX_ASSERT(!readIntFlag(WAS_IN_PERSISTENT_EVENT_LIST));
				}

				// we need to discuss whether to send a touch lost event here if requested

				setHasNoTouch();
			}
		}
		else if (mReportPairIndex != INVALID_REPORT_PAIR_ID)
		{
			processReportPairOnDeactivate();
		}
	}
	else if (mReportPairIndex != INVALID_REPORT_PAIR_ID)
	{
		processReportPairOnDeactivate();
	}

	if (mManager != 0)
	{
		if ((!readIntFlag(TOUCH_KNOWN)) && mManager->touchStatusKnown() && (!mManager->getTouchStatus()))
		{
			// for pairs that are inserted asleep, we do not know the touch state. If they run through narrowphase and a touch is found,
			// then a managerNewTouch() call will inform this object about the found touch. However, if narrowphase detects that there 
			// is no touch, this object will not be informed about it. The low level manager will always know though. Now, before destroying
			// the pair manager, we need to record "does not have touch" state if available.
			raiseFlag(HAS_NO_TOUCH);
		}

		destroyManager();
	}

	//
	// We distinguish two scenarios here:
	//
	// A) island generation deactivates objects:
	//    -> the deactivated body was active
	//    -> narrowphase ran on this pair
	//    -> the touch status is known
	//      -> touch:    the objects of the pair are in the same island
	//      -> no touch: the objects of the pair are in different islands
	//
	//    As a consequence, the edge state is not changed. The assumption is that anything that could break the touch status
	//    from here on will have to mark the edges connected (for example if the object gets moved).
	//
	// B) user deactivates objects:
	//    -> the touch status might not be known (for example, the pose gets integrated after the solver which might cause a change
	//       in touch status. If the object gets put to sleep after that, we have to be conservative and mark the edge connected.
	//       other example: an active objects gets moved by the user and then deactivated).
	//
	if (!(infoFlag & Actor::AS_PART_OF_ISLAND_GEN) && (!readIntFlag(CONTACTS_RESPONSE_DISABLED)) )
	{
		getScene().getInteractionScene().getLLIslandManager().setEdgeConnected(mLLIslandHook);
	}

	RbElementInteraction::onDeactivate(infoFlag);

	return true;
}


void Sc::ShapeInstancePairLL::createManager()
{
	Sc::Scene& scene = getScene();

	PxsMaterialManager* materialManager = scene.getMaterialManager();

	ShapeSim& shapeSim0 = getShape0();
	ShapeSim& shapeSim1 = getShape1();

	PxActorType::Enum type0 = shapeSim0.getActorSim().getActorType(), type1 = shapeSim1.getActorSim().getActorType();

	const int disableResponse = readIntFlag(CONTACTS_RESPONSE_DISABLED) ? 1 : 0;
	const int disableDiscreteContact = !(getPairFlags() & PxPairFlag::eDETECT_DISCRETE_CONTACT);
	const int disableCCDContact = !(getPairFlags() & PxPairFlag::eDETECT_CCD_CONTACT);
	const int reportContactInfo = (getPairFlags() & PxPairFlag::eNOTIFY_CONTACT_POINTS) || readIntFlag(CONTACTS_COLLECT_POINTS);
	const int hasForceThreshold = !disableResponse && (getPairFlags() & CONTACT_FORCE_THRESHOLD_PAIRS);
	int touching;
	if (readIntFlag(TOUCH_KNOWN))
		touching = readIntFlag(HAS_TOUCH) ? 1 : -1;
	else
		touching = 0;

	// Check if contact generation callback has been ordered on the pair
	int contactChangeable = 0;
	if (getPairFlags() & PxPairFlag::eMODIFY_CONTACTS)
		contactChangeable = 1;

	// Static actors are in dominance group zero and must remain there

	Sc::BodySim* bs0 = shapeSim0.getBodySim();
	Sc::BodySim* bs1 = shapeSim1.getBodySim();
	const PxDominanceGroup dom0 = bs0 ? bs0->getActorCore().getDominanceGroup() : (PxDominanceGroup)0;
	const PxDominanceGroup dom1 = bs1 ? bs1->getActorCore().getDominanceGroup() : (PxDominanceGroup)0;

	const PxDominanceGroupPair cdom = scene.getDominanceGroupPair(dom0, dom1);

	PxsTransformCache& cache = scene.getInteractionScene().getLowLevelContext()->getTransformCache();

	shapeSim0.createTransformCache(cache);
	shapeSim1.createTransformCache(cache);

	PxvManagerDescRigidRigid managerDesc;
	managerDesc.restDistance			= shapeSim0.getRestOffset() + shapeSim1.getRestOffset();
	managerDesc.rigidBody0				= bs0 ? &bs0->getLowLevelBody() : NULL;
	managerDesc.rigidBody1				= bs1 ? &bs1->getLowLevelBody() : NULL;
	managerDesc.reportContactInfo		= reportContactInfo;
	managerDesc.hasForceThreshold		= hasForceThreshold;
	managerDesc.contactChangeable		= contactChangeable;
	managerDesc.disableResponse			= disableResponse;
	managerDesc.disableDiscreteContact  = disableDiscreteContact;
	managerDesc.disableCCDContact		= disableCCDContact;
	managerDesc.dominance0				= cdom.dominance0;
	managerDesc.dominance1				= cdom.dominance1;
	managerDesc.shapeCore0				= &shapeSim0.getCore().getCore();
	managerDesc.shapeCore1				= &shapeSim1.getCore().getCore();

	managerDesc.hasArticulations		= int((type0 == PxActorType::eARTICULATION_LINK) | PxU32(type1 == PxActorType::eARTICULATION_LINK)<<1);
	managerDesc.hasDynamics				= int((type0 != PxActorType::eRIGID_STATIC)      | PxU32(type1 != PxActorType::eRIGID_STATIC)<<1);
	managerDesc.hasTouch				= touching;

	managerDesc.rigidCore0				= &shapeSim0.getPxsRigidCore();
	managerDesc.rigidCore1				= &shapeSim1.getPxsRigidCore();

	managerDesc.transformCache0			= shapeSim0.getTransformCacheID();
	managerDesc.transformCache1			= shapeSim1.getTransformCacheID();

	managerDesc.userData				= this;

	mManager = scene.getInteractionScene().getLowLevelContext()->createContactManager(managerDesc, materialManager);

	if (mManager)
		scene.getInteractionScene().getLLIslandManager().setEdgeRigidCM(mLLIslandHook, mManager);
}


void Sc::ShapeInstancePairLL::onPoseChangedWhileSleeping()
{
	//
	// if an operation that can break touch status occurs, all deactivated pairs need to set the sleep island edge
	// to connected to make sure that potentially joined islands get detected once parts of the island wake up.
	// Active interactions can be ignored because the edges of those will be marked connected on deactivation.
	//
	if (!mManager)
	{
		Scene& scene = getScene();

		if(!readIntFlag(CONTACTS_RESPONSE_DISABLED))
		{
			scene.getInteractionScene().getLLIslandManager().setEdgeConnected(mLLIslandHook);
		}

		if (!readIntFlag(TOUCH_KNOWN))
		{
			// conservative approach: if a pair was added asleep, and the bodies get moved, we want to check next frame
			// whether the other body should get woken up.
			BodySim* body0 = getShape0().getBodySim();
			BodySim* body1 = getShape1().getBodySim();
			if (body0 && body1 && !readIntFlag(ShapeInstancePairLL::CONTACTS_RESPONSE_DISABLED) )
				scene.addToLostTouchList(body0, body1);
		}
	}
}


void Sc::ShapeInstancePairLL::postNarrowPhaseSecondPass(PxsIslandManager& islandManager, bool connect)
{
	PX_ASSERT(mManager);
	PX_ASSERT(mLLIslandHook.isManaged());

	if (connect)
	{
		adjustCountersOnNewTouch();
		if(!readIntFlag(CONTACTS_RESPONSE_DISABLED))
		{
			islandManager.setEdgeConnected(mLLIslandHook);
		}
	}
	else
	{
		adjustCountersOnLostTouch();
		islandManager.setEdgeUnconnected(mLLIslandHook);
	}
}


void Sc::ShapeInstancePairLL::postIslandGenSecondPass()
{
	PX_ASSERT(mManager);
	PX_ASSERT(getScene().getInteractionScene().isActiveInteraction(this));

	if (isReportPair())
	{
		// for pairs that got processed in the second island gen pass, the check for re-insertion into the persistent report list
		// was postponed. It can be done now because it is clear that the pair stays awake now.
		processReportPairOnActivate();
	}

	if (mManager->getTouchStatus())
	{
		PX_ASSERT(getScene().getInteractionScene().getLLIslandManager().getIsEdgeConnected(mLLIslandHook));

		if (!hasTouch())
			managerNewTouch(0, false);
		else if (getPairFlags() & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
		{
			PX_ASSERT(readIntFlag(IS_IN_PERSISTENT_EVENT_LIST)); // should happen as part of the pair activation
			processUserNotification(PxPairFlag::eNOTIFY_TOUCH_PERSISTS, 0, false, 0);
		}
	}
	else
	{
		PX_ASSERT(!getScene().getInteractionScene().getLLIslandManager().getIsEdgeConnected(mLLIslandHook));

		if (hasTouch())
		{
			if (managerLostTouch(0, false) && !readIntFlag(ShapeInstancePairLL::CONTACTS_RESPONSE_DISABLED) )
				getScene().addToLostTouchList(getShape0().getBodySim(), getShape1().getBodySim());
		}
	}
}
