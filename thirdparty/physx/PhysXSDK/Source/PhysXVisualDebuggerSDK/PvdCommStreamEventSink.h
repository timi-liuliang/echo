/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef PVD_COMM_STREAM_EVENT_SINK_H
#define PVD_COMM_STREAM_EVENT_SINK_H
#include "PvdObjectModelBaseTypes.h"
#include "PvdCommStreamEvents.h"
#include "PvdCommStreamTypes.h"

namespace physx { namespace debugger {
	class PvdOutputStream;
	class PvdNetworkOutStream;
}}

namespace physx { namespace debugger { namespace comm {


	class PvdCommStreamEventBufferClient
	{
	protected:
		virtual ~PvdCommStreamEventBufferClient(){}
	public:
		virtual void addRef() = 0;
		virtual void release() = 0;
		virtual bool sendEventBuffer( DataRef<const PxU8> eventBuffer, PxU32 eventCount, PxU64 streamId ) = 0;

		virtual PvdNetworkOutStream& lock( PxU32 numBytes, PxU32 eventCount, PxU64 streamId ) = 0;
		virtual bool isConnected() = 0;
		virtual void unlock() = 0;
	};

	class PvdCommStreamEventSink
	{
	public:
		template<typename TStreamType>
		static void writeEvent( const EventSerializeable& evt, PvdCommStreamEventTypes::Enum evtType, TStreamType& stream )
		{
			EventStreamifier<TStreamType> streamifier_concrete( stream );
			EventSerializer& streamifier( streamifier_concrete );
			streamifier.streamify( evtType );
			const_cast<EventSerializeable&>( evt ).serialize( streamifier );
		}
	};

}}}
#endif
