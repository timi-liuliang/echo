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


#include "windows/PsWindowsInclude.h"
#include "PsUserAllocated.h"
#include "PsSync.h"

namespace physx
{
namespace shdfnd
{

	namespace 
	{
		HANDLE& getSync(SyncImpl* impl)
		{
			return *reinterpret_cast<HANDLE*>(impl);
		}
	}

	static const PxU32 gSize = sizeof(HANDLE);
	const PxU32& SyncImpl::getSize()  { return gSize; }

SyncImpl::SyncImpl()
{
#ifdef PX_WINMODERN
	getSync(this) = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
#else
	getSync(this) = CreateEvent(0,true,false,0);
#endif
}

SyncImpl::~SyncImpl()
{
	CloseHandle(getSync(this));
}

void SyncImpl::reset()
{
	ResetEvent(getSync(this));
}

void SyncImpl::set()
{
	SetEvent(getSync(this));
}

bool SyncImpl::wait(PxU32 milliseconds)
{
	if(milliseconds==-1)
		milliseconds = INFINITE;
#ifdef PX_WINMODERN
	return WaitForSingleObjectEx(getSync(this), milliseconds, false) == WAIT_OBJECT_0 ? true : false;
#else
	return WaitForSingleObject(getSync(this), milliseconds) == WAIT_OBJECT_0 ? true : false;
#endif
}

} // namespace shdfnd
} // namespace physx
