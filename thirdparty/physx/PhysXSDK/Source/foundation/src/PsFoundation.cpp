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


#include "PsFoundation.h"

#include "PxErrorCallback.h"
#include "PxQuat.h"
#include "PsThread.h"
#include "PsUtilities.h"
#include "PsTempAllocator.h"
#include "PsPAEventSrc.h"
#include "PsString.h"

#include <stdio.h>

#pragma warning(disable:4996) // intentionally suppressing this warning message

#if defined(PX_WINDOWS) && !defined(PX_VC9) && !defined(PX_VC10)
#	define vsnprintf _vsnprintf
#endif

namespace physx
{
namespace shdfnd
{


Foundation::Foundation(PxErrorCallback& errc, PxAllocatorCallback& alloc):
	mErrorCallback(errc),
	mAllocator(alloc),
#ifdef PX_CHECKED
	mReportAllocationNames(true),
#else
	mReportAllocationNames(false),
#endif
	mErrorMask(PxErrorCode::Enum(~0)),
	mErrorMutex(PX_DEBUG_EXP("Foundation::mErrorMutex")),
	mNamedAllocMutex(PX_DEBUG_EXP("Foundation::mNamedAllocMutex")),
	mTempAllocMutex(PX_DEBUG_EXP("Foundation::mTempAllocMutex"))
{
	PxI32 callbackIdx = mInteralErrorHandler.registerErrorCallback( mErrorCallback );
	PX_ASSERT(callbackIdx==0);
	PX_UNUSED(callbackIdx);
}

Foundation::~Foundation()
{
	// deallocate temp buffer allocations
	Allocator alloc;
	for(PxU32 i=0; i<mTempAllocFreeTable.size(); ++i)
	{
		for(TempAllocatorChunk* ptr = mTempAllocFreeTable[i]; ptr; )
		{
			TempAllocatorChunk* next = ptr->mNext;
			alloc.deallocate(ptr);
			ptr = next;
		}
	}
	mTempAllocFreeTable.reset();
}

Foundation&  Foundation::getInstance()
{ 
	PX_ASSERT(mInstance); 
	return *mInstance; 
}

PxU32 Foundation::getWarnOnceTimestamp()
{
	PX_ASSERT(mInstance != NULL);
	return mWarnOnceTimestap;
}

PxAllocatorCallback& Foundation::getAllocatorCallback() const
{
	return mAllocator.getBaseAllocator();
}

void Foundation::error(PxErrorCode::Enum c, const char* file, int line, const char* messageFmt, ...)
{
	va_list va;
	va_start(va, messageFmt);
	errorImpl(c, file, line, messageFmt, va);
	va_end(va);
}

void Foundation::errorImpl(PxErrorCode::Enum e, const char* file, int line, const char* messageFmt, va_list va)
{
	PX_ASSERT(messageFmt);
	if (e & mErrorMask)
	{
		//this function is reentrant but user's error callback may not be, so...
		Mutex::ScopedLock lock(mErrorMutex);	

		// using a static fixed size buffer here because:
		// 1. vsnprintf return values differ between platforms
		// 2. va_start is only usable in functions with ellipses
		// 3. ellipses (...) cannot be passed to called function
		// which would be necessary to dynamically grow the buffer here

		const int bufSize = 1024;
		static char stringBuffer[bufSize+1];

		vsnprintf(stringBuffer, (size_t)bufSize, messageFmt, va);
		stringBuffer[bufSize] = 0; // make sure there is a null termination character on all platforms

		mInteralErrorHandler.reportError(e, stringBuffer, file, line);
	}
}

Foundation* Foundation::createInstance(PxU32 version, PxErrorCallback& errc, PxAllocatorCallback& alloc)
{
	if (version != PX_PHYSICS_VERSION)
	{
		char* buffer = new char[256];
		physx::string::sprintf_s(buffer,256, "Wrong version: foundation version is 0x%08x, tried to create 0x%08x", PX_PHYSICS_VERSION, version);
		errc.reportError(PxErrorCode::eINVALID_PARAMETER, buffer, __FILE__, __LINE__);
		return 0;
	}

	if (!mInstance)
	{
		// if we don't assign this here, the Foundation object can't create member
		// subobjects which require the allocator

		mInstance = reinterpret_cast<Foundation*>( alloc.allocate( 
			sizeof(Foundation), "Foundation", __FILE__,__LINE__));

		if (mInstance)
		{
			PX_PLACEMENT_NEW(mInstance, Foundation)(errc, alloc);

			PX_ASSERT(mRefCount == 0);
			mRefCount = 1;

			//skip 0 which marks uninitialized timestaps in PX_WARN_ONCE
			mWarnOnceTimestap = (mWarnOnceTimestap == PX_MAX_U32) ? 1 : mWarnOnceTimestap + 1;

			return mInstance;
		}
		else
		{
			errc.reportError(PxErrorCode::eINTERNAL_ERROR, "Memory allocation for foundation object failed.", __FILE__, __LINE__);
		}
	}
	else
	{
		errc.reportError(PxErrorCode::eINVALID_OPERATION, "Foundation object exists already. Only one instance per process can be created.", __FILE__, __LINE__);
	}

	return 0;
}

void Foundation::destroyInstance()
{
	PX_ASSERT(mInstance != NULL);
	
	if (mRefCount == 1)
	{
		PxAllocatorCallback& alloc = mInstance->mAllocator.getBaseAllocator();
		mInstance->~Foundation();
		alloc.deallocate(mInstance);
		mInstance = 0;
		mRefCount = 0;
	}
	else
	{
		mInstance->error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Foundation destruction failed due to pending module references. Close/release all depending modules first.");
	}
}

void Foundation::incRefCount()
{
	PX_ASSERT(mInstance != NULL);

	if (mRefCount > 0)
	{
		mRefCount++;
	}
	else
	{
		mInstance->error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Foundation: Invalid registration detected.");
	}
}

void Foundation::decRefCount()
{
	PX_ASSERT(mInstance != NULL);

	if (mRefCount > 0)
	{
		mRefCount--;
	}
	else
	{
		mInstance->error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Foundation: Invalid deregistration detected.");
	}
}

void Foundation::release()
{
	Foundation::destroyInstance();
}

PxErrorCallback& Foundation::getErrorCallback() const
{
	return mErrorCallback;
}

void Foundation::setErrorLevel(PxErrorCode::Enum mask)
{
	mErrorMask = mask;
}

PxErrorCode::Enum Foundation::getErrorLevel() const
{
	return mErrorMask;
}

PxAllocatorCallback& getAllocator()
{ 
	return getFoundation().getCheckedAllocator();
}

void* Foundation::AlignCheckAllocator::allocate(size_t size, const char* typeName, const char* filename, int line)
{
	void* addr = mAllocator.allocate(size, typeName, filename, line);

	if (!addr)
		getFoundation().error(PxErrorCode::eABORT, __FILE__, __LINE__, "User allocator returned NULL.");


	if(!(reinterpret_cast<size_t>(addr)&15))
	{
		//Same comment as before in the allocation system.
		//We don't lock the listener array mutex because of an assumption
		//where the listener array is rarely changing.
		PxU32 theCount = mListenerCount;
		for( PxU32 idx = 0; idx < theCount; ++idx )
			mListeners[idx]->onAllocation( size, typeName, filename, line, addr );
		return addr;
	}

	getFoundation().error(PxErrorCode::eABORT, __FILE__, __LINE__, "Allocations for PhysX must be 16-byte aligned.");
	return 0;
}


Foundation* Foundation::mInstance = NULL;
PxU32 Foundation::mRefCount = 0;
PxU32 Foundation::mWarnOnceTimestap = 0;



} // namespace shdfnd
} // namespace physx


physx::PxFoundation* PxCreateFoundation(physx::PxU32 version, physx::PxAllocatorCallback& allocator, physx::PxErrorCallback& errorCallback)
{
	return physx::shdfnd::Foundation::createInstance(version, errorCallback, allocator);
}


physx::PxFoundation& PxGetFoundation()
{
	return physx::shdfnd::Foundation::getInstance();
}
