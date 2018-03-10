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


#ifndef PX_PROFILE_MEMORY_EVENT_BUFFER_H
#define PX_PROFILE_MEMORY_EVENT_BUFFER_H
#include "PxProfileDataBuffer.h"
#include "PsHash.h"
#include "PsHashMap.h"
#include "PxProfileMemoryEvents.h"
#include "PxProfileMemoryEventTypes.h"
#include "PxProfileScopedMutexLock.h"
#include "PxProfileFoundationWrapper.h"

namespace physx { namespace profile {

	template<typename TMutex,
			 typename TScopedLock>
	class MemoryEventBuffer : public DataBuffer<TMutex, TScopedLock>
	{
	public:
		typedef DataBuffer<TMutex, TScopedLock> TBaseType;
		typedef typename TBaseType::TMutexType TMutexType;
		typedef typename TBaseType::TScopedLockType TScopedLockType;
		typedef typename TBaseType::TU8AllocatorType TU8AllocatorType;
		typedef typename TBaseType::TMemoryBufferType TMemoryBufferType;
		typedef typename TBaseType::TBufferClientArray TBufferClientArray;
		typedef shdfnd::HashMap<const char*, PxU32, shdfnd::Hash<const char*>, TU8AllocatorType> TCharPtrToHandleMap;

	protected:
		TCharPtrToHandleMap mStringTable;

	public:

		static inline PxAllocatorCallback* getBaseAllocator( PxFoundation* inFoundation )
		{
			if ( inFoundation )
				return &inFoundation->getAllocatorCallback();
			return NULL;
		}

		MemoryEventBuffer( PxFoundation* inFoundation
					, PxU32 inBufferFullAmount
					, TMutexType* inBufferMutex )
			: TBaseType( getBaseAllocator( inFoundation ), inBufferFullAmount, inBufferMutex, "struct physx::profile::MemoryEvent" )
			, mStringTable( TU8AllocatorType( TBaseType::getWrapper(), "MemoryEventStringBuffer" ) )
		{
		}

		MemoryEventBuffer( PxAllocatorCallback& cback
					, PxU32 inBufferFullAmount
					, TMutexType* inBufferMutex )
			: TBaseType( &cback, inBufferFullAmount, inBufferMutex, "struct physx::profile::MemoryEvent" )
			, mStringTable( TU8AllocatorType( TBaseType::getWrapper(), "MemoryEventStringBuffer" ) )
		{
		}

		PxU32 getHandle( const char* inData )
		{
			if ( inData == NULL ) inData = "";
			const typename TCharPtrToHandleMap::Entry* result( mStringTable.find( inData ) );
			if ( result )
				return result->second;
			PxU32 hdl = mStringTable.size() + 1;
			mStringTable.insert( inData, hdl );
			StringTableEvent theEvent;
			theEvent.init( inData, hdl );
			sendEvent( theEvent );
			return hdl;
		}

		void onAllocation( size_t inSize, const char* inType, const char* inFile, PxU32 inLine, PxU64 addr )
		{
			if ( addr == 0 )
				return;
			PxU32 typeHdl( getHandle( inType ) );
			PxU32 fileHdl( getHandle( inFile ) );
			AllocationEvent theEvent;
			theEvent.init( inSize, typeHdl, fileHdl, inLine, addr );
			sendEvent( theEvent );
		}

		void onDeallocation( PxU64 addr )
		{
			if ( addr == 0 )
				return;
			DeallocationEvent theEvent;
			theEvent.init( addr );
			sendEvent( theEvent );
		}

		void flushProfileEvents()
		{
			TBaseType::flushEvents();
		}

	protected:
		
		template<typename TDataType>
		void sendEvent( TDataType inType )
		{
			MemoryEventHeader theHeader( getMemoryEventType<TDataType>() );
			inType.setup( theHeader );
			theHeader.streamify( TBaseType::mSerializer );
			inType.streamify( TBaseType::mSerializer, theHeader );
			if ( TBaseType::mDataArray.size() >= TBaseType::mBufferFullAmount )
				flushProfileEvents();
		}
	};

	class PxProfileMemoryEventBufferImpl : public shdfnd::UserAllocated
		, public PxProfileMemoryEventBuffer
	{
		typedef MemoryEventBuffer<PxProfileEventMutex, NullLock> TMemoryBufferType;
		TMemoryBufferType mBuffer;

	public:
		PxProfileMemoryEventBufferImpl( PxFoundation* inFoundation, PxU32 inBufferFullAmount )
			: mBuffer( inFoundation, inBufferFullAmount, NULL )
		{
		}

		PxProfileMemoryEventBufferImpl( PxAllocatorCallback& alloc, PxU32 inBufferFullAmount )
			: mBuffer( alloc, inBufferFullAmount, NULL )
		{
		}

		virtual void onAllocation( size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory )
		{
			mBuffer.onAllocation( size, typeName, filename, (PxU32)line, PX_PROFILE_POINTER_TO_U64( allocatedMemory ) );
		}
		virtual void onDeallocation( void* allocatedMemory )
		{
			mBuffer.onDeallocation( PX_PROFILE_POINTER_TO_U64( allocatedMemory ) );
		}
		
		virtual void addClient( PxProfileEventBufferClient& inClient ) { mBuffer.addClient( inClient ); }
		virtual void removeClient( PxProfileEventBufferClient& inClient ) { mBuffer.removeClient( inClient ); }
		virtual bool hasClients() const { return mBuffer.hasClients(); }

		virtual void flushProfileEvents() { mBuffer.flushProfileEvents(); }

		virtual void release(){ PX_PROFILE_DELETE( mBuffer.getWrapper().getAllocator(), this ); }
	};
}}

#endif
