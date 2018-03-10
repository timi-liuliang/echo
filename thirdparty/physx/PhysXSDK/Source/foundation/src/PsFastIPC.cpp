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
#include "foundation/PxPreprocessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "PxSimpleTypes.h"
#include "PsFastIPC.h"
#include "PsMemoryMappedFile.h"
#include "PsIPC.h"
#include "PsString.h"
#include "PsRingBuffer.h"

#pragma warning(disable:4996)

namespace physx
{
	namespace shdfnd
	{

class FastIPC : public PxIPC, public UserAllocated
{
private: // internal data structures and classes used to handle the ring buffers.


// this is part of the memory mapped file
struct FastIPCRingBuffer
{
	PxU32	mBufferSize;
	PxU32	mWriteAddress;
	PxU32	mReadAddress;
};

// this is part of the memory mapped file.
struct FastIPCHeader
{
	bool				mIsServer;
	bool				mIsClient;
	FastIPCRingBuffer	mServerRingBuffer;
	FastIPCRingBuffer	mClientRingBuffer;
};

public:

	FastIPC(PxIPC::ErrorCode &errorCode,
		PxIPC::ConnectionType connectionType,
		const char *mappingObject,
		PxU32 serverRingBufferSize,
		PxU32 clientRingBufferSize,
		bool allowLongMessages,
		bool bufferSends,
		PxU32 maxBufferSendSize)
	{
		mSendThreadID = 0;
		mReceiveThreadID = 0;
		errorCode = EC_MAPFILE_CREATE;
		PxU32 mapSize = clientRingBufferSize+serverRingBufferSize+sizeof(FastIPCHeader);
		mIsServer = false;
		mHeader = NULL;
		mMemoryMappedFile = PX_NEW(MemoryMappedFile)(mappingObject,mapSize);
		if ( mMemoryMappedFile )
		{
			mHeader = (FastIPCHeader *)mMemoryMappedFile->getBaseAddress();
		}

		if ( mHeader )
		{
			char *serverBaseAddress = (char *)(mHeader+1);
			char *clientBaseAddress = serverBaseAddress+serverRingBufferSize;

			errorCode = EC_OK;

			mServerRingBuffer.init(serverBaseAddress,
								   serverRingBufferSize,
								   allowLongMessages,
								   bufferSends,
								   maxBufferSendSize,
								   &mHeader->mServerRingBuffer.mWriteAddress,
								   &mHeader->mServerRingBuffer.mReadAddress);

			mClientRingBuffer.init(clientBaseAddress,
									clientRingBufferSize,
								   allowLongMessages,
								   bufferSends,
								   maxBufferSendSize,
									&mHeader->mClientRingBuffer.mWriteAddress,
									&mHeader->mClientRingBuffer.mReadAddress);


			switch ( connectionType )
			{
				case CT_CLIENT: 					// start up as a client, will succeed even if the server has not yet been found.
				case CT_CLIENT_REQUIRE_SERVER:   // start up as a client, but only if the server already exists.
					if ( mHeader->mIsClient ) // if there is already a client, we can't create another one.
					{
						errorCode = EC_CLIENT_ALREADY_EXISTS;
					}
					else if ( mHeader->mIsServer) // if there is already as server, make sure the buffer sizes match
					{
						if ( mHeader->mClientRingBuffer.mBufferSize != clientRingBufferSize ||
							 mHeader->mServerRingBuffer.mBufferSize != serverRingBufferSize )
						{
							errorCode = EC_BUFFER_MISSMATCH;
						}
					}
					else if ( connectionType == CT_CLIENT_REQUIRE_SERVER ) // if we require a server to already be present, then we fail.
					{
						errorCode = EC_SERVER_NOT_FOUND;
					}
					// if everything passes, set up the client
					if ( errorCode == EC_OK )
					{
						mHeader->mIsClient = true; // snarf the fact that we are the client.
						mHeader->mClientRingBuffer.mBufferSize = clientRingBufferSize;
						if ( !mHeader->mIsServer )
						{
							mHeader->mServerRingBuffer.mBufferSize = serverRingBufferSize;
						}

						mReader = &mServerRingBuffer;
						mWriter = &mClientRingBuffer;
					}
					break;
				case CT_SERVER: 					// will start up as a server, will fail if an existing server is already open.
					if ( mHeader->mIsServer ) // if there is already a client, we can't create another one.
					{
						errorCode = EC_SERVER_ALREADY_EXISTS;
					}
					else if ( mHeader->mIsClient) // if there is already as client, make sure the buffer sizes match
					{
						if ( mHeader->mClientRingBuffer.mBufferSize != clientRingBufferSize ||
							 mHeader->mServerRingBuffer.mBufferSize != serverRingBufferSize )
						{
							errorCode = EC_BUFFER_MISSMATCH;
						}
					}
					if ( errorCode == EC_OK )
					{
						mHeader->mIsServer = true; // snarf the fact that we are the client.
						mHeader->mClientRingBuffer.mBufferSize = clientRingBufferSize;
						if ( !mHeader->mIsClient )
						{
							mHeader->mServerRingBuffer.mBufferSize = serverRingBufferSize;
						}

						mWriter = &mServerRingBuffer;
						mReader = &mClientRingBuffer;
						mIsServer = true;
					}
					break;
				case CT_CLIENT_OR_SERVER:  		// connect as either a client or server, don't care who is created first.
					if ( mHeader->mIsClient && mHeader->mIsServer ) // if there is already both an active client and server, we fail
					{
						errorCode = EC_CLIENT_SERVER_ALREADY_EXISTS;
					}
					else if ( mHeader->mIsServer || mHeader->mIsClient )
					{
						if ( mHeader->mClientRingBuffer.mBufferSize != clientRingBufferSize ||
							 mHeader->mServerRingBuffer.mBufferSize != serverRingBufferSize )
						{
							errorCode = EC_BUFFER_MISSMATCH;
						}
					}
					if ( errorCode == EC_OK )
					{
						if ( mHeader->mIsServer )
						{
  								mHeader->mIsClient = true; // set the fact that we are the client.
								mReader = &mServerRingBuffer;
      							mWriter = &mClientRingBuffer;
  								mHeader->mServerRingBuffer.mBufferSize = clientRingBufferSize;
						}
						else
						{
  								mHeader->mIsServer = true; // snarf the fact that we are the client.
      							mWriter = &mServerRingBuffer;
      							mReader = &mClientRingBuffer;
  								mHeader->mClientRingBuffer.mBufferSize = clientRingBufferSize;
  								mIsServer = true;
						}
						mHeader->mClientRingBuffer.mBufferSize = clientRingBufferSize;
						mHeader->mServerRingBuffer.mBufferSize = serverRingBufferSize;
					}
					break;
				case CT_LAST: // Make compiler happy
					break;
			}
		}
		else
		{
			errorCode = EC_MAPFILE_VIEW;
		}
	}

	~FastIPC(void)
	{
		if ( mHeader )
		{
			if ( mIsServer ) // on clean exit, release the server buffers..
			{
				mHeader->mIsServer = false;		// kills the server.
			}
			else
			{
				mHeader->mIsClient = false;	// kills the client
			}
			if ( mMemoryMappedFile )
			{
				delete mMemoryMappedFile;
			}
		}
	}

	// Send a blob of binary data to the other process.
	virtual ErrorCode		 sendData(const void *data,PxU32 data_len,bool bufferIfFull)
	{
		ErrorCode ret = EC_OK;

   		PX_ASSERT(mWriter);
   		if ( mWriter )
   		{
			PxRingBuffer::ErrorCode rec = mWriter->sendData(PxRingBuffer::MT_APP,data,data_len,bufferIfFull);
			if ( rec != PxRingBuffer::EC_OK )
			{
				ret = EC_FAIL;
			}
    	}
    	else
    	{
    		ret = EC_FAIL;
    	}

		return ret;
	}

	virtual const void * receiveData(PxU32 &data_len)
	{
		const void *ret = NULL;
		data_len = 0;
   		PX_ASSERT(mReader);
   		if ( mReader )
   		{
			mReader->receiveData(ret,data_len);
    	}
		return ret;
	}

	virtual bool isServer(void) const
	{
		return mIsServer;
	}

	virtual	ErrorCode			receiveAcknowledge(void) // acknowledge that we have processed the incmoing message and can advance the read buffer.
	{
		ErrorCode ret = EC_OK;


   		PX_ASSERT(mReader);
   		if ( mReader )
   		{
			PxRingBuffer::ErrorCode rec = mReader->receiveAcknowledge();
			if ( rec != PxRingBuffer::EC_OK )
			{
				ret = EC_FAIL;
    		}
    	}
    	else
    	{
    		ret = EC_FAIL;
    	}


		return ret;
	}

	virtual bool			pumpPendingSends(void)
	{
		bool ret = false;
		PX_ASSERT(mWriter);
		if ( mWriter )
		{
			ret = mWriter->pumpPendingSends();
		}
		return ret;
	}

	virtual bool haveConnection(void) const
	{
		bool ret = false;
		if ( mHeader )
		{
			ret = mIsServer ? mHeader->mIsClient : mHeader->mIsServer;
		}
		return ret;
	}

	virtual bool canSend(PxU32 len) // return true if we can send a message of this size.
	{
		bool ret = false;
		if ( mWriter )
		{
			if ( mWriter->canSend(len) == PxRingBuffer::EC_OK )
			{
				ret = true;
			}
			else
			{
				bool pendingSend = mWriter->pumpPendingSends();
				if ( !pendingSend )
				{
					ret = true;
				}
			}
		}
		return ret;
	}

	virtual void release(void)
	{
		delete this;
	}

private:
	PxU32					mSendThreadID;			// The thread id we are using for sends
	PxU32					mReceiveThreadID;		// The thread id we are using for receives
	bool					mIsServer;				// True if we are a server
	FastIPCHeader			*mHeader;				// The header for ringbuffer/memory data transport.
	PxRingBuffer			mServerRingBuffer;		// The ring buffer for the server.
	PxRingBuffer			mClientRingBuffer;		// The ring buffer for the client
	PxRingBuffer			*mWriter;				// The ring buffer we use to write data
	PxRingBuffer			*mReader;				// The ring buffer we use to read data
	MemoryMappedFile *mMemoryMappedFile;	// If using memory mapped files
};



PxIPC * createFastIPC(PxIPC::ErrorCode &errorCode,
					PxIPC::ConnectionType connectionType,
						const char *mappingObject,
						PxU32 serverRingBufferSize,
						PxU32 clientRingBufferSize,
					 	bool allowLongMessages,
					 	bool bufferSends,
					 	PxU32 maxBufferSendSize)

{
	FastIPC *ret = PX_NEW(FastIPC)(errorCode,
							connectionType,
						 	mappingObject,
							serverRingBufferSize,
							clientRingBufferSize,
					 		allowLongMessages,
							bufferSends,
							maxBufferSendSize);

	if ( errorCode != PxIPC::EC_OK )
	{
		delete ret;
		ret = NULL;
	}
	return static_cast< PxIPC * >(ret);
}


} // end of namespace
} // end of namespace
