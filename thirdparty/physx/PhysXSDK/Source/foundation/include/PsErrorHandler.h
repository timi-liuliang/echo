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

#ifndef PX_FOUNDATION_PSERRORHANDLER_H
#define PX_FOUNDATION_PSERRORHANDLER_H

#include "foundation/PxErrorCallback.h"


namespace physx
{
namespace shdfnd
{

class PX_FOUNDATION_API ErrorHandler
{
public:
	static const PxU32 MAX_LISTENER_COUNT = 2;

	ErrorHandler();
	~ErrorHandler();

	/**
	\brief Reports an error code.
	\param code Error code, see #PxErrorCode
	\param message Message to display.
	\param file File error occured in.
	\param line Line number error occured on.
	*/
	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);

	/**
	\brief Register the PxErrorCallback.
	\param callback Callback to register.
	\return The index of callback, return -1 if callbacks are more than #MAX_LISTENER_COUNT.
	*/
	PxI32 registerErrorCallback( PxErrorCallback& callback );

	/**
	\brief Un-Register the PxErrorCallback
	\param callback Callback to un-register.
	*/
	void unRegisterErrorCallback( PxErrorCallback& callback );

	/**
	\brief Un-Register the PxErrorCallback
	\param index The index of callback to un-register.
	*/
	void unRegisterErrorCallback( PxI32 index );

	/**
	\brief return the max number of error callback listeners
	*/
	PX_FORCE_INLINE PxU32 getMaxCallbackNum() const { return MAX_LISTENER_COUNT; }

	/**
	\brief return the number of error callback listeners
	*/
	PX_FORCE_INLINE PxU32 getCallbackCount() const { return (PxU32)mListenerCount; }
	
	/**
	\brief Return the PxErrorCallback
	\param idx The callback index.
	*/
	PX_FORCE_INLINE PxErrorCallback* getErrorCallback( PxI32 idx ) const { return mErrorCallbacks[idx]; }

private:
	PxErrorCallback*	mErrorCallbacks[MAX_LISTENER_COUNT];
	PxI32				mListenerCount;
	PxU32				mCallbackBitmap;
	
};

} // namespace shdfnd
} // namespace physx


/** @} */
#endif
