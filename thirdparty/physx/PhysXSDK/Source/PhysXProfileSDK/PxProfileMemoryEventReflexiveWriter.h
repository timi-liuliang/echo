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

#ifndef PX_PROFILE_MEMORY_EVENT_REFLEXIVE_WRITER_H
#define PX_PROFILE_MEMORY_EVENT_REFLEXIVE_WRITER_H
#include "PxProfileMemoryBuffer.h"
#include "PxProfileFoundationWrapper.h"
#include "PxProfileEventSerialization.h"
#include "PxProfileMemoryEvents.h"

namespace physx { namespace profile {

	struct MemoryEventReflexiveWriter
	{
		typedef WrapperReflectionAllocator<PxU8>	TAllocatorType;
		typedef MemoryBuffer<TAllocatorType>		TMemoryBufferType;
		typedef EventSerializer<TMemoryBufferType>	TSerializerType;


		FoundationWrapper	mWrapper;
		TMemoryBufferType	mBuffer;
		TSerializerType		mSerializer;

		MemoryEventReflexiveWriter( PxFoundation* inFoundation )
			: mWrapper( inFoundation )
			, mBuffer( TAllocatorType( mWrapper ) )
			, mSerializer( &mBuffer )
		{
		}

		template<typename TDataType>
		void operator()( const MemoryEventHeader& inHeader, const TDataType& inType )
		{
			//copy to get rid of const.
			MemoryEventHeader theHeader( inHeader );
			TDataType theData( inType );

			//write them out.
			theHeader.streamify( mSerializer );
			theData.streamify( mSerializer, theHeader );
		}
	};
}}

#endif