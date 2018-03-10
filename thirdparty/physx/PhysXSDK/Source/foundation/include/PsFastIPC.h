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


#ifndef PS_FAST_IPC_H
#define PS_FAST_IPC_H

#include "PxSimpleTypes.h"
#include "PsIPC.h"

namespace physx
{
	namespace shdfnd
	{

PxIPC* createFastIPC(PxIPC::ErrorCode &errorCode,		// error code return if creation fails
					  PxIPC::ConnectionType connectionType=PxIPC::CT_CLIENT_OR_SERVER, // how to establish the connection
						const char* mappingObject="Global\\FastIPC",		// Name of communications channel
						PxU32 serverRingBufferSize=1024*4,				// buffer size for outgoing server messages.
						PxU32 clientRingBufferSize=1024*4, 			// buffer size for incoming client messages.
						bool allowLongMessages=true,
						bool	bufferSends=false,
						PxU32 maxBufferSendSize=1024*8);			// True if we want to buffer sends, if true, then pumpSendBuffers should be called by the send thread.

void	  releaseFastIPC(PxIPC* f);

} // end of namespace
} // end of namespace

#endif
