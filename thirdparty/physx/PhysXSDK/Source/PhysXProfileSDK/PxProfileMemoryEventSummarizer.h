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


#ifndef PX_PHYSX_PROFILE_MEMORY_EVENT_SUMMARIZER_H
#define PX_PHYSX_PROFILE_MEMORY_EVENT_SUMMARIZER_H
#include "PxProfileBase.h"
#include "PsHashMap.h"
#include "PxProfileFoundationWrapper.h"
#include "PxProfileMemoryEvents.h"
#include "PxProfileMemoryEventRecorder.h"
#include "PxProfileMemoryEventParser.h"

namespace physx { namespace profile {

	struct MemoryEventSummarizerEntry
	{
		PxU32		mType;
		PxU32		mFile;
		PxU32		mLine;

		MemoryEventSummarizerEntry( const AllocationEvent& evt )
			: mType( evt.mType )
			, mFile( evt.mFile )
			, mLine( evt.mLine )
		{
		}

		MemoryEventSummarizerEntry( PxU32 tp, PxU32 f, PxU32 line )
			: mType( tp )
			, mFile( f )
			, mLine( line )
		{
		}
	};
}}

namespace physx { namespace shdfnd {

	template <>
	struct Hash<physx::profile::MemoryEventSummarizerEntry>
	{
	public:
		PxU32 operator()(const physx::profile::MemoryEventSummarizerEntry& entry) const
		{
			//Combine hash values in a semi-reasonable way.
			return Hash<PxU32>()( entry.mType )
					^ Hash<PxU32>()( entry.mFile )
					^ Hash<PxU32>()( entry.mLine );
		}

		bool operator()(const physx::profile::MemoryEventSummarizerEntry& lhs, const physx::profile::MemoryEventSummarizerEntry& rhs) const
		{
			return lhs.mType == rhs.mType
				&& lhs.mFile == rhs.mFile
				&& lhs.mLine == rhs.mLine;
		}
	};
}}

namespace physx { namespace profile {

	struct MemoryEventSummarizerAllocatedValue
	{
		MemoryEventSummarizerEntry	mEntry;
		PxU32						mSize;
		MemoryEventSummarizerAllocatedValue( MemoryEventSummarizerEntry en, PxU32 sz )
			: mEntry( en )
			, mSize( sz )
		{
		}
	};

	template<typename TSummarizerType>
	struct SummarizerParseHandler
	{
		TSummarizerType* mSummarizer;
		SummarizerParseHandler( TSummarizerType* inType )
			: mSummarizer( inType )
		{
		}
		template<typename TDataType>
		void operator()( const MemoryEventHeader& inHeader, const TDataType& inType )
		{
			mSummarizer->handleParsedData( inHeader, inType );
		}
	};

	template<typename TForwardType>
	struct MemoryEventForward
	{
		TForwardType* mForward;
		MemoryEventForward( TForwardType& inForward )
			: mForward( &inForward )
		{
		}
		template<typename TDataType>
		void operator()( const MemoryEventHeader& inHeader, const TDataType& inType )
		{
			TForwardType& theForward( *mForward );
			theForward( inHeader, inType );
		}
	};

	struct NullMemoryEventHandler
	{
		template<typename TDataType>
		void operator()( const MemoryEventHeader&, const TDataType&)
		{
		}
	};

	template<typename TForwardType>
	struct NewEntryOperatorForward
	{
		TForwardType* mForward;
		NewEntryOperatorForward( TForwardType& inForward )
			: mForward( &inForward )
		{
		}
		void operator()( const MemoryEventSummarizerEntry& inEntry, const char* inTypeStr, const char* inFileStr, PxU32 inTotalsArrayIndex )
		{
			TForwardType& theType( *mForward );
			theType( inEntry, inTypeStr, inFileStr, inTotalsArrayIndex );
		}
	};

	struct NullNewEntryOperator
	{
		void operator()( const MemoryEventSummarizerEntry&, const char*, const char*, PxU32)
		{
		}
	};

	//Very specialized class meant to take a stream of memory events
	//endian-convert it.
	//Produce a new stream
	//And keep track of the events in a meaningful way.
	//It collapses the allocations into groupings keyed
	//by file, line, and type.
	template<bool TSwapBytes
			, typename TNewEntryOperator
			, typename MemoryEventHandler>
	struct MemoryEventSummarizer : public PxProfileEventBufferClient
	{
		typedef MemoryEventSummarizer< TSwapBytes, TNewEntryOperator, MemoryEventHandler > TThisType;
		typedef WrapperReflectionAllocator<MemoryEventSummarizerEntry> TAllocatorType;
		typedef shdfnd::HashMap<MemoryEventSummarizerEntry, PxU32, shdfnd::Hash<MemoryEventSummarizerEntry>, TAllocatorType> TSummarizeEntryToU32Hash;
		typedef shdfnd::HashMap<PxU64, MemoryEventSummarizerAllocatedValue, shdfnd::Hash<PxU64>, TAllocatorType> TU64ToSummarizerValueHash;
		FoundationWrapper mWrapper;
		TSummarizeEntryToU32Hash		mEntryIndexHash;
		ProfileArray<PxI32>				mTotalsArray;
		MemoryEventParser<TSwapBytes>	mParser;
		TU64ToSummarizerValueHash		mOutstandingAllocations;
		TNewEntryOperator				mNewEntryOperator;
		MemoryEventHandler				mEventHandler;

		
		MemoryEventSummarizer( PxAllocatorCallback& inAllocator
								, TNewEntryOperator inNewEntryOperator
								, MemoryEventHandler inEventHandler)

			: mWrapper( inAllocator )
			, mEntryIndexHash( TAllocatorType( mWrapper ) )
			, mTotalsArray( mWrapper )
			, mParser( inAllocator )
			, mOutstandingAllocations( mWrapper )
			, mNewEntryOperator( inNewEntryOperator )
			, mEventHandler( inEventHandler )
		{
		}
		virtual ~MemoryEventSummarizer(){}

		//parse this data block.  This will endian-convert the data if necessary
		//and then 
		void handleData( const PxU8* inData, PxU32 inLen )
		{
			SummarizerParseHandler<TThisType> theHandler( this );
			parseEventData<TSwapBytes>( mParser, inData, inLen, &theHandler );
		}

		template<typename TDataType>
		void handleParsedData( const MemoryEventHeader& inHeader, const TDataType& inData )
		{
			//forward it to someone who might care
			mEventHandler( inHeader, inData );
			//handle the parsed data.
			doHandleParsedData( inData );
		}

		template<typename TDataType>
		void doHandleParsedData( const TDataType& ) {}
		
		void doHandleParsedData( const AllocationEvent& inEvt ) 
		{
			onAllocation( inEvt.mSize, inEvt.mType, inEvt.mFile, inEvt.mLine, inEvt.mAddress );
		}
		
		void doHandleParsedData( const DeallocationEvent& inEvt ) 
		{
			onDeallocation( inEvt.mAddress );
		}

		PxU32 getOrCreateEntryIndex( const MemoryEventSummarizerEntry& inEvent )
		{
			PxU32 index = 0;
			const TSummarizeEntryToU32Hash::Entry* entry( mEntryIndexHash.find(inEvent ) );
			if ( !entry )
			{
				index = mTotalsArray.size();
				mTotalsArray.pushBack( 0 );
				mEntryIndexHash.insert( inEvent, index );

				//Force a string lookup and such here.
				mNewEntryOperator( inEvent, mParser.getString( inEvent.mType), mParser.getString( inEvent.mFile ), index );
			}
			else
				index = entry->second;
			return index;
		}

		//Keep a running total of what is going on, letting a listener know when new events happen.
		void onMemoryEvent( const MemoryEventSummarizerEntry& inEvent, PxI32 inSize )
		{
			MemoryEventSummarizerEntry theEntry( inEvent );
			PxU32 index = getOrCreateEntryIndex( theEntry );
			mTotalsArray[index] += inSize;
		}

		void onAllocation( PxU32 inSize, PxU32 inType, PxU32 inFile, PxU32 inLine, PxU64 inAddress )
		{
			MemoryEventSummarizerEntry theEntry( inType, inFile, inLine );
			onMemoryEvent( theEntry, static_cast<PxI32>( inSize ) );
			mOutstandingAllocations.insert( inAddress, MemoryEventSummarizerAllocatedValue( theEntry, inSize ) );
		}

		void onDeallocation( PxU64 inAddress )
		{
			const TU64ToSummarizerValueHash::Entry* existing( mOutstandingAllocations.find( inAddress ) );
			if ( existing )
			{
				const MemoryEventSummarizerAllocatedValue& data( existing->second );
				onMemoryEvent( data.mEntry, -1 * static_cast<PxI32>( data.mSize ) );
				mOutstandingAllocations.erase( inAddress );
			}
			//Not much we can do with an deallocation when we didn't track the allocation.
		}

		PxI32 getTypeTotal( const char* inTypeName, const char* inFilename, PxU32 inLine )
		{
			PxU32 theType( mParser.getHandle( inTypeName ) );
			PxU32 theFile( mParser.getHandle( inFilename ) );
			PxU32 theLine = inLine; //all test lines are 50...
			PxU32 index = getOrCreateEntryIndex( MemoryEventSummarizerEntry( theType, theFile, theLine ) );
			return mTotalsArray[index];
		}

		virtual void handleBufferFlush( const PxU8* inData, PxU32 inLength )
		{
			handleData( inData, inLength );
		}
		
		virtual void handleClientRemoved() {}
	};

}}

#endif
