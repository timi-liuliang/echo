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

#include "PxErrors.h"
#include "PxErrorCallback.h"
#include "PsErrorHandler.h"
#include "PsBitUtils.h"
#include "PsFoundation.h"
#include "PsMutex.h"

using namespace physx;
using namespace physx::shdfnd;

PX_COMPILE_TIME_ASSERT(ErrorHandler::MAX_LISTENER_COUNT <= 31);

	namespace {
		PX_INLINE Foundation::Mutex& getMutex() { return getFoundation().getErrorMutex(); }
	}

ErrorHandler::ErrorHandler()
: mListenerCount(0)
, mCallbackBitmap(0)
{
	for( PxU32 i = 0; i < MAX_LISTENER_COUNT; i++ )
	{
		mErrorCallbacks[i] = NULL;
	}
}

ErrorHandler::~ErrorHandler()
{
	Foundation::Mutex::ScopedLock lock(getMutex());

	for( PxU32 i = 0; i < MAX_LISTENER_COUNT; i++ )
	{
		mErrorCallbacks[i] = NULL;
	}

	mListenerCount = 0;
}


void ErrorHandler::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	Foundation::Mutex::ScopedLock lock(getMutex());

	for( PxU32 i = 0; i < MAX_LISTENER_COUNT; i++ )
	{
		if( mErrorCallbacks[i] != NULL )
			mErrorCallbacks[i]->reportError( code, message, file, line );
	}
}


PxI32 ErrorHandler::registerErrorCallback( PxErrorCallback& callback )
{
	Foundation::Mutex::ScopedLock lock(getMutex());

	//Only support at most 31 listener
	PxU32 reserveBitmap = (~mCallbackBitmap) & 0x7FFFFFFF;

	PX_ASSERT(reserveBitmap != 0);

	//Use bitmap to find the empty listener 
	PxU32 callbackIdx = lowestSetBit(reserveBitmap);

	if( callbackIdx < MAX_LISTENER_COUNT )
	{
		mErrorCallbacks[callbackIdx] = &callback;
		mCallbackBitmap |= 1<<callbackIdx;
		mListenerCount++;
		return (PxI32)callbackIdx;
	}

	return -1;
}


void ErrorHandler::unRegisterErrorCallback( PxErrorCallback& callback )
{
	for( PxU32 i = 0; i < MAX_LISTENER_COUNT; i++ )
	{
		if( mErrorCallbacks[i] == &callback )
			unRegisterErrorCallback( (PxI32)i );
	}
}

void ErrorHandler::unRegisterErrorCallback( PxI32 index )
{
	if( mErrorCallbacks[index] != NULL )
	{
		Foundation::Mutex::ScopedLock lock(getMutex());

		mErrorCallbacks[index] = NULL;
		mCallbackBitmap &= ~(1<<index);

		mListenerCount--;
	}
}
