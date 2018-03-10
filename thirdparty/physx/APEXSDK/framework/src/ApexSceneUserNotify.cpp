/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexDefs.h"
#if NX_SDK_VERSION_MAJOR == 2

#include "ApexSceneUserNotify.h"
#include <NxJoint.h>
#include <NxScene.h>

namespace physx
{
namespace apex
{

ApexSceneUserNotify::~ApexSceneUserNotify(void)
{
	// All callbacks should have been removed by now... something is wrong.
	PX_ASSERT(mModuleNotifiers.size() == 0);
}

void ApexSceneUserNotify::addModuleNotifier(NxUserNotify& notify)
{
	mModuleNotifiers.pushBack(&notify);
}

void ApexSceneUserNotify::removeModuleNotifier(NxUserNotify& notify)
{
	const PxU32 numNotifiers = mModuleNotifiers.size();
	PxU32 found = numNotifiers;
	for (PxU32 i = 0; i < numNotifiers; i++)
	{
		if (mModuleNotifiers[i] == &notify)
		{
			found = i;
			break;
		}
	}
	PX_ASSERT(found < numNotifiers);
	if (found < numNotifiers)
	{
		mModuleNotifiers.replaceWithLast(found);
	}
}

void ApexSceneUserNotify::playBatchedNotifications()
{

	for (PxU32 i = 0; i < mBatchedBreakNotifications.size(); i++)
	{
		BatchedBreakNotification& notify = mBatchedBreakNotifications[i];
		PX_ASSERT(mAppNotify != NULL);
		bool releaseJoint = mAppNotify->onJointBreak(notify.breakingImpulse, *notify.breakingJoint) || notify.allowedRelease;

		if (releaseJoint)
		{
			// scene isn't running anymore, guess we need to release the joint by hand.
			notify.breakingJoint->getScene().releaseJoint(*notify.breakingJoint);
		}
	}

	// release if the array is too big
	if (mBatchedBreakNotifications.size() * 4 < mBatchedBreakNotifications.capacity())
	{
		mBatchedBreakNotifications.shrink();
	}

	mBatchedBreakNotifications.clear();

	for (PxU32 i = 0; i < mBatchedSleepWakeEventBorders.size(); i++)
	{
		PX_ASSERT(mAppNotify != NULL);
		const SleepWakeBorders border = mBatchedSleepWakeEventBorders[i];
		if (border.sleepEvents)
		{
			mAppNotify->onSleep(&mBatchedSleepEvents[border.start], border.count);
		}
		else
		{
			mAppNotify->onWake(&mBatchedWakeEvents[border.start], border.count);
		}
	}
	mBatchedSleepWakeEventBorders.clear();
	mBatchedSleepEvents.clear();
	mBatchedWakeEvents.clear();
}

bool ApexSceneUserNotify::onJointBreak(PxF32 breakingImpulse, NxJoint& brokenJoint)
{
	bool releaseJoint = false;
	for (Array<NxUserNotify*>::Iterator curr = mModuleNotifiers.begin(); curr != mModuleNotifiers.end(); ++curr)
	{
		releaseJoint |= (*curr)->onJointBreak(breakingImpulse, brokenJoint);
	}

	if (mAppNotify != NULL)
	{
		if (mBatchAppNotify)
		{
			mBatchedBreakNotifications.pushBack(BatchedBreakNotification(breakingImpulse, brokenJoint, releaseJoint));

			// only release when replaying the batched notifications.
			releaseJoint = false;
		}
		else
		{
			releaseJoint |= mAppNotify->onJointBreak(breakingImpulse, brokenJoint);
		}
	}
	return releaseJoint;
}

void ApexSceneUserNotify::onWake(NxActor** actors, PxU32 count)
{
	for (Array<NxUserNotify*>::Iterator curr = mModuleNotifiers.begin(); curr != mModuleNotifiers.end(); ++curr)
	{
		(*curr)->onWake(actors, count);
	}

	if (mAppNotify != NULL)
	{
		if (mBatchAppNotify)
		{
			SleepWakeBorders border(mBatchedWakeEvents.size(), count, false);
			mBatchedWakeEvents.resize(mBatchedWakeEvents.size() + count);
			for (PxU32 i = 0; i < count; i++)
			{
				mBatchedWakeEvents.pushBack(actors[i]);
			}
		}
		else
		{
			mAppNotify->onWake(actors, count);
		}
	}
}

void ApexSceneUserNotify::onSleep(NxActor** actors, PxU32 count)
{
	for (Array<NxUserNotify*>::Iterator curr = mModuleNotifiers.begin(); curr != mModuleNotifiers.end(); ++curr)
	{
		(*curr)->onSleep(actors, count);
	}
	if (mAppNotify)
	{
		if (mBatchAppNotify)
		{
			SleepWakeBorders border(mBatchedSleepEvents.size(), count, true);
			mBatchedSleepEvents.resize(mBatchedSleepEvents.size() + count);
			for (PxU32 i = 0; i < count; i++)
			{
				mBatchedSleepEvents.pushBack(actors[i]);
			}
		}
		else
		{
			mAppNotify->onSleep(actors, count);
		}
	}
}











ApexSceneUserContactReport::ApexSceneUserContactReport(void) : mAppReport(NULL), mBatchAppNotify(false)
{
}

ApexSceneUserContactReport::~ApexSceneUserContactReport(void)
{
	// All callbacks should have been removed by now... something is wrong.
	PX_ASSERT(mModuleReport.size() == 0);
}

void ApexSceneUserContactReport::addModuleReport(NxUserContactReport& report)
{
	mModuleReport.pushBack(&report);
}

void ApexSceneUserContactReport::removeModuleReport(NxUserContactReport& report)
{
	const PxU32 numReports = mModuleReport.size();
	PxU32       found      = numReports;
	for (PxU32 i = 0; i < numReports; i++)
	{
		if (mModuleReport[i] == &report)
		{
			found = i;
			break;
		}
	}
	PX_ASSERT(found < numReports);
	if (found < numReports)
	{
		mModuleReport.replaceWithLast(found);
	}
}

void ApexSceneUserContactReport::setApplicationReport(NxUserContactReport* report)
{
	mAppReport = report;
}

class ApexContactStreamIterator : public NxContactStreamIterator
{
public:
	ApexContactStreamIterator( NxConstContactStream streamIt) : NxContactStreamIterator(streamIt) { }

	NxConstContactStream getStreamIt()	{	return stream; }
};

PxU32 ApexSceneUserContactReport::getContactStreamLength(NxConstContactStream stream)
{
	if (stream == NULL)
	{
		return 0;
	}

	ApexContactStreamIterator it(stream);

	while (it.goNextPair()) // user can call getNumPairs() here
	{
		while (it.goNextPatch()) // user can also call getShape(), isDeletedShape() and getNumPatches() here
		{
			while (it.goNextPoint()) //user can also call getPatchNormal() and getNumPoints() here
			{
				//user can also call getPoint() and getSeparation() here
			}
		}
	}
	return PxU32(it.getStreamIt() - stream);
}


void ApexSceneUserContactReport::playBatchedNotifications()
{
	for (PxU32 i = 0; i < mBatchedContactPairs.size(); i++)
	{
		PX_ASSERT(mAppReport != NULL);
		NxContactPair pair = mBatchedContactPairs[i].pair;
		const PxU32 events = mBatchedContactPairs[i].events;
		PX_ASSERT(mBatchedContactPairs[i].streamStart < mBatchedContactStreams.size());
		pair.stream = mBatchedContactStreams.begin() + mBatchedContactPairs[i].streamStart;

#if 0
		// sanity
		NxContactStreamIterator it(pair.stream);

		while (it.goNextPair()) // user can call getNumPairs() here
		{
			while (it.goNextPatch()) // user can also call getShape(), isDeletedShape() and getNumPatches() here
			{
				while (it.goNextPoint()) //user can also call getPatchNormal() and getNumPoints() here
				{
					//user can also call getPoint() and getSeparation() here
				}
			}
		}
#endif
		mAppReport->onContactNotify(pair, events);
	}

	mBatchedContactPairs.clear();
	mBatchedContactStreams.clear();
}


void ApexSceneUserContactReport::onContactNotify(NxContactPair& pair, PxU32 events)
{
	for (Array<NxUserContactReport*>::Iterator curr = mModuleReport.begin(); curr != mModuleReport.end(); curr++)
	{
		(*curr)->onContactNotify(pair, events);
	}
	if (mAppReport != NULL)
	{
		if (mBatchAppNotify)
		{
			ContactStreamEntry entry;
			entry.pair = pair;
			entry.events = events;
			entry.streamStart = mBatchedContactStreams.size();

			mBatchedContactPairs.pushBack(entry);

			const PxU32 length = getContactStreamLength(pair.stream);
			for (PxU32 i = 0; i < length; i++)
			{
				mBatchedContactStreams.pushBack(pair.stream[i]);
			}
		}
		else
		{
			mAppReport->onContactNotify(pair, events);
		}
	}
}











ApexSceneUserContactModify::ApexSceneUserContactModify(void)
{
	mAppContactModify = 0;
}

ApexSceneUserContactModify::~ApexSceneUserContactModify(void)
{
	// All callbacks should have been removed by now... something is wrong.
	PX_ASSERT(mModuleContactModify.size() == 0);
}

void ApexSceneUserContactModify::addModuleContactModify(NxUserContactModify& contactModify)
{
	mModuleContactModify.pushBack(&contactModify);
}

void ApexSceneUserContactModify::removeModuleContactModify(NxUserContactModify& contactModify)
{
	const PxU32 numContactModifies = mModuleContactModify.size();
	PxU32       found      = numContactModifies;
	for (PxU32 i = 0; i < numContactModifies; i++)
	{
		if (mModuleContactModify[i] == &contactModify)
		{
			found = i;
			break;
		}
	}
	PX_ASSERT(found < numContactModifies);
	if (found < numContactModifies)
	{
		mModuleContactModify.replaceWithLast(found);
	}
}

void ApexSceneUserContactModify::setApplicationContactModify(NxUserContactModify* contactModify)
{
	mAppContactModify = contactModify;
}

bool ApexSceneUserContactModify::onContactConstraint(
    NxU32& changeFlags,
    const NxShape* shape0,
    const NxShape* shape1,
    const NxU32 featureIndex0,
    const NxU32 featureIndex1,
    NxContactCallbackData& data)
{
	bool keepContact = true;
	for (Array<NxUserContactModify*>::Iterator curr = mModuleContactModify.begin(); curr != mModuleContactModify.end(); curr++)
	{
		keepContact &= (*curr)->onContactConstraint(changeFlags, shape0, shape1, featureIndex0, featureIndex1, data);
	}
	if (mAppContactModify)
	{
		keepContact &= mAppContactModify->onContactConstraint(changeFlags, shape0, shape1, featureIndex0, featureIndex1, data);
	}
	return keepContact;
}







void ApexSceneUserTriggerReport::playBatchedNotifications()
{
	for (PxU32 i = 0; i < mBatchedTriggerReports.size(); i++)
	{
		PX_ASSERT(mAppTriggerReport != NULL);
		BatchedTriggerEntry& entry = mBatchedTriggerReports[i];
		mAppTriggerReport->onTrigger(*entry.triggerShape, *entry.otherShape, entry.status);
	}
	mBatchedTriggerReports.clear();
}

void ApexSceneUserTriggerReport::onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
{
	if (mAppTriggerReport != NULL)
	{
		if (mBatchAppNotify)
		{
			mBatchedTriggerReports.pushBack(BatchedTriggerEntry(triggerShape, otherShape, status));
		}
		else
		{
			mAppTriggerReport->onTrigger(triggerShape, otherShape, status);
		}
	}
}






void ApexSceneUserFluidNotify::playBatchedNotifications()
{
	for (PxU32 i = 0; i < mBatchedEmitterEvents.size(); i++)
	{
		PX_ASSERT(mAppNotify != NULL);
		NxFluidEmitter* emitter = mBatchedEmitterEvents[i].emitter;
		bool release = mAppNotify->onEmitterEvent(*emitter, mBatchedEmitterEvents[i].eventType);
		if (release)
		{
			emitter->getFluid().releaseEmitter(*emitter);
		}
	}
	mBatchedEmitterEvents.clear();

	for (PxU32 i = 0; i < mBatchedFluidEvents.size(); i++)
	{
		PX_ASSERT(mAppNotify != NULL);
		NxFluid* fluid = mBatchedFluidEvents[i].fluid;
		bool release = mAppNotify->onEvent(*fluid, mBatchedFluidEvents[i].eventType);
		if (release)
		{
			fluid->getScene().releaseFluid(*fluid);
		}
	}
	mBatchedFluidEvents.clear();
}

bool ApexSceneUserFluidNotify::onEmitterEvent(NxFluidEmitter& emitter, NxFluidEmitterEventType eventType)
{
	if (mAppNotify != NULL)
	{
		if (mBatchAppNotify)
		{
			mBatchedEmitterEvents.pushBack(EmitterEventEntry(emitter, eventType));
		}
		else
		{
			return mAppNotify->onEmitterEvent(emitter, eventType);
		}
	}
	return false; // keep the thing
}

bool ApexSceneUserFluidNotify::onEvent(NxFluid& fluid, NxFluidEventType eventType)
{
	if (mAppNotify != NULL)
	{
		if (mBatchAppNotify)
		{
			mBatchedFluidEvents.pushBack(FluidEventEntry(fluid, eventType));
		}
		else
		{
			return mAppNotify->onEvent(fluid, eventType);
		}
	}
	return false; // keep the thing
}

}
} // namespace physx::apex


#elif NX_SDK_VERSION_MAJOR == 3

#pragma warning(push)
#pragma warning(disable: 4324)

#include <ApexSceneUserNotify.h>
#include "foundation/PxPreprocessor.h"
#include <PxJoint.h>
#include <PxScene.h>

namespace physx
{
namespace apex
{

ApexSceneUserNotify::~ApexSceneUserNotify(void)
{
	// All callbacks should have been removed by now... something is wrong.
	PX_ASSERT(mModuleNotifiers.size() == 0);
}

void ApexSceneUserNotify::addModuleNotifier(PxSimulationEventCallback& notify)
{
	mModuleNotifiers.pushBack(&notify);
}

void ApexSceneUserNotify::removeModuleNotifier(PxSimulationEventCallback& notify)
{
	const PxU32 numNotifiers = mModuleNotifiers.size();
	PxU32 found = numNotifiers;
	for (PxU32 i = 0; i < numNotifiers; i++)
	{
		if (mModuleNotifiers[i] == &notify)
		{
			found = i;
			break;
		}
	}
	PX_ASSERT(found < numNotifiers);
	if (found < numNotifiers)
	{
		mModuleNotifiers.replaceWithLast(found);
	}
}

void ApexSceneUserNotify::playBatchedNotifications()
{
#if TODO_HANDLE_NEW_CONTACT_STREAM
	// onConstraintBreak
	{
		for (PxU32 i = 0; i < mBatchedBreakNotifications.size(); i++)
		{
			PxConstraintInfo& constraintInfo = mBatchedBreakNotifications[i];
			PX_ASSERT(mAppNotify != NULL);
			mAppNotify->onConstraintBreak(&constraintInfo, 1);

			/*
			// apan, shold we release joint? how?
			if (releaseJoint)
			{
			// scene isn't running anymore, guess we need to release the joint by hand.
			notify.breakingJoint->getScene().releaseJoint(*notify.breakingJoint);
			}
			*/
		}
		// release if the array is too big
		if (mBatchedBreakNotifications.size() * 4 < mBatchedBreakNotifications.capacity())
		{
			mBatchedBreakNotifications.shrink();
		}

		mBatchedBreakNotifications.clear();
	}


	// onContact
	{
		for (PxU32 i = 0; i < mBatchedContactNotifications.size(); i++)
		{
			BatchedContactNotification& contact = mBatchedContactNotifications[i];
			PX_ASSERT(contact.batchedStreamStart < mBatchedContactStreams.size());
			contact.batchedPair.stream = (PxConstContactStream)(mBatchedContactStreams.begin() + contact.batchedStreamStart);

			mAppNotify->onContact(contact.batchedPair, contact.batchedEvents);
		}
		mBatchedContactNotifications.clear();
		mBatchedContactStreams.clear();
	}

	// onSleep/onWake
	{
		for (PxU32 i = 0; i < mBatchedSleepWakeEventBorders.size(); i++)
		{
			const SleepWakeBorders border = mBatchedSleepWakeEventBorders[i];
			if (border.sleepEvents)
			{
				mAppNotify->onSleep(&mBatchedSleepEvents[border.start], border.count);
			}
			else
			{
				mAppNotify->onWake(&mBatchedWakeEvents[border.start], border.count);
			}
		}
		mBatchedSleepWakeEventBorders.clear();
		mBatchedSleepEvents.clear();
		mBatchedWakeEvents.clear();
	}

	// mBatchedTriggerReports
	{
		for (PxU32 i = 0; i < mBatchedTriggerReports.size(); i++)
		{
			PxTriggerPair& triggerPair = mBatchedTriggerReports[i];
			mAppNotify->onTrigger(&triggerPair, 1);
		}
		mBatchedTriggerReports.clear();
	}
#endif
}

void ApexSceneUserNotify::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
{
	for (Array<PxSimulationEventCallback*>::Iterator curr = mModuleNotifiers.begin(); curr != mModuleNotifiers.end(); ++curr)
	{
		(*curr)->onConstraintBreak(constraints, count);
	}

	if (mAppNotify != NULL)
	{
		if (mBatchAppNotify)
		{
			for (PxU32 i = 0 ; i < count; i++)
			{
				mBatchedBreakNotifications.pushBack(constraints[i]);
			}
		}
		else
		{
			mAppNotify->onConstraintBreak(constraints, count);
		}
	}
}

void ApexSceneUserNotify::onWake(PxActor** actors, PxU32 count)
{
	for (Array<PxSimulationEventCallback*>::Iterator curr = mModuleNotifiers.begin(); curr != mModuleNotifiers.end(); ++curr)
	{
		(*curr)->onWake(actors, count);
	}

	if (mAppNotify != NULL)
	{
		if (mBatchAppNotify)
		{
			SleepWakeBorders border(mBatchedWakeEvents.size(), count, false);
			mBatchedSleepWakeEventBorders.pushBack(border);
			mBatchedWakeEvents.resize(mBatchedWakeEvents.size() + count);
			for (PxU32 i = 0; i < count; i++)
			{
				mBatchedWakeEvents.pushBack(actors[i]);
			}
		}
		else
		{
			mAppNotify->onWake(actors, count);
		}
	}
}

void ApexSceneUserNotify::onSleep(PxActor** actors, PxU32 count)
{
	for (Array<PxSimulationEventCallback*>::Iterator curr = mModuleNotifiers.begin(); curr != mModuleNotifiers.end(); ++curr)
	{
		(*curr)->onSleep(actors, count);
	}
	if (mAppNotify)
	{
		if (mBatchAppNotify)
		{
			SleepWakeBorders border(mBatchedSleepEvents.size(), count, true);
			mBatchedSleepWakeEventBorders.pushBack(border);
			mBatchedSleepEvents.resize(mBatchedSleepEvents.size() + count);
			for (PxU32 i = 0; i < count; i++)
			{
				mBatchedSleepEvents.pushBack(actors[i]);
			}
		}
		else
		{
			mAppNotify->onSleep(actors, count);
		}
	}
}


void ApexSceneUserNotify::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	for (Array<PxSimulationEventCallback*>::Iterator curr = mModuleNotifiers.begin(); curr != mModuleNotifiers.end(); ++curr)
	{
		(*curr)->onContact(pairHeader, pairs, nbPairs);
	}

	if (mAppNotify)
	{
		if (mBatchAppNotify)
		{
#if TODO_HANDLE_NEW_CONTACT_STREAM
			mBatchedContactNotifications.pushBack(BatchedContactNotification(pairHeader, pairs, nbPairs));
			const PxU32 length = pair.contactCount; //getContactStreamLength(pair.stream);
			for (PxU32 i = 0; i < length; i++)
			{
				mBatchedContactStreams.pushBack(pair.stream[i]);
			}
#endif
		}
		else
		{
			mAppNotify->onContact(pairHeader, pairs, nbPairs);
		}
	}
}


#if TODO_HANDLE_NEW_CONTACT_STREAM
class ApexContactStreamIterator : public PxContactStreamIterator
{
public:
	ApexContactStreamIterator( PxConstContactStream streamIt) : PxContactStreamIterator(streamIt) { }

	PxConstContactStream getStreamIt()	{	return streamIt; }
};
#endif

void ApexSceneUserNotify::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	if (mAppNotify != NULL)
	{
		if (mBatchAppNotify)
		{
			for (PxU32 i = 0; i < count; i++)
			{
				mBatchedTriggerReports.pushBack(pairs[count]);
			}
		}
		else
		{
			mAppNotify->onTrigger(pairs, count);
		}
	}
}







ApexSceneUserContactModify::ApexSceneUserContactModify(void)
{
	mAppContactModify = 0;
}

ApexSceneUserContactModify::~ApexSceneUserContactModify(void)
{
	// All callbacks should have been removed by now... something is wrong.
	PX_ASSERT(mModuleContactModify.size() == 0);
}

void ApexSceneUserContactModify::addModuleContactModify(PxContactModifyCallback& contactModify)
{
	mModuleContactModify.pushBack(&contactModify);
}

void ApexSceneUserContactModify::removeModuleContactModify(PxContactModifyCallback& contactModify)
{
	const PxU32 numContactModifies = mModuleContactModify.size();
	PxU32       found      = numContactModifies;
	for (PxU32 i = 0; i < numContactModifies; i++)
	{
		if (mModuleContactModify[i] == &contactModify)
		{
			found = i;
			break;
		}
	}
	PX_ASSERT(found < numContactModifies);
	if (found < numContactModifies)
	{
		mModuleContactModify.replaceWithLast(found);
	}
}

void ApexSceneUserContactModify::setApplicationContactModify(PxContactModifyCallback* contactModify)
{
	mAppContactModify = contactModify;
}

void ApexSceneUserContactModify::onContactModify(PxContactModifyPair* const pairs, PxU32 count)
{
	for (Array<PxContactModifyCallback*>::Iterator curr = mModuleContactModify.begin(); curr != mModuleContactModify.end(); curr++)
	{
		(*curr)->onContactModify(pairs, count);
	}
	if (mAppContactModify)
	{
		mAppContactModify->onContactModify(pairs, count);
	}
}



}
} // namespace physx::apex
#pragma warning(pop)

#endif // NX_SDK_VERSION_MAJOR == 2