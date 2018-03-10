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
#ifndef PX_PROFILER_CALLBACK_H
#define PX_PROFILER_CALLBACK_H

#ifdef PHYSX_PROFILE_SDK

#include "PxProfileZone.h"
#include "PxProfileZoneManager.h"
#include "PxProfileEventSystem.h"
#include "PxProfileEventHandler.h"
#include "PxProfileEventNames.h"
#include "PxProfileScopedEvent.h"
#include "PxProfileCompileTimeEventFilter.h"
#include "PxProfileScopedEvent.h"
#include "PVDBinding.h"
//#include "PvdConnectionType.h"

extern physx::PxProfileZone *gProfileZone;

class PxProfilerCallbackProfileEvent
{
public:
	PxProfilerCallbackProfileEvent(physx::PxU16 eventId,physx::PxU64 context)
	{
		mEventId = eventId;
		mContext = context;
		gProfileZone->startEvent(mEventId,mContext);
	}


	~PxProfilerCallbackProfileEvent(void)
	{
		gProfileZone->stopEvent(mEventId,mContext);
	}

private:
	physx::PxU16			mEventId;
	physx::PxU64			mContext;
};

#if PX_NVTX == 1

#include "nvToolsExt.h"

// C++ function templates to enable NvToolsExt functions
namespace nvtx
{
	class Attributes
	{
	public:
		inline Attributes()
		{
			clear();
		}

		inline Attributes& category(uint32_t category)
		{
			m_event.category = category;
			return *this;
		}

		inline Attributes& color(uint32_t argb)
		{
			m_event.colorType = NVTX_COLOR_ARGB;
			m_event.color = argb;
			return *this;
		}

		inline Attributes& payload(uint64_t value)
		{
			m_event.payloadType = NVTX_PAYLOAD_TYPE_UNSIGNED_INT64;
			m_event.payload.ullValue = value;
			return *this;
		}

		inline Attributes& payload(physx::PxU32 value)
		{
			return payload(static_cast<uint64_t>(value));
		}

		inline Attributes& payload(physx::PxI32 value)
		{
			return payload(static_cast<int64_t>(value));
		}

		inline Attributes& payload(physx::PxF32 value)
		{
			return payload(static_cast<double>(value));
		}

		inline Attributes& payload(int64_t value)
		{
			m_event.payloadType = NVTX_PAYLOAD_TYPE_INT64;
			m_event.payload.llValue = value;
			return *this;
		}

		inline Attributes& payload(double value)
		{
			m_event.payloadType = NVTX_PAYLOAD_TYPE_DOUBLE;
			m_event.payload.dValue = value;
			return *this;
		}

		inline Attributes& message(const char* message)
		{
			m_event.messageType = NVTX_MESSAGE_TYPE_ASCII;
			m_event.message.ascii = message;
			return *this;
		}

		inline Attributes& message(const wchar_t* message)
		{
			m_event.messageType = NVTX_MESSAGE_TYPE_UNICODE;
			m_event.message.unicode = message;
			return *this;
		}

		inline Attributes& clear()
		{
			memset(&m_event, 0, NVTX_EVENT_ATTRIB_STRUCT_SIZE);
			m_event.version = NVTX_VERSION;
			m_event.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
			return *this;
		}

		inline const nvtxEventAttributes_t* out() const
		{
			return &m_event;
		}

	private:
		nvtxEventAttributes_t m_event;
	};


	class ScopedRange
	{
	public:
		inline ScopedRange(const char* message)
		{
			nvtxRangePushA(message);
		}

		inline ScopedRange(const wchar_t* message)
		{
			nvtxRangePushW(message);
		}

		inline ScopedRange(const nvtxEventAttributes_t* attributes)
		{
			nvtxRangePushEx(attributes);
		}

		inline ScopedRange(const nvtx::Attributes& attributes)
		{
			nvtxRangePushEx(attributes.out());
		}

		inline ~ScopedRange()
		{
			nvtxRangePop();
		}
	};

	class RuntimeEnabledScope
	{
		bool mEnabled;
	public:
		inline RuntimeEnabledScope(const char* message, bool enabled): mEnabled(enabled)
		{
			if(enabled)
				nvtxRangePushA(message);
		}

		inline RuntimeEnabledScope(const wchar_t* message, bool enabled): mEnabled(enabled)
		{
			if(enabled)
				nvtxRangePushW(message);
		}

		inline RuntimeEnabledScope(const nvtxEventAttributes_t* attributes, bool enabled): mEnabled(enabled)
		{
			if(enabled)
				nvtxRangePushEx(attributes);
		}

		inline RuntimeEnabledScope(const nvtx::Attributes& attributes, bool enabled): mEnabled(enabled)
		{
			if(enabled)
				nvtxRangePushEx(attributes.out());
		}

		inline ~RuntimeEnabledScope()
		{
			if(mEnabled)
				nvtxRangePop();
		}
	};

	#define NV_TEXT_COL (0xff96d700)
	#define NV_TEXT_PROFILE_START(msg) nvtx::Attributes a; nvtx::RangePush(a.color(NV_TEXT_COL).message(msg));
	#define NV_TEXT_PROFILE_STOP() nvtx::RangePop();

	#define NV_TEXT_PROFILE_ZONE(msg) nvtx::Attributes a; nvtx::RuntimeEnabledScope __nvtxZone(a.color(NV_TEXT_COL).message(msg), true);
	#define NV_TEXT_PROFILE_VALUE(msg, val) nvtx::Attributes a; nvtx::RuntimeEnabledScope __nvtxZone(a.color(NV_TEXT_COL).message(msg).payload(val), true);

} // namespace nvtx
#else
	#define NV_TEXT_COL(_p)
	#define NV_TEXT_PROFILE_START( _p, _id)
	#define NV_TEXT_PROFILE_STOP( _p, _id)
	#define NV_TEXT_PROFILE_ZONE( _name)
	#define NV_TEXT_PROFILE_VALUE(msg, val)
#endif // defined PX_NVTX


// PH: 3.2 doesn't have a profiler in release mode (d'oh)
#if defined(PHYSX_PROFILE_SDK)

#define PX_PROFILER_PERF_SCOPE(name) PX_ASSERT(gProfileZone); static physx::PxU16 eventId = gProfileZone->getEventIdForName(name);  PxProfilerCallbackProfileEvent _profile(eventId,0); NV_TEXT_PROFILE_ZONE(name);
#define PX_PROFILER_PERF_DSCOPE(name,data) PX_ASSERT(gProfileZone); static physx::PxU16 eventId = gProfileZone->getEventIdForName(name);  PxProfilerCallbackProfileEvent _profile(eventId,0); gProfileZone->eventValue(eventId,0,data); NV_TEXT_PROFILE_VALUE(name, data)

#define PX_PROFILER_START_EVENT(x,y) PX_ASSERT(gProfileZone); gProfileZone->startEvent(x,y)
#define PX_PROFILER_STOP_EVENT(x,y) PX_ASSERT(gProfileZone); gProfileZone->stopEvent(x,y)

#define PX_PROFILER_PLOT(x,y) { PX_ASSERT(gProfileZone); static physx::PxU16 eventId = gProfileZone->getEventIdForName(y); gProfileZone->eventValue(eventId,0,x); }

#else // PHYSX_PROFILE_SDK

#define PX_PROFILER_PERF_SCOPE(name)
#define PX_PROFILER_PERF_DSCOPE(name,data)

#define PX_PROFILER_START_EVENT(x,y)
#define PX_PROFILER_STOP_EVENT(x,y)

#define PX_PROFILER_PLOT(x,y)

#endif // PHYSX_PROFILE_SDK

#else

#define PX_PROFILER_PERF_SCOPE(name)
#define PX_PROFILER_PERF_DSCOPE(name,data)

#define PX_PROFILER_START_EVENT(x,y)
#define PX_PROFILER_STOP_EVENT(x,y)

#define PX_PROFILER_PLOT(x,y)

#endif

#endif // PX_FOUNDATION_PX_PROFILER_CALLBACK_H
