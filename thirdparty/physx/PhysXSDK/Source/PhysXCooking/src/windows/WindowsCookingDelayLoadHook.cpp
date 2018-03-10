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


#include "windows/PxWindowsDelayLoadHook.h"
#include "windows/PsWindowsInclude.h"
#include "windows/PsWindowsLoadLibrary.h"
#include <delayimp.h>

static const physx::PxDelayLoadHook* gDelayLoadHook = NULL;

void physx::PxSetPhysXCookingDelayLoadHook(const physx::PxDelayLoadHook* hook)
{
	gDelayLoadHook = hook;
}


using namespace physx;

#pragma comment(lib, "delayimp")


FARPROC WINAPI delayHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	switch (dliNotify) {
	case dliStartProcessing :
		break;

	case dliNotePreLoadLibrary :
		{
			return shdfnd::physXCommonDliNotePreLoadLibrary(pdli->szDll,gDelayLoadHook);
		}
		break;

	case dliNotePreGetProcAddress :
		break;

	case dliFailLoadLib :
		break;

	case dliFailGetProc :
		break;

	case dliNoteEndProcessing :
		break;

	default :

		return NULL;
	}

	return NULL;
}

PfnDliHook __pfnDliNotifyHook2 = delayHook;
