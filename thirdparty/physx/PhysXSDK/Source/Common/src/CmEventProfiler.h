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


#ifndef PX_PHYSICS_COMMON_PROFILE_EVENT_H
#define PX_PHYSICS_COMMON_PROFILE_EVENT_H

#ifndef PX_PROFILE_EVENT_PROFILE_THRESHOLD
#define PX_PROFILE_EVENT_PROFILE_THRESHOLD EventPriorities::Detail //default knob
#endif

#include "PxPhysXCommonConfig.h" // added for definition of PX_PHYSX_COMMON_API
#include "PxProfileCompileTimeEventFilter.h"
#include "PxProfileScopedEvent.h"
#include "PxProfileEventNames.h"
#include "PxProfileZone.h"
#include "PxProfileEventId.h"


#define PX_PROFILE_EVENT_DEFINITION_HEADER "CmProfileEventDefs.h"

	//Define all the event enumeration values as well as functions for creating the events.
#include "CmProfileDeclareEventInfo.h"

#include "CmNvToolsExtProfiler.h"

namespace physx
{
namespace Cm
{
	struct PX_UNIX_EXPORT CmEventNameProvider : public physx::PxProfileNameProvider
	{
		PX_PHYSX_COMMON_API physx::PxProfileNames getProfileNames() const;
	};

#define PX_PROFILE_BEGIN_SUBSYSTEM( subsys ) struct subsys {
#define PX_PROFILE_EVENT( subsys, name, priority ) static const physx::PxProfileEventId name; \
	PX_PHYSX_COMMON_API static const physx::PxProfileEventId& Get##name() { return name; }
#define PX_PROFILE_EVENT_DETAIL( subsys, name, priority ) PX_PROFILE_EVENT(subsys, name, priority )
#define PX_PROFILE_END_SUBSYSTEM( subsys ) };
	struct ProfileEventId
	{
#include "CmProfileEventDefs.h"	
	};
#undef PX_PROFILE_BEGIN_SUBSYSTEM
#undef PX_PROFILE_EVENT
#undef PX_PROFILE_EVENT_DETAIL
#undef PX_PROFILE_END_SUBSYSTEM

	class EventProfiler
	{
		physx::PxU64					mEventContext;
		physx::PxProfileEventSender*	mSDK;
#if PX_NVTX
		nvtx::EventHashMap				mCrossThreadEvents;
		physx::shdfnd::Mutex			mCrossThreadMutex;
#endif
	public:
		EventProfiler( physx::PxProfileEventSender* inSDK = NULL, physx::PxU64 inEventContext = 0 )
			: mEventContext( inEventContext )
			, mSDK( inSDK )
		{
		}
		EventProfiler( const EventProfiler& other )  { *this = other; }
		EventProfiler& operator=( const EventProfiler& other )
		{
			mEventContext = other.mEventContext;
			mSDK = other.mSDK;
			return *this;
		}
		physx::PxProfileEventSender* getProfileEventSender() { return mSDK; }
		physx::PxU64 getEventContext() const { return mEventContext; }
#if PX_NVTX		
		PX_PHYSX_COMMON_API	const char*	getStringFromId(PxU16 id);
		PX_FORCE_INLINE		nvtxRangeId_t		getCrossEvent(PxU16 id, PxU64 context) 
		{  
			physx::shdfnd::Mutex::ScopedLock lock(mCrossThreadMutex); 
			const physx::shdfnd::Pair<const nvtx::EventIdContextPair, PxU64>* e = mCrossThreadEvents.find(nvtx::EventIdContextPair(context, id));
			if(e)
				return e->second;
			else 
				return 0;
		};
		PX_PHYSX_COMMON_API	void		storeCrossEvent(PxU16 id, PxU64 context, nvtxRangeId_t crossEvent) 
		{ 
			physx::shdfnd::Mutex::ScopedLock lock(mCrossThreadMutex);
			mCrossThreadEvents[nvtx::EventIdContextPair(context, id)] =  crossEvent;
		}
#endif

	};
PX_PHYSX_COMMON_API void enableNVTXSupport(bool enable);
PX_PHYSX_COMMON_API bool isNVTXSupportEnabled();

}

template<bool TEnabled>
class CmProfileZone
{
	physx::PxProfileEventSender* mEventSender;
	physx::PxU16 mEventId;
	physx::PxU64 mEventContext;
public:
	template<typename TProfileDataProvider>
	CmProfileZone( TProfileDataProvider& inProvider, physx::PxU16 inEventId )
		: mEventSender( inProvider.getEventProfiler().getProfileEventSender() )
		, mEventId( inEventId )
		, mEventContext( inProvider.getEventProfiler().getEventContext() )
	{
		PX_ASSERT( mEventSender );
		mEventSender->startEvent( inEventId, mEventContext );
	}

	~CmProfileZone()
	{
		PX_ASSERT( mEventSender );
		mEventSender->stopEvent( mEventId, mEventContext );
	}
};

template<>
class CmProfileZone<false> 
{
public:
	template<typename TProfileDataProvider> CmProfileZone( TProfileDataProvider&, physx::PxU16) {}
};


template<bool TEnabled>
struct CmProfileValue
{
	template<typename TProfileDataProvider>
	CmProfileValue( TProfileDataProvider& inProvider, physx::PxU16 inEventId, physx::PxI64 theValue )
	{
		physx::PxProfileEventSender* theEventSender = inProvider.getEventProfiler().getProfileEventSender();
		physx::PxU64 theContext = inProvider.getEventProfiler().getEventContext();
		PX_ASSERT( theEventSender );
		theEventSender->eventValue( inEventId, theContext, theValue );
	}
};

template<> struct CmProfileValue<false>
{
	template<typename TProfileDataProvider> 
	CmProfileValue( TProfileDataProvider&, physx::PxU16, physx::PxI64 )
	{
	}
};

//---------------------------------------------------------------------------

inline physx::PxU64 getProfileEventContext() { return 0; }

#if !PX_IS_SPU && (defined(PX_CHECKED) || defined(PX_PROFILE) || defined(PX_DEBUG))

#define CM_PROFILE_START( _p, _id) \
	if(_p) {physx::profile::startEvent( _id.mCompileTimeEnabled, _p->getProfileEventSender(), _id, _p->getEventContext() ); \
	if(Cm::isNVTXSupportEnabled()) {NV_TEXT_PROFILE_START((*(_p)), _id); } }
#define CM_PROFILE_STOP( _p, _id)  \
	if(_p) {physx::profile::stopEvent( _id.mCompileTimeEnabled, _p->getProfileEventSender(), _id, _p->getEventContext() );  \
	if(Cm::isNVTXSupportEnabled()) { NV_TEXT_PROFILE_STOP((*(_p)), _id); } }


#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)

#define CM_PROFILE_ZONE( _p, _id) \
	physx::profile::DynamicallyEnabledScopedEvent<PxProfileEventSender> CONCAT(scopedEvent,__LINE__)( _p.getProfileEventSender(), _id, _p.getEventContext() ); \
	if(Cm::isNVTXSupportEnabled()) { NV_TEXT_PROFILE_ZONE( _p, _id); }
	
#define CM_PROFILE_ZONE_WITH_SUBSYSTEM( _p, subsystem, eventId )  \
	CmProfileZone<PX_PROFILE_EVENT_FILTER_VALUE(subsystem,eventId)> __zone##eventId( _p, physx::profile::EventIds::subsystem##eventId ); \
	if(Cm::isNVTXSupportEnabled()) { NV_TEXT_PROFILE_ZONE_WITH_SUBSYSTEM( _p, subsystem, eventId ); }

#define CM_PROFILE_VALUE( _p, subsystem, eventId, value ) \
	CmProfileValue<PX_PROFILE_EVENT_FILTER_VALUE(subsystem,eventId)> __val( _p, physx::profile::EventIds::subsystem##eventId, static_cast<PxI64>( value ) ); \
if(Cm::isNVTXSupportEnabled()) { NV_TEXT_PROFILE_VALUE( _p, subsystem, eventId, value ); }

// there is just one filtering option for all tasks now
#define CM_PROFILE_TASK_ZONE(_p, _id) CM_PROFILE_ZONE( _p, _id )

#define CM_PROFILE_START_CROSSTHREAD( _p, _id) \
	if ( _id.mCompileTimeEnabled && _p.getProfileEventSender() ) _p.getProfileEventSender()->startEvent( _id, _p.getEventContext(), PxProfileEventSender::CrossThreadId ); \
	if(Cm::isNVTXSupportEnabled()) { NV_TEXT_PROFILE_START_CROSSTHREAD( _p, _id); }

#define CM_PROFILE_STOP_CROSSTHREAD( _p, _id) \
	if ( _id.mCompileTimeEnabled && _p.getProfileEventSender() ) _p.getProfileEventSender()->stopEvent( _id, _p.getEventContext(), PxProfileEventSender::CrossThreadId ); \
	if(Cm::isNVTXSupportEnabled()) { NV_TEXT_PROFILE_STOP_CROSSTHREAD( _p, _id); }

#else
#define CM_PROFILE_START( _p, _id)
#define CM_PROFILE_STOP( _p, _id)

#define CM_PROFILE_VALUE( _p, subsystem, eventId, value )

#define CM_PROFILE_ZONE( _p, _id)
#define CM_PROFILE_ZONE_WITH_SUBSYSTEM( _p, subsystem, eventId )
#define CM_PROFILE_TASK_ZONE(_p, _id)

#define CM_PROFILE_START_CROSSTHREAD( _p, _id)
#define CM_PROFILE_STOP_CROSSTHREAD( _p, _id)
#endif

}

#endif

