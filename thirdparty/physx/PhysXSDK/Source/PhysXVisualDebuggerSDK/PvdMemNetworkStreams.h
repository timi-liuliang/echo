/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
#ifndef PVD_MEM_NETWORK_STREAMS_H
#define PVD_MEM_NETWORK_STREAMS_H
#include "physxvisualdebuggersdk/PvdNetworkStreams.h"
#include "PvdFoundation.h"

namespace physx { namespace debugger {


	struct MemPvdNetworkOutStream : public PvdNetworkOutStream
	{
		PxAllocatorCallback&	mAllocator;
		ForwardingMemoryBuffer	mBuffer;
		bool					mConnected;
		PxU32					mRefCount;
		PxU32					mDataWritten;

		typedef MutexT<ForwardingAllocator> TMutexType;
		typedef TMutexType::ScopedLock		TScopedLockType;
		mutable TMutexType					mMutex;

		MemPvdNetworkOutStream( PxAllocatorCallback& alloc, String streamName )
			: mAllocator(alloc)
			, mBuffer( alloc, streamName )
			, mConnected( true )
			, mRefCount( 0 )
			, mDataWritten( 0 )
			, mMutex( ForwardingAllocator( alloc, "MemPvdNetworkOutStream::mMutex" ) )
		{
		}
		
		virtual PvdError write( const PxU8* inBytes, PxU32 inLength )
		{
			if ( !mConnected ) { PX_ASSERT( false ); return PvdErrorType::NetworkError; }
			mBuffer.write( inBytes, inLength );
			{
				TScopedLockType theLocker(mMutex);
				mDataWritten += inLength;
			}
			return PvdErrorType::Success;
		}

		virtual bool isConnected() const { return mConnected; }
		virtual void disconnect() { mConnected = false; }
		virtual PvdError flush() { return PvdErrorType::Success; }
		virtual void destroy() { release(); }
		virtual void addRef() { ++mRefCount; }
		virtual void release()
		{
			if ( mRefCount ) --mRefCount;
			if ( !mRefCount ) PVD_DELETE( mBuffer.getAllocator(), this );
		}
		virtual PxU64 getWrittenDataSize()
		{
			TScopedLockType theLocker(mMutex);
			return mDataWritten;
		}
	};
}}

#endif