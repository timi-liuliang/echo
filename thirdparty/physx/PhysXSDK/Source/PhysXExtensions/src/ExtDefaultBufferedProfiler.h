/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_PHYSICS_EXTENSIONS_DEFAULT_BUFFERED_PROFILER_H
#define PX_PHYSICS_EXTENSIONS_DEFAULT_BUFFERED_PROFILER_H

#include "CmPhysXCommon.h"
#include "PxDefaultBufferedProfiler.h"
#include "PxProfileEventHandler.h"
#include "PxProfileZoneManager.h"
#include "PxProfileEventBufferClient.h"
#include "PsMutex.h"
#include "PsArray.h"
#include "PsUserAllocated.h"

namespace physx
{
	class PxProfileZone;	

	namespace Ext
	{
		class DefaultBufferedProfiler;
		class ProfileEventHandler;

		// profile zone client callback used for buffer flush handle +
		// helper class that holds the profile zone and builds hashMap above
		// profile event id-names pair.
		//////////////////////////////////////////////////////////////////////////

		class ProfileZoneInterface: public PxProfileZoneClient, public Ps::UserAllocated
		{		
			PX_NOCOPY(ProfileZoneInterface)
		public:

			typedef Ps::HashMap<const PxU64,const char*>			ProfileIdNameMap;
			typedef const physx::Ps::Pair<const PxU64,const char*>	ProfileIdNamePair;

			ProfileZoneInterface(PxProfileZone& profileZone, DefaultBufferedProfiler* profiler);

			const char* getName() const;

			PxProfileZone& getProfileZone() { return mProfileZone; }

			void setDefaultBufferedProfiler(DefaultBufferedProfiler* profiler) { mDefaultBufferedProfiler = profiler; }
			void setBufferFlushFull(bool full) { mBufferFullFlush = full; }

			const char* getProfileEventName(const PxU64 id);

			~ProfileZoneInterface()
			{
				mIdNameMap.clear();
			}

			// interface from PxProfileZoneClient
		public:
			virtual void handleEventAdded( const PxProfileEventName& ) {}
			virtual void handleClientRemoved() {}

			virtual void handleBufferFlush(const PxU8* inData, PxU32 inLength );

		private:
			PxProfileZone&						mProfileZone;
			ProfileIdNameMap					mIdNameMap;			
			DefaultBufferedProfiler*			mDefaultBufferedProfiler;
			bool								mBufferFullFlush;
		};
		
		//////////////////////////////////////////////////////////////////////////

		// This is the main class that handles stop and start events and collects them for later printing
		class ProfileEventHandler: public PxProfileEventHandler
		{
		public:

			static const PxU32 EVENTS_RESERVE_SIZE = 512;
			static const PxU32 THREADS_RESERVE_SIZE = 8;

			// help structure to hold the event data. Times are in profiler ticks, we convert when we print
			struct ProfileEvent
			{
				PxU16 id;
				PxU64 contextId;
				PxU64 startTime;	
				PxU64 stopTime;

				static const PxU64 INVALID_TIME = PxU64(-1);
			};

			struct CUDAProfileEventOccurence
			{
				PxU16						eventId;
				PxU64 startTime;
				PxU64 endTime;
			};

			struct EventCollection
			{
				PxU32				threadId;
				PxU8				threadPriority;
				PxU8				cpuId;
				Ps::Array<ProfileEvent>		events;				
			};

			struct SortCUDAProfileOccurences
			{
				bool operator()(CUDAProfileEventOccurence a, CUDAProfileEventOccurence b) const
				{
					return (a.eventId < b.eventId) || ((a.eventId == b.eventId) && (a.startTime < b.startTime));
				}
			};

		public:
			ProfileEventHandler();

			virtual void onStartEvent( const PxProfileEventId& inId, PxU32 threadId, PxU64 contextId, PxU8 cpuId, PxU8 threadPriority, PxU64 timestamp );
			virtual void onStopEvent( const PxProfileEventId& inId, PxU32 threadId, PxU64 contextId, PxU8 cpuId, PxU8 threadPriority, PxU64 timestamp );

			virtual void onEventValue( const PxProfileEventId& , PxU32 , PxU64 , PxI64  ) {}
			virtual void onCUDAProfileBuffer( PxU64 , PxF32 , const PxU8* , PxU32 , PxU32  );

			void clear();

			// lets print the stored events on screen
			void reportEvents(Ps::Array<PxBufferedProfilerCallback*>& callbacks);

			void setProfileZoneInterface(ProfileZoneInterface* profileZoneInterface) { mProfileZoneInterface = profileZoneInterface; }

		protected:
			EventCollection* findCollection(PxU32 threadId);
			void reportCollection(PxBufferedProfilerCallback& callback, const EventCollection& collection);
			void reportCudaCollection(PxBufferedProfilerCallback& callback);

			EventCollection						mCrossThreadCollection;
			Ps::Array<EventCollection>			mThreadCollections;	
			ProfileZoneInterface*				mProfileZoneInterface;

			Ps::Array<CUDAProfileEventOccurence>	mCUDAEventOccurences;
		};

		//////////////////////////////////////////////////////////////////////////

		// our zone handler where we watch for the zone we're interested in, and hook our client into it
		class ProfileZoneHandler: public PxProfileZoneHandler
		{
		public:
			ProfileZoneHandler(ProfileEventHandler* handler, const char* profileZoneNames)
				: mProfileEventHandler(handler), mProfileZoneNames(profileZoneNames), mBufferedProfiler(NULL)
			{}
			
			virtual void onZoneAdded( PxProfileZone& inSDK );
			virtual void onZoneRemoved( PxProfileZone&  inSDK);

			void flushEvents(Ps::Array<PxBufferedProfilerCallback*>& callbacks);
			void release();

			void setProfileZoneNames(const char* names) { mProfileZoneNames = names; }
			void setBufferedProfiler(DefaultBufferedProfiler* profiler) { mBufferedProfiler = profiler; }

		private:
			ProfileEventHandler*					mProfileEventHandler;
			Ps::Array<ProfileZoneInterface*>		mProfileZoneInterfaces;
			const char*								mProfileZoneNames;
			DefaultBufferedProfiler*				mBufferedProfiler;
		};

		// Default implementation of the buffered profiler
		//////////////////////////////////////////////////////////////////////////

		class DefaultBufferedProfiler: public PxDefaultBufferedProfiler, public Ps::UserAllocated
		{
			PX_NOCOPY(DefaultBufferedProfiler)
		public:			
			DefaultBufferedProfiler(PxFoundation& foundation, const char * profileZoneNames);

			// flush all the event buffers to ensure the buffered callback sees all events that have been issued
			virtual void flushEvents();

			// get the underlying profile zone manager to hand to PxPhysics on creation
			virtual PxProfileZoneManager& getProfileZoneManager() { return *mProfileZoneManager; }

			virtual void addBufferedProfilerCallback(PxBufferedProfilerCallback& cb);

			virtual void removeBufferedProfilerCallback(PxBufferedProfilerCallback& cb);

			// release
			virtual void release();

			Ps::Mutex& getBufferedProfilerMutex()  { return mMutex; }
			Ps::Array<PxBufferedProfilerCallback*>&	getBufferedProfilerCallbacks()  { return mCallbacks; }
			ProfileEventHandler&	getProfileEventHandler() { return mProfileEventHandler; }

		protected:

			virtual ~DefaultBufferedProfiler();

		private:
			PxProfileZoneManager*						mProfileZoneManager;
			ProfileEventHandler							mProfileEventHandler;			
			ProfileZoneHandler							mProfileZoneHandler;
			Ps::Array<PxBufferedProfilerCallback*>		mCallbacks;
			Ps::Mutex									mMutex;			
		};


	} // namespace Ext
}

#endif // PX_PHYSICS_EXTENSIONS_DEFAULT_BUFFERED_PROFILER_H
