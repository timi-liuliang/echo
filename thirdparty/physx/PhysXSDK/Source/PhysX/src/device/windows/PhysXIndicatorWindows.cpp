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

#include "PhysXIndicator.h"
#include "nvPhysXtoDrv.h"

#pragma warning (push)
#pragma warning (disable : 4668) //'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#include <windows.h>
#pragma warning (pop)

#include <stdio.h>

#if _MSC_VER >= 1800
#include <VersionHelpers.h>
#endif

// Scope-based to indicate to NV driver that CPU PhysX is happening
physx::PhysXIndicator::PhysXIndicator(bool isGpu) 
: mPhysXDataPtr(0), mFileHandle(0), mIsGpu(isGpu)
{
    // Get the windows version (we can only create Global\\ namespace objects in XP)
	/**
		Operating system		Version number
		----------------		--------------
		Windows 7				6.1
		Windows Server 2008 R2	6.1
		Windows Server 2008		6.0
		Windows Vista			6.0
		Windows Server 2003 R2	5.2
		Windows Server 2003		5.2
		Windows XP				5.1
		Windows 2000			5.0
	**/
	
	char configName[128];

#if _MSC_VER >= 1800
	if (!IsWindowsVistaOrGreater())
#else
	OSVERSIONINFOEX windowsVersionInfo;
	windowsVersionInfo.dwOSVersionInfoSize = sizeof (windowsVersionInfo);
	GetVersionEx((LPOSVERSIONINFO)&windowsVersionInfo);
	
	if (windowsVersionInfo.dwMajorVersion < 6)
#endif
		NvPhysXToDrv_Build_SectionNameXP(GetCurrentProcessId(), configName);
	else
		NvPhysXToDrv_Build_SectionName(GetCurrentProcessId(), configName);
	
	mFileHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
		PAGE_READWRITE, 0, sizeof(NvPhysXToDrv_Data_V1), configName);

	if (!mFileHandle || mFileHandle == INVALID_HANDLE_VALUE)
		return;

	bool alreadyExists = ERROR_ALREADY_EXISTS == GetLastError();

	mPhysXDataPtr = (physx::NvPhysXToDrv_Data_V1*)MapViewOfFile(mFileHandle, 
		FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, sizeof(NvPhysXToDrv_Data_V1));

	if(!mPhysXDataPtr)
		return;

	if (!alreadyExists)
	{
		mPhysXDataPtr->bCpuPhysicsPresent = 0;
		mPhysXDataPtr->bGpuPhysicsPresent = 0;
	}

	updateCounter(1);

	// init header last to prevent race conditions
	// this must be done because the driver may have already created the shared memory block,
	// thus alreadyExists may be true, even if PhysX hasn't been initialized
	NvPhysXToDrv_Header_Init(mPhysXDataPtr->header);
}

physx::PhysXIndicator::~PhysXIndicator()
{
	if(mPhysXDataPtr)
	{
		updateCounter(-1);
		UnmapViewOfFile(mPhysXDataPtr);
	}

	if(mFileHandle && mFileHandle != INVALID_HANDLE_VALUE)
		CloseHandle(mFileHandle);
}

void physx::PhysXIndicator::setIsGpu(bool isGpu)
{
	if(!mPhysXDataPtr)
		return;

	updateCounter(-1);
	mIsGpu = isGpu;
	updateCounter(1);
}

PX_INLINE void physx::PhysXIndicator::updateCounter(int delta)
{
	(mIsGpu ? mPhysXDataPtr->bGpuPhysicsPresent 
		: mPhysXDataPtr->bCpuPhysicsPresent) += delta;
}
