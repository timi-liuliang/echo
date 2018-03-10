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


#ifndef PX_PHYSX_PROFILE_MEMORY_EVENT_RECORDER_H
#define PX_PHYSX_PROFILE_MEMORY_EVENT_RECORDER_H
#include "PxProfileBase.h"
#include "PsHashMap.h"
#include "PxProfileFoundationWrapper.h"
#include "PxProfileMemoryEvents.h"
#include "PxProfileMemoryEventTypes.h"
#include "PsUserAllocated.h"

namespace physx { namespace profile {

	//Remember outstanding events.
	//Remembers allocations, forwards them to a listener if one is attached
	//and will forward all outstanding allocations to a listener when one is
	//attached.
	struct MemoryEventRecorder : public PxAllocationListener
	{
		typedef WrapperReflectionAllocator<PxU8> TAllocatorType;
		typedef shdfnd::HashMap<PxU64,FullAllocationEvent,shdfnd::Hash<PxU64>,TAllocatorType> THashMapType;

		FoundationWrapper		mWrapper;
		THashMapType			mOutstandingAllocations;
		PxAllocationListener*	mListener;

		MemoryEventRecorder( PxFoundation* inFoundation )
			: mWrapper( inFoundation, true )
			, mOutstandingAllocations( TAllocatorType( mWrapper ) )
			, mListener( NULL )
		{
		}

		static PxU64 ToU64( void* inData ) { return PX_PROFILE_POINTER_TO_U64( inData ); }
		static void* ToVoidPtr( PxU64 inData ) { return (void*)(size_t)inData; }
		virtual void onAllocation( size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory )
		{
			onAllocation( size, typeName, filename, (PxU32)line, ToU64( allocatedMemory ) );
		}
		
		void onAllocation( size_t size, const char* typeName, const char* filename, PxU32 line, PxU64 allocatedMemory )
		{
			if ( allocatedMemory == 0 )
				return;
			FullAllocationEvent theEvent;
			theEvent.init( size, typeName, filename, line, allocatedMemory );
			mOutstandingAllocations.insert( allocatedMemory, theEvent );
			if ( mListener != NULL ) mListener->onAllocation( size, typeName, filename, (int)line, ToVoidPtr(allocatedMemory) );
		}
		
		virtual void onDeallocation( void* allocatedMemory )
		{
			onDeallocation( ToU64( allocatedMemory ) );
		}

		void onDeallocation( PxU64 allocatedMemory )
		{
			if ( allocatedMemory == 0 )
				return;
			mOutstandingAllocations.erase( allocatedMemory );
			if ( mListener != NULL ) mListener->onDeallocation( ToVoidPtr( allocatedMemory ) );
		}

		void flushProfileEvents() {}

		void setListener( PxAllocationListener* inListener )
		{
			mListener = inListener;
			if ( mListener )
			{	
				for ( THashMapType::Iterator iter = mOutstandingAllocations.getIterator();
					!iter.done();
					++iter )
				{
					const FullAllocationEvent& evt( iter->second );
					mListener->onAllocation( evt.mSize, evt.mType, evt.mFile, (int)evt.mLine, ToVoidPtr( evt.mAddress ) );
				}
			}
		}
	};

	class PxProfileMemoryEventRecorderImpl : public shdfnd::UserAllocated
											, public PxProfileMemoryEventRecorder
	{
		MemoryEventRecorder mRecorder;
	public:
		PxProfileMemoryEventRecorderImpl( PxFoundation* inFnd )
			: mRecorder( inFnd )
		{
		}

		virtual void onAllocation( size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory )
		{
			mRecorder.onAllocation( size, typeName, filename, line, allocatedMemory );
		}

		virtual void onDeallocation( void* allocatedMemory )
		{
			mRecorder.onDeallocation( allocatedMemory );
		}
		
		virtual void setListener( PxAllocationListener* inListener )
		{
			mRecorder.setListener( inListener );
		}

		virtual void release()
		{
			PX_PROFILE_DELETE( mRecorder.mWrapper.getAllocator(), this );
		}
	};

}}
#endif
