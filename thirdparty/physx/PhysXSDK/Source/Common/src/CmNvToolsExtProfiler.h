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

#ifndef PX_PHYSICS_COMMON_NV_TOOLS_EXT_PROFILER_H
#define PX_PHYSICS_COMMON_NV_TOOLS_EXT_PROFILER_H

#if PX_NVTX

#include "nvToolsExt.h"
#include "PsThread.h"
#include "PsHash.h"
#include "PsHashMap.h"

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

	inline void Mark(const nvtx::Attributes& attrib) { nvtxMarkEx(attrib.out()); }
	inline void Mark(const nvtxEventAttributes_t* eventAttrib) { nvtxMarkEx(eventAttrib); }
	inline void Mark(const char* message) { nvtxMarkA(message); }
	inline void Mark(const wchar_t* message) { nvtxMarkW(message); }

	inline nvtxRangeId_t RangeStart(const nvtx::Attributes& attrib) { return nvtxRangeStartEx(attrib.out()); }
	inline nvtxRangeId_t RangeStart(const nvtxEventAttributes_t* eventAttrib) { return nvtxRangeStartEx(eventAttrib); }
	inline nvtxRangeId_t RangeStart(const char* message) { return nvtxRangeStartA(message); }
	inline nvtxRangeId_t RangeStart(const wchar_t* message) { return nvtxRangeStartW(message); }

	inline void RangeEnd(nvtxRangeId_t id) { nvtxRangeEnd(id); }

	inline int RangePush(const nvtx::Attributes& attrib) { return nvtxRangePushEx(attrib.out()); }
	inline int RangePush(const nvtxEventAttributes_t* eventAttrib) { return nvtxRangePushEx(eventAttrib); }
	inline int RangePush(const char* message) { return nvtxRangePushA(message); }
	inline int RangePush(const wchar_t* message) { return nvtxRangePushW(message); }

	inline void RangePop() { nvtxRangePop(); }

	inline void NameCategory(uint32_t category, const char* name) { nvtxNameCategoryA(category, name); }
	inline void NameCategory(uint32_t category, const wchar_t* name) { nvtxNameCategoryW(category, name); }

	inline void NameOsThread(uint32_t threadId, const char* name) { nvtxNameOsThreadA(threadId, name); }
	inline void NameOsThread(uint32_t threadId, const wchar_t* name) { nvtxNameOsThreadW(threadId, name); }
	inline void NameCurrentThread(const char* name) { nvtxNameOsThreadA( uint32_t(physx::shdfnd::Thread::getId()), name); }
	inline void NameCurrentThread(const wchar_t* name) { nvtxNameOsThreadW( uint32_t(physx::shdfnd::Thread::getId()), name); }


	template <bool TEnabled> 
	class TemplateEnabledRange: public ScopedRange
	{
	public:
		TemplateEnabledRange(const char* message): ScopedRange(message) {}
		TemplateEnabledRange(const wchar_t* message): ScopedRange(message) {}
		TemplateEnabledRange(const nvtxEventAttributes_t* attributes): ScopedRange(attributes) {}
		TemplateEnabledRange(const Attributes& attributes): ScopedRange(attributes) {}
		~TemplateEnabledRange() {}
	};

	template <> 
	class TemplateEnabledRange<false>
	{
	public:
		TemplateEnabledRange(const char* message) { (void)message; }
		TemplateEnabledRange(const wchar_t* message) { (void)message; }
		TemplateEnabledRange(const nvtxEventAttributes_t* attributes) { (void)attributes; }
		TemplateEnabledRange(const Attributes& attributes) { (void)attributes; }
		~TemplateEnabledRange() {}
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

	struct EventIdContextPair
	{
		EventIdContextPair(): context(physx::PxU64(-1)), id(physx::PxU16(-1)) {}
		EventIdContextPair(physx::PxU64 c, physx::PxU16 i): context(c), id(i) {}
		PX_FORCE_INLINE bool operator==(const EventIdContextPair& p) const { return (context==p.context) && (id==p.id); }

		physx::PxU64 context;
		physx::PxU16 id;
	};

	template<class Key>
	struct Hash
	{
	};

	// hash object for hash map template parameter
	template <>
	struct Hash<EventIdContextPair>
	{
		physx::PxU32 operator()(const EventIdContextPair& k) const { return physx::shdfnd::hash((physx::PxU64(k.id)<<32) | (k.context&0xffffffff)); }
		bool operator()(const EventIdContextPair& k0, const EventIdContextPair& k1) const { return k0 == k1; }
	};

	typedef Hash<EventIdContextPair> EventHash;
	typedef physx::shdfnd::HashMap<nvtx::EventIdContextPair, nvtxRangeId_t, nvtx::EventHash> EventHashMap;

	#define NV_TEXT_COL(_nvTextCol) (((((_nvTextCol)*7)&255)<<8)+0xff8300de)
	#define NV_TEXT_PROFILE_START( _p, _id) nvtx::Attributes _nvtxAttr; physx::PxU32 _nvtxCont = physx::PxU32((_p).getEventContext()); nvtx::RangePush(_nvtxAttr.color(NV_TEXT_COL(_nvtxCont)).message(_p.getStringFromId(_id.mEventId)));
	#define NV_TEXT_PROFILE_STOP( _p, _id) nvtx::RangePop();

	#define NV_TEXT_PROFILE_ZONE( _p, _id) nvtx::Attributes _nvtxAttr; physx::PxU32 _nvtxCont = physx::PxU32((_p).getEventContext()); nvtx::RuntimeEnabledScope __nvtxZone(_nvtxAttr.color(NV_TEXT_COL(_nvtxCont)).message(_p.getStringFromId(_id.mEventId)).category(_nvtxCont), _id.mCompileTimeEnabled);

	#define NV_TEXT_PROFILE_ZONE_WITH_SUBSYSTEM( _p, subsystem, eventId ) nvtx::Attributes _nvtxAttr; physx::PxU32 _nvtxCont = physx::PxU32((_p).getEventProfiler().getEventContext()); nvtx::TemplateEnabledRange<PX_PROFILE_EVENT_FILTER_VALUE(subsystem,eventId)> __nvtxZone(_nvtxAttr.color(NV_TEXT_COL(_nvtxCont)).message(#subsystem "." #eventId).category(_nvtxCont));
	#define NV_TEXT_PROFILE_VALUE( _p, subsystem, eventId, value ) if(PX_PROFILE_EVENT_FILTER_VALUE(subsystem,eventId)){nvtx::Attributes _nvtxAttr; physx::PxU32 _nvtxCont = physx::PxU32((_p).getEventProfiler().getEventContext()); nvtx::Mark(_nvtxAttr.color(NV_TEXT_COL(_nvtxCont)).category(_nvtxCont).payload(value).message(#subsystem "." #eventId)); }

	#define NV_TEXT_PROFILE_START_CROSSTHREAD( _p, _id) \
		if (_id.mCompileTimeEnabled) { nvtx::Attributes _nvtxAttr; physx::PxU32 _nvtxCont = physx::PxU32((_p).getEventContext()); _p.storeCrossEvent(_id.mEventId, _p.getEventContext(), nvtx::RangeStart(_nvtxAttr.color(NV_TEXT_COL(_nvtxCont)).message(_p.getStringFromId(_id.mEventId)).category(_nvtxCont))); }

	#define NV_TEXT_PROFILE_STOP_CROSSTHREAD( _p, _id) \
		if (_id.mCompileTimeEnabled) { nvtx::RangeEnd(_p.getCrossEvent(_id.mEventId, _p.getEventContext())); }
} // namespace nvtx
#else
	#define NV_TEXT_COL(_p)
	#define NV_TEXT_PROFILE_START( _p, _id)
	#define NV_TEXT_PROFILE_STOP( _p, _id)
	#define NV_TEXT_PROFILE_ZONE( _p, _id)
	#define NV_TEXT_PROFILE_ZONE_WITH_SUBSYSTEM( _p, subsystem, eventId )
	#define NV_TEXT_PROFILE_VALUE( _p, subsystem, eventId, value )
	#define NV_TEXT_PROFILE_START_CROSSTHREAD( _p, _id)
	#define NV_TEXT_PROFILE_STOP_CROSSTHREAD( _p, _id)
#endif // defined PX_NVTX

#endif
