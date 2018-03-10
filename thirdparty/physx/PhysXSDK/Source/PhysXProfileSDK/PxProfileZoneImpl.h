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


#ifndef PX_PHYSX_PROFILE_SDK_IMPL_H
#define PX_PHYSX_PROFILE_SDK_IMPL_H
#include "PxProfileZone.h"
#include "PsMutex.h"
#include "PxProfileEventSystem.h"
#include "PxProfileEventMutex.h"
#include "PxProfileEventFilter.h"
#include "PxProfileZoneManager.h"
#include "PxProfileContextProviderImpl.h"
#include "PxProfileScopedMutexLock.h"

namespace physx { namespace profile {
	
	typedef shdfnd::MutexT<WrapperReflectionAllocator<PxU8> >	TZoneMutexType;
	typedef ScopedLockImpl<TZoneMutexType>				TZoneLockType;
	typedef EventBuffer< PxDefaultContextProvider, TZoneMutexType, TZoneLockType, PxProfileNullEventFilter > TZoneEventBufferType;

	template<typename TNameProvider>
	class ZoneImpl : TZoneEventBufferType //private inheritance intended
					, public PxProfileZone
					, public PxProfileEventBufferClient
	{
		typedef shdfnd::MutexT<WrapperReflectionAllocator<PxU8> >	TMutexType;
		typedef ProfileHashMap<const char*, PxU32>			TNameToEvtIndexMap;
		//ensure we don't reuse event ids.
		typedef ProfileHashMap<PxU16, const char*>			TEvtIdToNameMap;
		typedef TMutexType::ScopedLock						TLockType;


		const char*										mName;
		FoundationWrapper								mWrapper;
		mutable TMutexType								mMutex;
		ProfileArray<PxProfileEventName>				mEventNames;
		TNameToEvtIndexMap								mNameToEvtIndexMap;
		//ensure we don't reuse event ids.
		TEvtIdToNameMap									mEvtIdToNameMap;
		PxProfileZoneManager*							mProfileZoneManager;

		ProfileArray<PxProfileZoneClient*>				mClients;
		volatile bool									mEventsActive;
		PxUserCustomProfiler*							mUserCustomProfiler;
		
		PX_NOCOPY(ZoneImpl<TNameProvider>)
	public:
		ZoneImpl( PxAllocatorCallback* inAllocator, const char* inName, PxU32 bufferSize = 0x4000 /*16k*/, const TNameProvider& inProvider = TNameProvider() )
			: TZoneEventBufferType( inAllocator, bufferSize, PxDefaultContextProvider(), NULL, PxProfileNullEventFilter() )
			, mName( inName )
			, mWrapper( inAllocator )
			, mMutex( WrapperReflectionAllocator<PxU8>( mWrapper ) )
			, mEventNames( mWrapper )
			, mNameToEvtIndexMap( mWrapper )
			, mEvtIdToNameMap( mWrapper )
			, mProfileZoneManager( NULL )
			, mClients( mWrapper )
			, mEventsActive( false )
			, mUserCustomProfiler(NULL)
		{
			TZoneEventBufferType::setBufferMutex( &mMutex );
			//Initialize the event name structure with existing names from the name provider.
			PxProfileNames theNames( inProvider.getProfileNames() );
			for ( PxU32 idx = 0; idx < theNames.mEventCount; ++idx )
			{
				const PxProfileEventName& theName (theNames.mEvents[idx]);
				doAddName( theName.mName, theName.mEventId.mEventId, theName.mEventId.mCompileTimeEnabled );
			}
			TZoneEventBufferType::addClient( *this );
		}

		virtual ~ZoneImpl() { 
			if ( mProfileZoneManager != NULL )
				mProfileZoneManager->removeProfileZone( *this );
			mProfileZoneManager = NULL;
			TZoneEventBufferType::removeClient( *this );
		}

		virtual void setUserCustomProfiler(PxUserCustomProfiler* up)
		{
			mUserCustomProfiler = up;
		}

		void doAddName( const char* inName, PxU16 inEventId, bool inCompileTimeEnabled )
		{
			TLockType theLocker( mMutex );
			mEvtIdToNameMap.insert( inEventId, inName );
			PxU32 idx = static_cast<PxU32>( mEventNames.size() );
			mNameToEvtIndexMap.insert( inName, idx );
			mEventNames.pushBack( PxProfileEventName( inName, PxProfileEventId( inEventId, inCompileTimeEnabled ) ) );
		}

		virtual PxU16 getEventIdForName( const char* inName )
		{
			return getEventIdsForNames( &inName, 1 );
		}
		
		virtual PxU16 getEventIdsForNames( const char** inNames, PxU32 inLen )
		{
			TLockType theLocker( mMutex );
			if ( inLen == 0 )
				return 0;
			
			const TNameToEvtIndexMap::Entry* theEntry( mNameToEvtIndexMap.find( inNames[0] ) );
			if ( theEntry )
				return mEventNames[theEntry->second].mEventId;
			
			//Else git R dun.  
			PxU16 nameSize = static_cast<PxU16>( mEventNames.size() );
			//We don't allow 0 as an event id.
			PxU16 eventId = nameSize;
			//Find a contiguous set of unique event ids
			bool foundAnEventId = false;
			do
			{
				foundAnEventId = false;
				++eventId;
				for ( PxU16 idx = 0; idx < inLen && foundAnEventId == false; ++idx )
					foundAnEventId = mEvtIdToNameMap.find( PxU16(eventId + idx) ) != NULL;
			}
			while( foundAnEventId );

			PxU32 clientCount = mClients.size();
			for ( PxU16 nameIdx = 0; nameIdx < inLen; ++nameIdx )
			{
				PxU16 newId = PxU16(eventId + nameIdx);
				doAddName( inNames[nameIdx], newId, true );
				for( PxU32 clientIdx =0; clientIdx < clientCount; ++clientIdx )
					mClients[clientIdx]->handleEventAdded( PxProfileEventName( inNames[nameIdx], PxProfileEventId( newId ) ) );
			}

			return eventId;
		}

		virtual void setProfileZoneManager(PxProfileZoneManager* inMgr)
		{
			mProfileZoneManager = inMgr;
		}

		virtual PxProfileZoneManager* getProfileZoneManager()
		{
			return mProfileZoneManager;
		}

		

		const char* getName() { return mName; }

		PxProfileEventBufferClient* getEventBufferClient() { return this; }

		//SDK implementation
		
		void addClient( PxProfileZoneClient& inClient ) 
		{ 
			TLockType lock( mMutex ); 
			mClients.pushBack( &inClient );
			mEventsActive = true;
		}

		void removeClient( PxProfileZoneClient& inClient ) 
		{
			TLockType lock( mMutex );
			for ( PxU32 idx =0; idx < mClients.size(); ++idx )
			{
				if ( mClients[idx] == &inClient )
				{
					inClient.handleClientRemoved();
					mClients.replaceWithLast( idx );
					break;
				}
			}
			mEventsActive = mClients.size() != 0;
		}

		virtual bool hasClients() const 
		{
			return mEventsActive;
		}

		virtual PxProfileNames getProfileNames() const 
		{ 
			TLockType theLocker( mMutex );
			const PxProfileEventName* theNames = mEventNames.begin();
			PxU32 theEventCount = (PxU32)mEventNames.size();
			return PxProfileNames( theEventCount, theNames );
		}

		virtual void release() 
		{ 
			PX_PROFILE_DELETE( mWrapper.getAllocator(), this );
		}
		
		//Implementation chaining the buffer flush to our clients
		virtual void handleBufferFlush( const PxU8* inData, PxU32 inLength )
		{
			TLockType theLocker( mMutex );
			PxU32 clientCount = mClients.size();
			for( PxU32 idx =0; idx < clientCount; ++idx )
				mClients[idx]->handleBufferFlush( inData, inLength );
		}
		//Happens if something removes all the clients from the manager.
		virtual void handleClientRemoved() {}

		//Send a profile event, optionally with a context.  Events are sorted by thread
		//and context in the client side.
		virtual void startEvent( PxU16 inId, PxU64 contextId) 
		{ 
			if ( mUserCustomProfiler )
			{
				const char* name = mEvtIdToNameMap[inId];
				mUserCustomProfiler->onStartEvent(name,contextId,0);
			}
			if( mEventsActive ) 
			{
				TZoneEventBufferType::startEvent( inId, contextId ); 
			}
		}
		virtual void stopEvent( PxU16 inId, PxU64 contextId) 
		{ 
			if ( mUserCustomProfiler )
			{
				const char* name = mEvtIdToNameMap[inId];
				mUserCustomProfiler->onStopEvent(name,contextId,0);
			}
			if( mEventsActive ) 
			{
				TZoneEventBufferType::stopEvent( inId, contextId ); 
			}
		}

		virtual void startEvent( PxU16 inId, PxU64 contextId, PxU32 threadId) 
		{ 
			if ( mUserCustomProfiler )
			{
				const char* name = mEvtIdToNameMap[inId];
				mUserCustomProfiler->onStartEvent(name,contextId,threadId);
			}
			if( mEventsActive ) 
			{
				TZoneEventBufferType::startEvent( inId, contextId, threadId ); 
			}
		}
		virtual void stopEvent( PxU16 inId, PxU64 contextId, PxU32 threadId ) 
		{ 
			if ( mUserCustomProfiler )
			{
				const char* name = mEvtIdToNameMap[inId];
				mUserCustomProfiler->onStopEvent(name,contextId,threadId);
			}
			if( mEventsActive ) 
			{
				TZoneEventBufferType::stopEvent( inId, contextId, threadId ); 
			}
		}

		/**
		 *	Set an specific events value.  This is different than the profiling value
		 *	for the event; it is a value recorded and kept around without a timestamp associated
		 *	with it.  This value is displayed when the event itself is processed.
		 */
		virtual void eventValue( PxU16 inId, PxU64 contextId, PxI64 inValue ) 
		{ 
			if ( mUserCustomProfiler )
			{
				const char* name = mEvtIdToNameMap[inId];
				mUserCustomProfiler->onEventValue(name,inValue);
			}
			if( mEventsActive ) 
			{
				TZoneEventBufferType::eventValue( inId, contextId, inValue ); 
			}
		}
		virtual void CUDAProfileBuffer( PxF32 batchRuntimeInMilliseconds, const PxU8* cudaData, PxU32 bufLenInBytes, PxU32 bufferVersion ) 
		{
			if( mEventsActive ) TZoneEventBufferType::CUDAProfileBuffer( batchRuntimeInMilliseconds, cudaData, bufLenInBytes, bufferVersion ); 
		}
		virtual void flushProfileEvents() { TZoneEventBufferType::flushProfileEvents(); }
	};

}}
#endif
