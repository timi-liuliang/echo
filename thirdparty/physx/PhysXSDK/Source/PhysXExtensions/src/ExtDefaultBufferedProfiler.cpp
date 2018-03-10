/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ExtDefaultBufferedProfiler.h"
#include "PsAllocator.h"
#include "PxPhysicsAPI.h"
#include "PsTime.h"
#include "PsSort.h"

using namespace physx;

namespace physx
{
	PxDefaultBufferedProfiler* PxDefaultBufferedProfilerCreate(PxFoundation	& foundation, const char * profileZoneNames)
	{
		PxDefaultBufferedProfiler* retVal = PX_NEW(Ext::DefaultBufferedProfiler)(foundation, profileZoneNames);
		return retVal;
	}

	//////////////////////////////////////////////////////////////////////////

	namespace Ext
	{
		namespace Local
		{
			// helper class for CUDA events
			class OverflowRecord
			{
			public:
				OverflowRecord()
					: mLastValue(PxU32(-1)), mHighBitHighOffset(0), mHighBitLowOffset(0)
				{
				}

				static bool IsHighBitHight(PxU32 value) { return (value & 0x80000000) > 0; }
				//Accounting for small jitter, the next value in the stream
				//should be higher than the last value.
				//You also have to take into account that the values won't be strictly incrementing.
				//They will be *roughly* incrementing.  So you have to account for some jitter
				//in the multiprocessor timing stream.  We detect possible cases of overflow
				//by looking at the high bit of this value vs. the high bit of the last value.
				//We account for jitter by taking the absolute value of the difference of the two values
				//and ensuring this is more than a certain amount.
				PxU64 NextValue(PxU32 value)
				{
					const PxU32 largestJitterValue = PX_MAX_U32 / 4;

					//Detect overflow by checking the high bit of this value against the high
					//bit of the last value
					bool highBitRaised = IsHighBitHight( value );
					if (mLastValue != PxU32(-1))
					{

						bool lastHighBitRaised = IsHighBitHight( mLastValue );
						if (highBitRaised != lastHighBitRaised)
						{

							if (highBitRaised)
							{
								PxU32 diff = value - mLastValue;
								if ( diff < largestJitterValue )
									mHighBitHighOffset = mHighBitLowOffset;
							}
							else 
							{
								PxU32 diff = mLastValue - value;
								if ( diff > largestJitterValue && mHighBitLowOffset == mHighBitHighOffset)
									mHighBitLowOffset += PX_MAX_U32;
							}
						}
					}
					mLastValue = value;
					PxU64 offset = highBitRaised ? mHighBitHighOffset : mHighBitLowOffset;
					return value + offset;
				}
			private:
				PxU32   mLastValue;
				//Offset to use if the high bit is raised
				PxU64	mHighBitHighOffset;
				//Offset to use if the high bit is not raised.
				PxU64	mHighBitLowOffset;
			};

			//////////////////////////////////////////////////////////////////////////

			struct OverflowRecordPair
			{
				PxU8 mpId;
				OverflowRecord mOverflowRecord;
			};
		}

		//////////////////////////////////////////////////////////////////////////

		ProfileEventHandler::ProfileEventHandler()
			: mProfileZoneInterface(NULL)
		{ 
			mCrossThreadCollection.events.reserve(EVENTS_RESERVE_SIZE);
			mCrossThreadCollection.threadId = PxBufferedProfilerCallback::CROSS_THREAD_ID;
			mThreadCollections.reserve(THREADS_RESERVE_SIZE);
			clear(); 
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileEventHandler::onCUDAProfileBuffer( PxU64 , PxF32  , const PxU8* cudaData, PxU32 bufLenInBytes, PxU32 bufferVersion )
		{
			if(bufferVersion == 1)
			{
				struct currentWarpProfileEvent
				{
					PxU16 block;
					PxU8  warp;
					PxU8  mpId;
					PxU8  hwWarpId;
					PxU8  userDataCfg;
					PxU16 eventId;
					PxU32 startTime;
					PxU32 endTime;
				};

				Ps::Array<Local::OverflowRecordPair> overflowPair;

				//Run through dataset.  We need to be able to correct rollover, meaning one of the timers
				//runs through PxU32.MAX_VALUE and resets to zero.
				PxU32 numEvents = bufLenInBytes/sizeof(currentWarpProfileEvent);
				const currentWarpProfileEvent* cudaEvents = reinterpret_cast<const currentWarpProfileEvent*> (cudaData);
				for (PxU32 i = 0; i < numEvents; i++)
				{
					const currentWarpProfileEvent& cudaEvent = cudaEvents[i];
					Local::OverflowRecord* record = NULL;
					for (PxU32 j = 0; j < overflowPair.size(); j++)
					{
						if(overflowPair[j].mpId == cudaEvent.mpId)
						{
							record = &overflowPair[j].mOverflowRecord;
							break;
						}
					}

					if(!record)
					{
						Local::OverflowRecordPair pair;
						pair.mpId = cudaEvent.mpId;						
						overflowPair.pushBack(pair);
						record = &overflowPair.back().mOverflowRecord;
					}
					
					//account for overflow
					PxU64 startTime = record->NextValue(cudaEvent.startTime);
					PxU64 endTime = record->NextValue(cudaEvent.endTime);

					CUDAProfileEventOccurence cudaEventOccurence = { cudaEvent.eventId, startTime, endTime };
					mCUDAEventOccurences.pushBack(cudaEventOccurence);
				}
			}		
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileEventHandler::onStartEvent( const PxProfileEventId& inId, PxU32 threadId, PxU64 contextId, PxU8 cpuId, PxU8 threadPriority, PxU64 timestamp )
		{
			EventCollection* threadCollection = findCollection(threadId);

			// add a new collection for this thread Id if it's not already there
			if(!threadCollection)
			{
				EventCollection ec;
				ec.cpuId = cpuId;
				ec.threadId = threadId;
				ec.threadPriority = threadPriority;				
				mThreadCollections.pushBack(ec);
				threadCollection = &mThreadCollections.back();
				threadCollection->events.reserve(EVENTS_RESERVE_SIZE);
			}

			// add the event record			
			ProfileEvent ev = { inId, contextId, timestamp, ProfileEvent::INVALID_TIME };
			threadCollection->events.pushBack(ev);
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileEventHandler::onStopEvent( const PxProfileEventId& inId, PxU32 threadId, PxU64 contextId, PxU8 cpuId, PxU8 threadPriority, PxU64 timestamp )
		{
			PX_UNUSED(contextId);
			PX_UNUSED(cpuId);
			PX_UNUSED(threadPriority);

			EventCollection* threadCollection = findCollection(threadId);
			PX_ASSERT(threadCollection != NULL);

			// an event (e.g. narrow phase batch) can occur several times per thread per frame, so
			// we take the earliest event with a matching id that does not yet have a stop time
			const PxU32 eventsSize = threadCollection->events.size();
			for (PxU32 i = 0; i < eventsSize; i++)
			{
				ProfileEvent& ev = threadCollection->events[i];
				if(ev.id == inId.mEventId && ev.stopTime == ProfileEvent::INVALID_TIME)
				{					
					ev.stopTime = timestamp;
					PX_ASSERT(ev.stopTime >= ev.startTime);
					break;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileEventHandler::clear()
		{				
			mCrossThreadCollection.events.clear();
			for (PxU32 i = mThreadCollections.size(); i--;)
				mThreadCollections[i].events.clear();

			mCUDAEventOccurences.clear();
			mProfileZoneInterface = NULL;
		}

		//////////////////////////////////////////////////////////////////////////
		
		void ProfileEventHandler::reportEvents(Ps::Array<PxBufferedProfilerCallback*>& callbacks)
		{
			PX_ASSERT(mProfileZoneInterface);

			for (PxU32 callbackIndex = callbacks.size(); callbackIndex--; )
			{
				PxBufferedProfilerCallback& callback = *callbacks[callbackIndex];
				reportCollection(callback, mCrossThreadCollection);
				for (PxU32 i = mThreadCollections.size(); i--;)
				{
					reportCollection(callback, mThreadCollections[i]);
				}

				reportCudaCollection(callback);
			}

			clear();
		}

		//////////////////////////////////////////////////////////////////////////

		ProfileEventHandler::EventCollection* ProfileEventHandler::findCollection(PxU32 threadId)
		{
			if(threadId == PxProfileEventSender::CrossThreadId)
				return &mCrossThreadCollection;

			for (PxU32 i = mThreadCollections.size(); i--; )
			{
				if(mThreadCollections[i].threadId == threadId)
					return &mThreadCollections[i];
			}
			return NULL;
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileEventHandler::reportCollection(PxBufferedProfilerCallback& callback, const EventCollection& collection)
		{
			for (PxU32 i = collection.events.size(); i--;)
			{
				const ProfileEvent& ev = collection.events[i];
				PxBufferedProfilerEvent e;

				e.cpuId = collection.cpuId;
				e.threadId = collection.threadId;
				e.threadPriority = collection.threadPriority;

				e.contextId = ev.contextId;
				e.name = mProfileZoneInterface->getProfileEventName(ev.id);
				e.profileZoneName = mProfileZoneInterface->getName();
				e.startTimeNs = Ps::Time::getBootCounterFrequency().toTensOfNanos(ev.startTime) * 10;
				e.stopTimeNs = Ps::Time::getBootCounterFrequency().toTensOfNanos(ev.stopTime) * 10;

				callback.onEvent(e);
			}
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileEventHandler::reportCudaCollection(PxBufferedProfilerCallback& callback)
		{
			if(mCUDAEventOccurences.empty())
				return;

			// sort the cuda occurrences - according to eventID and startTime
			Ps::sort(mCUDAEventOccurences.begin(), mCUDAEventOccurences.size(), SortCUDAProfileOccurences());
			PxU16 currentEventId = mCUDAEventOccurences[0].eventId;
			CUDAProfileEventOccurence currentEvent = mCUDAEventOccurences[0];
			Ps::Array<CUDAProfileEventOccurence> outOccurences;

			// now group the occurrences that do overlap
			// put the group occurrences into out array
			for (PxU32 i = 1; i < mCUDAEventOccurences.size(); i++)
			{
				const CUDAProfileEventOccurence& occurence = mCUDAEventOccurences[i];
				// occurrences are sorted by eventId, so once it changes, we have new group occurrence
				if(currentEventId != occurence.eventId)
				{
					outOccurences.pushBack(currentEvent);
					currentEvent = occurence;
					currentEventId = occurence.eventId;
				}
				else
				{
					// occurrences are sorted by start time, so we check for overlap and add occurrence or create new group
					if(currentEvent.endTime >= occurence.startTime)
					{
						currentEvent.endTime = PxMax(currentEvent.endTime, occurence.endTime);
					}
					else
					{
						outOccurences.pushBack(currentEvent);
						currentEvent = occurence;
					}
				}
			}
			outOccurences.pushBack(currentEvent);

			// fire the callback with grouped occurrences
			for (PxU32 i = outOccurences.size(); i--;)
			{
				const CUDAProfileEventOccurence& ev = outOccurences[i];				

				const char* name = mProfileZoneInterface->getProfileEventName(ev.eventId);
				const char* profileZoneName = mProfileZoneInterface->getName();

				PxBufferedProfilerEvent ce = { ev.startTime, ev.endTime, name, profileZoneName, ev.eventId, 0, 0, 0, 0 };

				callback.onEvent(ce);
			}
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileZoneHandler::onZoneAdded( PxProfileZone& inSDK )
		{
			if(strstr(mProfileZoneNames,inSDK.getName()))
			{				
				ProfileZoneInterface* pzInt = PX_NEW (ProfileZoneInterface)(inSDK, mBufferedProfiler);
				inSDK.addClient(*pzInt);
				mProfileZoneInterfaces.pushBack(pzInt);
			}
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileZoneHandler::onZoneRemoved( PxProfileZone&  inSDK)	
		{
			for (PxU32 i = mProfileZoneInterfaces.size(); i--;)
			{
				if(&mProfileZoneInterfaces[i]->getProfileZone() == &inSDK)
				{
					inSDK.removeClient(*mProfileZoneInterfaces[i]);
					PX_DELETE(mProfileZoneInterfaces[i]);
					mProfileZoneInterfaces.replaceWithLast(i);
					break;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileZoneHandler::flushEvents(Ps::Array<PxBufferedProfilerCallback*>& callbacks)
		{
			PX_ASSERT(mProfileEventHandler);
			for (PxU32 i = mProfileZoneInterfaces.size(); i--;)
			{
				mProfileZoneInterfaces[i]->setBufferFlushFull(false);
				mProfileEventHandler->setProfileZoneInterface(mProfileZoneInterfaces[i]);
				mProfileZoneInterfaces[i]->getProfileZone().flushProfileEvents();
				mProfileEventHandler->reportEvents(callbacks);
				mProfileZoneInterfaces[i]->setBufferFlushFull(true);
			}
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileZoneHandler::release()
		{
			for (PxU32 i = mProfileZoneInterfaces.size(); i--;)
			{
				mProfileZoneInterfaces[i]->getProfileZone().removeClient(*mProfileZoneInterfaces[i]);
				PX_DELETE(mProfileZoneInterfaces[i]);
			}
			mProfileZoneInterfaces.clear();
		}

		//////////////////////////////////////////////////////////////////////////

		ProfileZoneInterface::ProfileZoneInterface(PxProfileZone& profileZone, DefaultBufferedProfiler* profiler)
			: mProfileZone(profileZone), mDefaultBufferedProfiler(profiler), mBufferFullFlush(true)
		{
			for (PxU32 i = mProfileZone.getProfileNames().mEventCount; i-- ; )
			{
				const PxProfileEventName& evName = mProfileZone.getProfileNames().mEvents[i];
				mIdNameMap[evName.mEventId] = evName.mName;
			}
		}

		//////////////////////////////////////////////////////////////////////////

		const char* ProfileZoneInterface::getName() const 
		{ 
			return mProfileZone.getName(); 
		}

		//////////////////////////////////////////////////////////////////////////

		const char* ProfileZoneInterface::getProfileEventName(const PxU64 id)
		{
			ProfileIdNamePair* item = mIdNameMap.find(id);
			if(item)
				return item->second;

			// try to parse the event name provider, the event id might get added on the fly
			// like PxTaskManager does that
			const PxProfileNames& names = mProfileZone.getProfileNames();
			for (PxU32 i = 0; i < names.mEventCount; i++)
			{
				if(names.mEvents[i].mEventId.mEventId == id)
				{
					mIdNameMap[id] = names.mEvents[i].mName;
					return names.mEvents[i].mName;
				}
			}

			return "<unknown event>";
		}

		//////////////////////////////////////////////////////////////////////////

		void ProfileZoneInterface::handleBufferFlush(const PxU8* inData, PxU32 inLength )
		{
			PX_ASSERT(mDefaultBufferedProfiler);
			if(!mBufferFullFlush)
				PxProfileEventHandler::parseEventBuffer(inData, inLength, mDefaultBufferedProfiler->getProfileEventHandler(), false);
			else
			{				
				Ps::Mutex::ScopedLock lock(mDefaultBufferedProfiler->getBufferedProfilerMutex());
				mDefaultBufferedProfiler->getProfileEventHandler().setProfileZoneInterface(this);
				PxProfileEventHandler::parseEventBuffer(inData, inLength, mDefaultBufferedProfiler->getProfileEventHandler(), false);
				mDefaultBufferedProfiler->getProfileEventHandler().reportEvents(mDefaultBufferedProfiler->getBufferedProfilerCallbacks());
			}
		}

		//////////////////////////////////////////////////////////////////////////

		DefaultBufferedProfiler::DefaultBufferedProfiler(PxFoundation& foundation, const char * profileZoneNames)
			: mProfileZoneManager(NULL), mProfileZoneHandler(&mProfileEventHandler, profileZoneNames)
		{
			mProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(&foundation);
			PX_ASSERT(mProfileZoneManager);
			mProfileZoneHandler.setBufferedProfiler(this);
			mProfileZoneManager->addProfileZoneHandler(mProfileZoneHandler);			
		}

		//////////////////////////////////////////////////////////////////////////

		DefaultBufferedProfiler::~DefaultBufferedProfiler()
		{

		}

		//////////////////////////////////////////////////////////////////////////

		void DefaultBufferedProfiler::flushEvents()
		{
			Ps::Mutex::ScopedLock lock(mMutex);			
			mProfileZoneHandler.flushEvents(mCallbacks);			
		}

		//////////////////////////////////////////////////////////////////////////

		void DefaultBufferedProfiler::addBufferedProfilerCallback(PxBufferedProfilerCallback& cb)
		{
			Ps::Mutex::ScopedLock lock(mMutex);
			mCallbacks.pushBack( &cb );
		}

		//////////////////////////////////////////////////////////////////////////

		void DefaultBufferedProfiler::removeBufferedProfilerCallback(PxBufferedProfilerCallback& cb)
		{
			Ps::Mutex::ScopedLock lock(mMutex);
			for( PxU32 idx = mCallbacks.size(); idx-- ; )
			{
				if ( mCallbacks[idx] == &cb )
				{
					mCallbacks.replaceWithLast( idx );
					break;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////

		void DefaultBufferedProfiler::release()
		{			
			mProfileZoneHandler.release();
			mProfileZoneManager->removeProfileZoneHandler(mProfileZoneHandler);
			mCallbacks.clear();
			mProfileZoneManager->release();

			PX_DELETE(this);
		}

	}
}


