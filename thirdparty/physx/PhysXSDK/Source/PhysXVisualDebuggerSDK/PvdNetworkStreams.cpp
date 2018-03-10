/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
#include "PvdNetworkStreams.h"
#include "PxSimpleTypes.h"
#include "PsSync.h"
#include "PsMutex.h"
#include "PvdFoundation.h"
#include "PsThread.h"
#include "PsSocket.h"
#include "PsFile.h"
#include <stdio.h>

using namespace physx::debugger;

namespace {
	
	class PvdDoubleBufferedWriteBuffer
	{
		PX_NOCOPY(PvdDoubleBufferedWriteBuffer)
	public:
		typedef ForwardingArray<PxU8>		TArrayType;
		typedef MutexT<ForwardingAllocator> TMutexType;
		typedef TMutexType::ScopedLock		TLockType;
		struct ReadResult
		{
			const PxU8*			mPtr;
			PxU32				mLen;

			ReadResult( const PxU8* inPtr, PxU32 inLen )
				: mPtr( inPtr )
				, mLen( inLen )
			{
			}
		};

	private:

		PxU32		mCapacity;
		TArrayType	mFirstArray;
		TArrayType	mSecondArray;


		PxU8*		mWritePtr;
		PxU8*		mWriteEndPtr;

		TMutexType	mMutex;
		Sync		mReadyToWrite;
		Sync		mReadyToRead;

	public:
		PvdDoubleBufferedWriteBuffer( PxAllocatorCallback& alloc, PxU32 inCap )
			: mCapacity( inCap / 2 )
			, mFirstArray( alloc, "PvdDoubleBufferedWriteBuffer::mFirstArray" )
			, mSecondArray( alloc, "PvdDoubleBufferedWriteBuffer::mSecondArray" )
			, mMutex( ForwardingAllocator( alloc, "PvdDoubleBufferedWriteBuffer::mMutex" ) )
		{
			mFirstArray.resize( mCapacity );
			mSecondArray.resize( mCapacity );

			mWritePtr = mFirstArray.begin();
			mWriteEndPtr = mFirstArray.end();
		}

		//Write data to the write buffer.
		PxU32 write( const PxU8* inData, PxU32 inLen )
		{
			PxU32 nbBytesCanWrite = 0;
			{
				TLockType theLock( mMutex );
				nbBytesCanWrite = PxMin( static_cast<PxU32>( mWriteEndPtr - mWritePtr ), inLen );
				if ( nbBytesCanWrite )
				{
					PxMemCopy( mWritePtr, inData, nbBytesCanWrite );
					mWritePtr += nbBytesCanWrite;
				}

				//If current buffer is full, other thread cannot write
				if(mWriteEndPtr - mWritePtr == 0)
					mReadyToWrite.reset();
			}
			mReadyToRead.set();
			return nbBytesCanWrite;
		}

		//DoubleBufferedOutStream::flush() need access this sync
		void signalReadReady()
		{
			mReadyToRead.set();
		}

		bool hasData()
		{
			TLockType theLock( mMutex );
			return mCapacity - static_cast<PxU32>( mWriteEndPtr - mWritePtr ) > 0;
		}

		//If there is any data in the write buffer, swap the
		//write buffer with the read buffer and return the data.
		ReadResult read()
		{
			const PxU8* theReadPtr = NULL;
			PxU32				theReadSize = 0;
			{
				TLockType theLock( mMutex );
				PxU32 nbBytesCanRead = mCapacity - static_cast<PxU32>( mWriteEndPtr - mWritePtr );
				if ( nbBytesCanRead )
				{
					//Swap the read/write pointers with the mutex held
					bool usingSecondArray = mWriteEndPtr == mSecondArray.end();
					const PxU8* theBeginPtr( mFirstArray.begin() );
					mWritePtr = mSecondArray.begin();
					mWriteEndPtr = mSecondArray.end();
					if ( usingSecondArray )
					{
						theBeginPtr = mSecondArray.begin();
						mWritePtr = mFirstArray.begin();
						mWriteEndPtr = mFirstArray.end();
					}
					theReadPtr = theBeginPtr;
					theReadSize = nbBytesCanRead;
				}
				else 
					mReadyToRead.reset();
			}
			mReadyToWrite.set();
			return ReadResult( theReadPtr, theReadSize );
		}

		void waitTillReadyToWrite( PxU32 inMilliseconds ) { mReadyToWrite.wait( inMilliseconds ); }
		bool waitTillReadyToRead( PxU32 inMilliseconds ) { return mReadyToRead.wait( inMilliseconds ); }
	};

	class DoubleBufferedOutStream : public PvdNetworkOutStream, public Thread
	{
		typedef PvdDoubleBufferedWriteBuffer TBufferType;
		typedef TBufferType::ReadResult TReadResult;
		typedef MutexT<ForwardingAllocator> TMutexType;
		typedef TMutexType::ScopedLock		TLockType;

		PxAllocatorCallback&	mAllocator;
		TBufferType				mBuffer;
		PvdNetworkOutStream*	mOutStream;
		TMutexType				mMutex;

	public:
		DoubleBufferedOutStream( PxAllocatorCallback& alloc, PxU32 inCapacity, PvdNetworkOutStream& inOutStream )
			: mAllocator( alloc )
			, mBuffer( alloc, inCapacity )
			, mOutStream( &inOutStream )
			, mMutex( ForwardingAllocator( alloc, "DoubleBufferedOutStream::mMutex" ) )
		{
			//16K of stack space.  I have no idea how little I could allocate
			//at this point.
			start( 0x4000 ); 
		}
		virtual ~DoubleBufferedOutStream()
		{
			signalQuit();
			waitForQuit();
			mOutStream->disconnect();
			mOutStream->release();
			mOutStream = NULL;
		}

		virtual PvdError write( const PxU8* inBytes, PxU32 inLength )
		{
			if ( mOutStream == NULL )
				return PvdErrorType::NetworkError;

			bool connected = mOutStream->isConnected();
			while( inLength && connected )
			{
				PxU32 written = mBuffer.write( inBytes, inLength );
				inLength -= written;
				if ( inLength )
				{
					inBytes += written;
					mBuffer.waitTillReadyToWrite(1);
					connected = mOutStream->isConnected();
				}
			}
			return connected ? PvdErrorType::Success : PvdErrorType::NetworkError;
		}
		
		/**
		 *	Return true if this stream is still connected.
		 */
		virtual bool isConnected() const { if ( mOutStream ) return mOutStream->isConnected();  return false; }
		/**
		 *	Close the in stream.
		 */
		virtual void disconnect() 
		{ 
			if ( mOutStream )
				mOutStream->disconnect(); 
		}
		/**
		 *	release any resources related to this stream.
		 */
		virtual void release()
		{
			PVD_DELETE( mAllocator, this );
		}

		virtual PxU64 getWrittenDataSize()
		{
			return ( mOutStream != NULL )? mOutStream->getWrittenDataSize() : 0;
		}

		bool checkFlushCondition()
		{
			TLockType locker( mMutex );
			return mBuffer.hasData() && mOutStream->isConnected();
		}
		
		virtual PvdError flush()
		{
			while( checkFlushCondition() )
			{
				mBuffer.signalReadReady();
				mBuffer.waitTillReadyToWrite(1);
				//toggle mutex so we know everything that can be written has been written.
				mMutex.lock();
				mMutex.unlock();
			}
			return mOutStream->isConnected() ? PvdErrorType::Success : PvdErrorType::NetworkError;
		}
		
		//The thread execute fn.
		virtual void execute()
		{
			setName("PVD::OutStream");
			while( !quitIsSignalled() && mOutStream->isConnected() )
			{
				if(mBuffer.waitTillReadyToRead(100))
				{
					TLockType lock( mMutex );
					TReadResult theResult = mBuffer.read();
					if ( theResult.mLen )
						mOutStream->write( theResult.mPtr, theResult.mLen );
				}
			}
			quit();
		}

	private:
		DoubleBufferedOutStream& operator=(const DoubleBufferedOutStream&);
		DoubleBufferedOutStream(DoubleBufferedOutStream &);
	};

	class StreamOwner : public PvdNetworkStreamOwner, public PvdNetworkInStream, public PvdNetworkOutStream
	{
		typedef MutexT<ForwardingAllocator> TMutexType;
		typedef TMutexType::ScopedLock		TScopedLockType;
		PxAllocatorCallback&	mAllocator;
		PvdNetworkOutStream*	mOutStream;
		PvdNetworkInStream*		mInStream;
		PxU32					mRefCount;
		mutable TMutexType		mMutex;
		mutable TMutexType		mReadMutex;
		bool					mLocked;
	public:
		StreamOwner( PxAllocatorCallback& alloc, PvdNetworkOutStream& inOutStream, PvdNetworkInStream* inInStream )
			: mAllocator( alloc )
			, mOutStream( &inOutStream )
			, mInStream( inInStream )
			, mRefCount( 0 )
			, mMutex( ForwardingAllocator( alloc, "StreamOwner::mMutex" ) )
			, mReadMutex( ForwardingAllocator( alloc, "StreamOwner::mReadMutex" ) )
			, mLocked( false )
		{
		}
		PX_INLINE void addRef()
		{
			TScopedLockType theLocker( mMutex );
			++mRefCount;
		}
		void disconnect()
		{
			{
				TScopedLockType theLocker( mMutex );
				PvdNetworkOutStream* theOutStream = mOutStream;
				mOutStream = NULL;
				if ( theOutStream ) theOutStream->release();
			}
			{
				TScopedLockType theReadLocker( mReadMutex );
				PvdNetworkInStream* theInStream = mInStream;
				mInStream = NULL;
				if ( theInStream ) theInStream->release();
			}
		}

		void release()
		{
			PxU32 theTempCount = 0;
			{
				TScopedLockType theLocker( mMutex );
				if ( mRefCount )
					--mRefCount;
				theTempCount = mRefCount;
				if ( !theTempCount )
					disconnect();
			}
			//Unlock the mutex.
			if ( !theTempCount )
				PVD_DELETE( mAllocator, this );
		}



		PvdError processError( PvdError error )
		{
			if ( error != PvdErrorType::Success )
				disconnect();
			return error;
		}

		template<typename TStreamType>
		PvdError processError( PvdError error, TStreamType* inStream )
		{
			if ( error != PvdErrorType::Success )
				inStream->disconnect();
			return error;
		}

#define CHECK_STREAM_CONNECTION( stream ) if ( !stream || !stream->isConnected() ) return PvdErrorType::NetworkError

		/**
		 * Out stream implementation
		 */
		PvdError write( const PxU8* inBytes, PxU32 inLength )
		{
			PX_ASSERT( mLocked ); //Assert that begin/end send were called before write
			CHECK_STREAM_CONNECTION( mOutStream );
			//This disconnect cannot be a full disconnect *because*
			//our send lock is held during this time.
			return processError( mOutStream->write( inBytes, inLength ), mOutStream );
		}
		
		PvdError readBytes( PxU8* outBytes, PxU32 ioRequested )
		{
			PvdError error = PvdErrorType::Success;
			{
				TScopedLockType theLocker( mReadMutex );
				CHECK_STREAM_CONNECTION( mInStream );
				error = mInStream->readBytes( outBytes, ioRequested );
			}
			//Process error calls disconnect which locks the write mutex, then the read mutex.
			//So the read mutex *must* be unlocked for this to work correctly.
			return processError( error );
		}

		bool isConnected() const
		{
			{
				TScopedLockType theLocker( mMutex );
				if ( mOutStream != NULL ) return mOutStream->isConnected();
			}
			{
				TScopedLockType theLocker(mReadMutex);
				if ( mInStream != NULL ) return mInStream->isConnected();
			}
			return false;
		}

		PvdError flush()
		{
			TScopedLockType theLocker( mMutex );
			CHECK_STREAM_CONNECTION( mOutStream );
			return mOutStream->flush();
		}

		bool checkConnected()
		{
			if ( !isConnected() )
			{
				disconnect();
				return false;
			}
			return true;
		}

		PvdNetworkOutStream& lock()
		{
			checkConnected();
			mMutex.lock();
			PX_ASSERT( !mLocked ); //Assert we aren't double locked on the same thread
			mLocked = true;
			return *this;
		}

		void unlock()
		{
			PX_ASSERT( mLocked ); //Assert we were locked in the first place
			mLocked = false;
			mMutex.unlock();
		}
		
		virtual PvdNetworkInStream* getInStream() 
		{
			if ( mInStream ) return this;
			return NULL;
		}

		virtual PxU64 getWrittenDataSize()
		{
			return ( mOutStream == NULL )? 0 : mOutStream->getWrittenDataSize();
		}

		virtual PxU64 getLoadedDataSize()
		{
			return ( mInStream == NULL )? 0 : mInStream->getLoadedDataSize();
		}
	private:
		StreamOwner& operator=(const StreamOwner&);
		StreamOwner(const StreamOwner &);
	};


#ifdef PX_WIIU
	typedef physx::FILE FStreamFile;
#else
	typedef FILE FStreamFile;
#endif
	class FStreamNetworkOutStream : public PvdNetworkOutStream
	{
		PxAllocatorCallback&				mAllocator;
		FStreamFile*						mFile;
		typedef MutexT<ForwardingAllocator> TMutexType;
		typedef TMutexType::ScopedLock		TScopedLockType;
		mutable TMutexType					mMutex;
		volatile bool						mConnected;
		volatile PxU64						mWrittenData;

	public:
		FStreamNetworkOutStream(PxAllocatorCallback& alloc, FStreamFile* file)
			: mAllocator( alloc )
			, mFile( file )
			, mMutex( ForwardingAllocator( alloc, "FStreamNetworkOutStream::mMutex" ) )
			, mConnected( true )
			, mWrittenData(0)
		{
		}

		virtual ~FStreamNetworkOutStream() 
		{
			if ( mFile != NULL )
			{
				fflush( mFile );
				fclose( mFile );
			}
			mFile = NULL;
		}


		virtual PvdError write( const PxU8* inBytes, PxU32 inLength )
		{
			if ( inBytes && inLength && mConnected )
			{
				fwrite( inBytes, 1, inLength, mFile );
				{
					TScopedLockType theLocker(mMutex);
					mWrittenData += inLength;
				}
			}
			return PvdErrorType::Success;
		}
		virtual PvdError flush() { fflush( mFile ); return PvdErrorType::Success; }
		virtual bool isConnected() const { return mConnected; }
		virtual void disconnect() { mConnected = false; }
		virtual void release() { PVD_DELETE( mAllocator, this ); }
		virtual PxU64 getWrittenDataSize()
		{
			TScopedLockType theLocker(mMutex);
			return mWrittenData;
		}
	private:
		FStreamNetworkOutStream& operator=(const FStreamNetworkOutStream&);
		FStreamNetworkOutStream(const FStreamNetworkOutStream &);
	};

	
	
	class SocketInfo
	{
		PxAllocatorCallback& mCallback;
		physx::shdfnd::Socket mSocket;
		PxU32		mRefCount;

	public:
		SocketInfo(PxAllocatorCallback& alloc) 
			: mCallback( alloc )
			, mSocket( false )
			, mRefCount( 0 ) {}

		~SocketInfo()
		{
			mSocket.disconnect();
		}

		physx::shdfnd::Socket& Socket() { return mSocket; }
		void addRef(){ ++mRefCount; }
		void release()
		{
			if ( mRefCount ) -- mRefCount;
			if ( !mRefCount ) PVD_DELETE( mCallback, this );
		}

	private:
		SocketInfo& operator=(const SocketInfo&);
	};

	//No caching at all here, the caching is either a layer above here
	//or a layer below.
	class SocketNetworkOutStream : public PvdNetworkOutStream
	{
		PxAllocatorCallback& mCallback;
		SocketInfo& mInfo;
		typedef MutexT<ForwardingAllocator> TMutexType;
		typedef TMutexType::ScopedLock		TScopedLockType;
		mutable TMutexType					mMutex;
		volatile PxU64						mWrittenData;

	public:
		SocketNetworkOutStream( PxAllocatorCallback& callback, SocketInfo& inInfo ) 
			: mCallback( callback )
			, mInfo( inInfo ) 
			, mMutex( ForwardingAllocator( callback, "SocketNetworkOutStream::mMutex" ) )
			, mWrittenData(0)
		{ 
			mInfo.addRef();
		}

		virtual ~SocketNetworkOutStream(){ mInfo.release(); }
	
		virtual PvdError write( const PxU8* inBytes, PxU32 inLength )
		{
			return doWriteBytes( inBytes, inLength );
		}

		PvdError doWriteBytes( const PxU8* inBytes, PxU32 inLength )
		{
			if ( inLength == 0 )
				return PvdErrorType::Success;

			PxU32 amountWritten = 0;
			PxU32 totalWritten = 0;
			do
			{
				//Sockets don't have to write as much as requested, so we need
				//to wrap this call in a do/while loop.
				//If they don't write any bytes then we consider them disconnected.
				amountWritten = mInfo.Socket().write( inBytes, inLength );
				inLength -= amountWritten;
				inBytes += amountWritten;
				totalWritten += amountWritten;
			}while( inLength && amountWritten );
			
			if ( amountWritten == 0 ) return PvdErrorType::NetworkError;

			{
				TScopedLockType theLocker(mMutex);
				mWrittenData += totalWritten;
			}

			return PvdErrorType::Success;
		}

		virtual PvdError flush() 
		{ 
			return mInfo.Socket().flush() == true ?  PvdErrorType::Success : PvdErrorType::NetworkError;
		}

		virtual bool isConnected() const { return mInfo.Socket().isConnected(); }
		virtual void disconnect() { mInfo.Socket().disconnect(); }
		virtual void release() { PVD_DELETE( mCallback, this ); }
		virtual PxU64 getWrittenDataSize()
		{
			TScopedLockType theLocker(mMutex);
			return mWrittenData;
		}

	private:
		SocketNetworkOutStream& operator=(const SocketNetworkOutStream&);
		SocketNetworkOutStream(const SocketNetworkOutStream &);
	};

	class SocketNetworkInStream : public PvdNetworkInStream
	{
		PxAllocatorCallback& mAllocator;
		SocketInfo& mInfo;

		typedef MutexT<ForwardingAllocator> TMutexType;
		typedef TMutexType::ScopedLock		TScopedLockType;
		mutable TMutexType					mMutex;
		volatile PxU64						mReadData;
	public:
		SocketNetworkInStream( PxAllocatorCallback& alloc, SocketInfo& inInfo ) 
			: mAllocator( alloc )
			, mInfo( inInfo ) 
			, mMutex( ForwardingAllocator( alloc, "SocketNetworkInStream::mMutex" ) )
		{ 
			mInfo.addRef(); 
		}
		virtual ~SocketNetworkInStream(){ mInfo.release(); }
		virtual PvdError readBytes( PxU8* outBytes, PxU32 ioRequested )
		{
			if ( !isConnected() || outBytes == NULL )
				return PvdErrorType::NetworkError;
			if ( ioRequested == 0 )
				return PvdErrorType::Success;
			PxU32 amountRead = 0;
			PxU32 totalRead = 0;
			do
			{
				amountRead = mInfo.Socket().read( outBytes, ioRequested );
				ioRequested -= amountRead;
				totalRead += amountRead;
			}while( ioRequested && amountRead );

			if ( amountRead == 0 )
				return PvdErrorType::NetworkError;

			{
				TScopedLockType theLocker(mMutex);
				mReadData += totalRead;
			}
			
			return PvdErrorType::Success;
		}
		virtual bool isConnected() const
		{
			return mInfo.Socket().isConnected();
		}
		virtual void disconnect() { mInfo.Socket().disconnect(); }
		virtual void release() { PVD_DELETE( mAllocator, this ); }

		virtual PxU64 getLoadedDataSize()
		{
			TScopedLockType theLocker(mMutex);
			return mReadData;
		}
	private:
		SocketNetworkInStream& operator=(const SocketNetworkInStream&);
		SocketNetworkInStream(const SocketNetworkInStream &);
	};
}

PvdNetworkOutStream& PvdNetworkOutStream::createDoubleBuffered( PxAllocatorCallback& alloc, PvdNetworkOutStream& stream, PxU32 bufSize )
{
	return *PVD_NEW( alloc, DoubleBufferedOutStream )( alloc, bufSize, stream );
}


PvdNetworkStreamOwner& PvdNetworkStreamOwner::create( PxAllocatorCallback& alloc, PvdNetworkOutStream& outStream, PvdNetworkInStream* inStream )
{
	return *PVD_NEW( alloc, StreamOwner )( alloc, outStream, inStream );
}

#ifdef PX_VC
#pragma warning(disable:4996)
#endif

PvdNetworkOutStream* PvdNetworkOutStream::createFromFile( PxAllocatorCallback& alloc, const char* fname )
{
	FStreamFile* file = NULL;
	physx::shdfnd::fopen_s(&file, fname, "wb");
	if ( file != NULL )
		return PVD_NEW( alloc, FStreamNetworkOutStream )( alloc, file );
	return NULL;
}



bool PvdNetworkStreams::connect( PxAllocatorCallback& allocator
				, const char* inHost
				, int inPort
				, unsigned int inTimeoutInMilliseconds
				, PvdNetworkInStream*& outInStream
				, PvdNetworkOutStream*& outOutStream )
{
	outInStream = NULL;
	outOutStream = NULL;
	SocketInfo* theInfo = PVD_NEW(allocator, SocketInfo)(allocator);
	if ( theInfo->Socket().connect( inHost, (PxU16)inPort, inTimeoutInMilliseconds ) )
	{
		theInfo->Socket().setBlocking( true );
		outInStream = PVD_NEW( allocator, SocketNetworkInStream )( allocator, *theInfo );
		outOutStream = PVD_NEW( allocator, SocketNetworkOutStream )( allocator, *theInfo );
		return true;
	}
	theInfo->addRef();
	theInfo->release();
	return false;
}
