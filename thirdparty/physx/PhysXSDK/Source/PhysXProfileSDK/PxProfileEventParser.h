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


#ifndef PX_PHYSX_PROFILE_EVENT_PARSER_H
#define PX_PHYSX_PROFILE_EVENT_PARSER_H
#include "PxProfileEvents.h"
#include "PxProfileEventSerialization.h"

namespace physx { namespace profile {
	
	struct EventParserData
	{
		EventContextInformation		mContextInformation;
		PxU64						mLastTimestamp;

		EventParserData() : mLastTimestamp( 0 )
		{
		}
	};

	//This object will be copied a lot so all of its important
	//data must be pointers.
	template<typename THandlerType, bool TSwapBytes>
	struct EventParseOperator
	{
		typedef EventDeserializer<TSwapBytes>	TDeserializerType;

		EventParserData*						mData;
		TDeserializerType*						mDeserializer;
		EventHeader*							mHeader;
		THandlerType*							mHandler;

		EventParseOperator( EventParserData* inData, TDeserializerType* inDeserializer, EventHeader* inHeader, THandlerType* inHandler ) 
			: mData( inData )
			, mDeserializer( inDeserializer )
			, mHeader( inHeader )
			, mHandler( inHandler ) 
		{}
		
		template<typename TEventType>
		bool parse( TEventType& ioEvent )
		{
			ioEvent.streamify( *mDeserializer, *mHeader );
			bool success = mDeserializer->mFail == false;
			PX_ASSERT( success );
			return success;
		}

		bool parseHeader( EventHeader& ioEvent )
		{
			ioEvent.streamify( *mDeserializer );
			bool success = mDeserializer->mFail == false;
			PX_ASSERT( success );
			return success;
		}

		template<typename TEventType>
		bool handleProfileEvent( TEventType& evt )
		{
			bool retval = parse( evt );
			if ( retval )
			{
				mData->mContextInformation = evt.mContextInformation;
				handle( evt.getRelativeEvent(), evt.mContextInformation );
			}
			return retval;
		}
		
		template<typename TEventType>
		bool handleRelativeProfileEvent( TEventType& evt )
		{
			bool retval = parse( evt );
			if ( retval )
				handle( evt, mData->mContextInformation );
			return retval;
		}

		template<typename TRelativeEventType>
		void handle( const TRelativeEventType& evt, const EventContextInformation& inInfo )
		{	
			mData->mLastTimestamp = mHeader->uncompressTimestamp( mData->mLastTimestamp, evt.getTimestamp() );
			const_cast<TRelativeEventType&>(evt).setTimestamp( mData->mLastTimestamp );
			evt.handle( mHandler, mHeader->mEventId
						, inInfo.mThreadId
						, inInfo.mContextId
						, inInfo.mCpuId
						, inInfo.mThreadPriority );
		}

		bool operator()( const StartEvent& )
		{
			StartEvent evt;
			return handleProfileEvent( evt );
		}
		bool operator()( const StopEvent& )
		{
			StopEvent evt;
			return handleProfileEvent( evt );
		}
		bool operator()( const RelativeStartEvent& )
		{
			RelativeStartEvent evt;
			return handleRelativeProfileEvent( evt );

		}
		bool operator()( const RelativeStopEvent& )
		{
			RelativeStopEvent evt;
			return handleRelativeProfileEvent( evt );
		}
		bool operator()( const EventValue& )
		{
			EventValue evt;
			bool retval = parse( evt );
			if ( retval )
			{
				evt.mValue = mHeader->uncompressTimestamp( 0, evt.mValue );
				evt.handle( mHandler, mHeader->mEventId );
			}
			return retval;
		}
		bool operator()( const CUDAProfileBuffer& )
		{
			CUDAProfileBuffer evt;
			bool retval = parse( evt );
			if ( retval )
				evt.handle( mHandler );
			return retval;
		}
		//Unknown event type.
		bool operator()(PxU8 )
		{
			PX_ASSERT( false );
			return false;
		}
	};

	template<bool TSwapBytes, typename THandlerType> 
	inline bool parseEventData( const PxU8* inData, PxU32 inLength, THandlerType* inHandler )
	{
		EventDeserializer<TSwapBytes> deserializer( inData, inLength );
		Event::EventData crapData;
		EventHeader theHeader;
		EventParserData theData;
		EventParseOperator<THandlerType,TSwapBytes> theOp( &theData, &deserializer, &theHeader, inHandler );
		while( deserializer.mLength && deserializer.mFail == false)
		{
			if ( theOp.parseHeader( theHeader ) )
				visit<bool>( static_cast<EventTypes::Enum>( theHeader.mEventType ), crapData, theOp );
		}
		return deserializer.mFail == false;
	}
}}

#endif
