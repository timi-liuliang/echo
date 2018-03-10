/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_SCENE_USER_NOTIFY_H
#define APEX_SCENE_USER_NOTIFY_H

#include "NxApexDefs.h"
#if NX_SDK_VERSION_MAJOR == 2

#include <NxUserNotify.h>
#include <NxUserContactReport.h>
#include <NxFluidUserNotify.h>

#include "foundation/PxSimpleTypes.h"
#include <PsArray.h>
#include <PsShare.h>


namespace physx
{
namespace apex
{

class ApexSceneUserNotify : public NxUserNotify
{
public:
	ApexSceneUserNotify() : mAppNotify(NULL), mBatchAppNotify(false) {}
	virtual ~ApexSceneUserNotify();

	void addModuleNotifier(NxUserNotify& notify);
	void removeModuleNotifier(NxUserNotify& notify);

	void setApplicationNotifier(NxUserNotify* notify)
	{
		mAppNotify = notify;
	}
	NxUserNotify* getApplicationNotifier() const
	{
		return mAppNotify;
	}

	void setBatchAppNotify(bool enable)
	{
		mBatchAppNotify = enable;
	}
	void playBatchedNotifications();

private:
	// from NxUserNotify
	virtual bool onJointBreak(PxF32 breakingImpulse, NxJoint& brokenJoint);
	virtual void onWake(NxActor** actors, PxU32 count);
	virtual void onSleep(NxActor** actors, PxU32 count);

private:
	Array<NxUserNotify*>				mModuleNotifiers;
	NxUserNotify*						mAppNotify;


	// for batch notification
	bool								mBatchAppNotify;

	struct BatchedBreakNotification
	{
		BatchedBreakNotification(PxF32 impulse, NxJoint& joint, bool release)
		{
			breakingImpulse = impulse;
			breakingJoint = &joint;
			allowedRelease = release;
		}
		PxF32 breakingImpulse;
		NxJoint* breakingJoint;
		bool allowedRelease;
	};
	Array<BatchedBreakNotification>		mBatchedBreakNotifications;

	struct SleepWakeBorders
	{
		SleepWakeBorders(PxU32 s, PxU32 c, bool sleep) : start(s), count(c), sleepEvents(sleep) {}
		PxU32 start;
		PxU32 count;
		bool sleepEvents;
	};
	Array<SleepWakeBorders>				mBatchedSleepWakeEventBorders;
	Array<NxActor*>						mBatchedSleepEvents;
	Array<NxActor*>						mBatchedWakeEvents;
};



class ApexSceneUserContactReport : public NxUserContactReport
{
public:
	ApexSceneUserContactReport();
	virtual ~ApexSceneUserContactReport();

	void addModuleReport(NxUserContactReport& report);
	void removeModuleReport(NxUserContactReport& report);

	void setApplicationReport(NxUserContactReport* report);
	NxUserContactReport* getApplicationReport() const
	{
		return mAppReport;
	}

	PxU32 getContactStreamLength(NxConstContactStream stream);
	void setBatchAppNotify(bool enable)
	{
		mBatchAppNotify = enable;
	}
	void playBatchedNotifications();

private:
	// from NxUserContactReport
	virtual void onContactNotify(NxContactPair& pair, PxU32 events);

private:
	Array<NxUserContactReport*>	mModuleReport;
	NxUserContactReport*        mAppReport;

	// for batch notification
	bool						mBatchAppNotify;

	struct ContactStreamEntry
	{
		NxContactPair pair;
		PxU32 events;
		PxU32 streamStart;
	};
	Array<ContactStreamEntry> mBatchedContactPairs;
	Array<PxU32> mBatchedContactStreams;
};

class ApexSceneUserContactModify : public NxUserContactModify
{
public:
	ApexSceneUserContactModify();
	virtual ~ApexSceneUserContactModify();

	void addModuleContactModify(NxUserContactModify& contactModify);
	void removeModuleContactModify(NxUserContactModify& contactModify);

	void setApplicationContactModify(NxUserContactModify* contactModify);
	NxUserContactModify* getApplicationContactModify() const
	{
		return mAppContactModify;
	}

private:
	// from NxUserContactModify
	virtual bool onContactConstraint(
	    NxU32& changeFlags,
	    const NxShape* shape0,
	    const NxShape* shape1,
	    const NxU32 featureIndex0,
	    const NxU32 featureIndex1,
	    NxContactCallbackData& data);

private:
	Array<NxUserContactModify*>	mModuleContactModify;
	NxUserContactModify*	   	mAppContactModify;
};

class ApexSceneUserTriggerReport : public NxUserTriggerReport
{
public:
	ApexSceneUserTriggerReport() : mAppTriggerReport(NULL), mBatchAppNotify(false) {}
	virtual ~ApexSceneUserTriggerReport() {}

	void setApplicationReport(NxUserTriggerReport* report)
	{
		mAppTriggerReport = report;
	}
	NxUserTriggerReport* getApplicationReport(void) const
	{
		return mAppTriggerReport;
	}

	void setBatchAppNotify(bool enable)
	{
		mBatchAppNotify = enable;
	}
	void playBatchedNotifications();

private:
	// from NxUserTriggerReport
	virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status);

private:
	NxUserTriggerReport* mAppTriggerReport;

	bool mBatchAppNotify;

	struct BatchedTriggerEntry
	{
		BatchedTriggerEntry(NxShape& ts, NxShape& os, NxTriggerFlag stat) : triggerShape(&ts), otherShape(&os), status(stat) {}
		NxShape* triggerShape;
		NxShape* otherShape;
		NxTriggerFlag status;
	};
	Array<BatchedTriggerEntry> mBatchedTriggerReports;
};

class ApexSceneUserFluidNotify : public NxFluidUserNotify
{
public:
	ApexSceneUserFluidNotify() : mAppNotify(NULL), mBatchAppNotify(false) {}
	virtual ~ApexSceneUserFluidNotify() {}

	void setApplicationNotifier(NxFluidUserNotify* notify)
	{
		mAppNotify = notify;
	}
	NxFluidUserNotify* getApplicationNotifier() const
	{
		return mAppNotify;
	}

	void setBatchAppNotify(bool enable)
	{
		mBatchAppNotify = enable;
	}
	void playBatchedNotifications();

private:
	// from NxFluidUserNotify
	virtual bool onEmitterEvent(NxFluidEmitter& emitter, NxFluidEmitterEventType eventType);
	virtual bool onEvent(NxFluid& fluid, NxFluidEventType eventType);

private:
	NxFluidUserNotify*					mAppNotify;

	// for batch notification
	bool								mBatchAppNotify;

	struct EmitterEventEntry
	{
		EmitterEventEntry(NxFluidEmitter& em, NxFluidEmitterEventType et) : emitter(&em), eventType(et) {}
		NxFluidEmitter* emitter;
		NxFluidEmitterEventType eventType;
	};
	Array<EmitterEventEntry> mBatchedEmitterEvents;

	struct FluidEventEntry
	{
		FluidEventEntry(NxFluid& f, NxFluidEventType et) : fluid(&f), eventType(et) {}
		NxFluid* fluid;
		NxFluidEventType eventType;
	};
	Array<FluidEventEntry> mBatchedFluidEvents;
};

}
} // namespace physx::apex



#elif NX_SDK_VERSION_MAJOR == 3

#include <PxSimulationEventCallback.h>
#include <PxContactModifyCallback.h>

#include "foundation/PxSimpleTypes.h"
#include <PsArray.h>
#include <PsShare.h>


namespace physx
{
namespace apex
{

class ApexSceneUserNotify : public PxSimulationEventCallback
{
public:
	ApexSceneUserNotify() : mAppNotify(NULL), mBatchAppNotify(false) {}
	virtual ~ApexSceneUserNotify();

	void addModuleNotifier(physx::PxSimulationEventCallback& notify);
	void removeModuleNotifier(physx::PxSimulationEventCallback& notify);

	void setApplicationNotifier(physx::PxSimulationEventCallback* notify)
	{
		mAppNotify = notify;
	}
	PxSimulationEventCallback* getApplicationNotifier() const
	{
		return mAppNotify;
	}

	void setBatchAppNotify(bool enable)
	{
		mBatchAppNotify = enable;
	}
	void playBatchedNotifications();

private:
	// from PxSimulationEventCallback
	virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count);
	virtual void onWake(PxActor** actors, PxU32 count);
	virtual void onSleep(PxActor** actors, PxU32 count);
	virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
	virtual void onTrigger(PxTriggerPair* pairs, PxU32 count);

private:
	Array<physx::PxSimulationEventCallback*>	mModuleNotifiers;
	PxSimulationEventCallback*			mAppNotify;


	// for batch notification
	bool								mBatchAppNotify;

	// onConstraintBreak
	Array<physx::PxConstraintInfo>		mBatchedBreakNotifications;

	// onContact
	struct BatchedContactNotification
	{
		BatchedContactNotification(const PxContactPairHeader& _pairHeader, const PxContactPair* _pairs, PxU32 _nbPairs)
		{
			pairHeader			= _pairHeader;
			nbPairs				= _nbPairs;

			pairs = (PxContactPair *)PX_ALLOC(sizeof(PxContactPair) * nbPairs, PX_DEBUG_EXP("BatchedContactNotifications"));
			PX_ASSERT(pairs != NULL);
			for (PxU32 i=0; i<nbPairs; i++)
			{
				pairs[i] = _pairs[i];
			}
		}

		~BatchedContactNotification()
		{
			if (pairs)
			{
				PX_FREE(pairs);
				pairs = NULL;
			}
		}

		PxContactPairHeader pairHeader;
		PxContactPair *		pairs;
		PxU32				nbPairs;
	};
	Array<BatchedContactNotification>		mBatchedContactNotifications;
	Array<PxU32>							mBatchedContactStreams;

	// onWake/onSleep
	struct SleepWakeBorders
	{
		SleepWakeBorders(PxU32 s, PxU32 c, bool sleep) : start(s), count(c), sleepEvents(sleep) {}
		PxU32 start;
		PxU32 count;
		bool sleepEvents;
	};
	Array<SleepWakeBorders>				mBatchedSleepWakeEventBorders;
	Array<PxActor*>						mBatchedSleepEvents;
	Array<PxActor*>						mBatchedWakeEvents;

	// onTrigger
	Array<PxTriggerPair> mBatchedTriggerReports;
};


class ApexSceneUserContactModify : public PxContactModifyCallback
{
public:
	ApexSceneUserContactModify();
	virtual ~ApexSceneUserContactModify();

	void addModuleContactModify(physx::PxContactModifyCallback& contactModify);
	void removeModuleContactModify(physx::PxContactModifyCallback& contactModify);

	void setApplicationContactModify(physx::PxContactModifyCallback* contactModify);
	PxContactModifyCallback* getApplicationContactModify() const
	{
		return mAppContactModify;
	}

private:
	// from PxContactModifyCallback
	virtual void onContactModify(PxContactModifyPair* const pairs, PxU32 count);

private:
	Array<physx::PxContactModifyCallback*>	mModuleContactModify;
	PxContactModifyCallback*	   	mAppContactModify;
};

}
} // namespace physx::apex

#endif // NX_SDK_VERSION_MAJOR == 2

#endif