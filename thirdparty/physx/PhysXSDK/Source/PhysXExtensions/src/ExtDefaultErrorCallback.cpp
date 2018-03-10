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

#include "PxDefaultErrorCallback.h"
#include "PxAssert.h"
#include "PsPrintString.h"
#include "PsThread.h"
#include <stdio.h>

using namespace physx;


PxDefaultErrorCallback::PxDefaultErrorCallback()
{
}

PxDefaultErrorCallback::~PxDefaultErrorCallback()
{
}

void PxDefaultErrorCallback::reportError(PxErrorCode::Enum e, const char* message, const char* file, int line)
{
	const char* errorCode = NULL;

	switch (e)
	{
	case PxErrorCode::eNO_ERROR:
		errorCode = "no error";
		break;
	case PxErrorCode::eINVALID_PARAMETER:
		errorCode = "invalid parameter";
		break;
	case PxErrorCode::eINVALID_OPERATION:
		errorCode = "invalid operation";
		break;
	case PxErrorCode::eOUT_OF_MEMORY:
		errorCode = "out of memory";
		break;
	case PxErrorCode::eDEBUG_INFO:
		errorCode = "info";
		break;
	case PxErrorCode::eDEBUG_WARNING:
		errorCode = "warning";
		break;
	case PxErrorCode::ePERF_WARNING:
		errorCode = "performance warning";
		break;
	case PxErrorCode::eABORT:
		errorCode = "abort";
		break;
	case PxErrorCode::eINTERNAL_ERROR:
		errorCode = "internal error";
		break;
	case PxErrorCode::eMASK_ALL:
	default:
		errorCode = "unknown error";
		break;
	}

	PX_ASSERT(errorCode);
	if(errorCode)
	{
		char buffer[1024];
		sprintf(buffer, "%s (%d) : %s : %s\n", file, line, errorCode, message);

		physx::shdfnd::printString(buffer);

		// in debug builds halt execution for abort codes
		PX_ASSERT(e != PxErrorCode::eABORT);

		// in release builds we also want to halt execution 
		// and make sure that the error message is flushed  
		while (e == PxErrorCode::eABORT)
		{
			physx::shdfnd::printString(buffer);
			physx::shdfnd::Thread::sleep(1000);
		}
	}	
}
