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


#ifndef PX_PHYSX_PROFILE_MEMORY_EVENT_PARSER_H
#define PX_PHYSX_PROFILE_MEMORY_EVENT_PARSER_H
#include "PxProfileMemoryEvents.h"
#include "PsHashMap.h"
#include "PxProfileFoundationWrapper.h"
#include "PsString.h"

namespace physx { namespace profile {

	template<bool TSwapBytes, typename TParserType, typename THandlerType> 
	bool parseEventData( TParserType& inParser, const PxU8* inData, PxU32 inLength, THandlerType* inHandler );

	template<bool TSwapBytes>
	struct MemoryEventParser
	{
		typedef WrapperReflectionAllocator<PxU8> TAllocatorType;
		typedef shdfnd::HashMap<PxU32, char*, shdfnd::Hash<PxU32>, TAllocatorType > THdlToStringMap;
		typedef EventDeserializer<TSwapBytes>	TDeserializerType;
		
		FoundationWrapper	mWrapper;
		THdlToStringMap		mHdlToStringMap;
		TDeserializerType	mDeserializer;

		MemoryEventParser( PxAllocatorCallback& inAllocator )
			: mWrapper( inAllocator )
			, mHdlToStringMap( TAllocatorType( mWrapper ) )
			, mDeserializer ( 0, 0 )
		{
		}

		~MemoryEventParser()
		{
			for ( THdlToStringMap::Iterator iter( mHdlToStringMap.getIterator() ); iter.done() == false; ++iter )
				mWrapper.getAllocator().deallocate( (void*)iter->second );
		}

		template<typename TOperator>
		void parse(const StringTableEvent&, const MemoryEventHeader& inHeader, TOperator& inOperator)
		{
			StringTableEvent evt;
			evt.init();
			evt.streamify( mDeserializer, inHeader );
			PxU32 len = static_cast<PxU32>( strlen( evt.mString ) );
			char* newStr = static_cast<char*>( mWrapper.getAllocator().allocate( len + 1, "const char*", __FILE__, __LINE__ ) );
			physx::string::strcpy_s( newStr, len+1, evt.mString );
			mHdlToStringMap[evt.mHandle] = newStr;
			inOperator( inHeader, evt );
		}

		const char* getString( PxU32 inHdl )
		{
			const THdlToStringMap::Entry* entry = mHdlToStringMap.find( inHdl );
			if ( entry ) return entry->second;
			return "";
		}

		//Slow reverse lookup used only for testing.
		PxU32 getHandle( const char* inStr )
		{
			for ( THdlToStringMap::Iterator iter = mHdlToStringMap.getIterator();
				!iter.done();
				++iter )
			{
				if ( safeStrEq( iter->second, inStr ) )
					return iter->first;
			}
			return 0;
		}

		template<typename TOperator>
		void parse(const AllocationEvent&, const MemoryEventHeader& inHeader, TOperator& inOperator)
		{
			AllocationEvent evt;
			evt.streamify( mDeserializer, inHeader );
			inOperator( inHeader, evt );
		}

		template<typename TOperator>
		void parse(const DeallocationEvent&, const MemoryEventHeader& inHeader, TOperator& inOperator)
		{
			DeallocationEvent evt;
			evt.streamify( mDeserializer, inHeader );
			inOperator( inHeader, evt );
		}

		template<typename TOperator>
		void parse(const FullAllocationEvent&, const MemoryEventHeader&, TOperator& )
		{
			PX_ASSERT( false ); //will never happen.
		}

		template<typename THandlerType>
		void parseEventData( const PxU8* inData, PxU32 inLength, THandlerType* inOperator )
		{
			physx::profile::parseEventData<TSwapBytes>( *this, inData, inLength, inOperator );
		}
	};
	

	template<typename THandlerType, bool TSwapBytes>
	struct MemoryEventParseOperator
	{
		MemoryEventParser<TSwapBytes>* mParser;
		THandlerType* mOperator;
		MemoryEventHeader* mHeader;
		MemoryEventParseOperator( MemoryEventParser<TSwapBytes>* inParser, THandlerType* inOperator, MemoryEventHeader* inHeader )
			: mParser( inParser )
			, mOperator( inOperator )
			, mHeader( inHeader )
		{
		}

		bool wasSuccessful() { return mParser->mDeserializer.mFail == false; }

		bool parseHeader()
		{
			mHeader->streamify( mParser->mDeserializer );
			return wasSuccessful();
		}

		template<typename TDataType>
		bool operator()( const TDataType& inType )
		{
			mParser->parse( inType, *mHeader, *mOperator );
			return wasSuccessful();
		}
		
		bool operator()( PxU8 ) { PX_ASSERT( false ); return false;}
	};

	template<bool TSwapBytes, typename TParserType, typename THandlerType> 
	inline bool parseEventData( TParserType& inParser, const PxU8* inData, PxU32 inLength, THandlerType* inHandler )
	{
		inParser.mDeserializer = EventDeserializer<TSwapBytes>( inData, inLength );
		MemoryEvent::EventData crapData;
		PxU32 eventCount = 0;
		MemoryEventHeader theHeader;
		MemoryEventParseOperator<THandlerType, TSwapBytes> theOp( &inParser, inHandler, &theHeader );
		while( inParser.mDeserializer.mLength && inParser.mDeserializer.mFail == false)
		{
			if ( theOp.parseHeader() )
			{
				if( visit<bool>( theHeader.getType(), crapData, theOp ) == false )
					inParser.mDeserializer.mFail = true;
			}
			++eventCount;
		}
		return inParser.mDeserializer.mFail == false;
	}
}}

#endif
