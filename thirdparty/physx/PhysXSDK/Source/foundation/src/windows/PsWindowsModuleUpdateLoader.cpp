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

#ifdef SUPPORT_UPDATE_LOADER_LOGGING
#ifdef PX_X86
#define NX_USE_SDK_DLLS
#include "PhysXUpdateLoader.h"
#endif
#endif /* SUPPORT_UPDATE_LOADER_LOGGING */

#include "windows/PsWindowsModuleUpdateLoader.h"
#include "windows/PsWindowsLoadLibrary.h"


namespace physx { namespace shdfnd {

#ifdef PX_VC
#pragma warning(disable: 4191)	//'operator/operation' : unsafe conversion from 'type of expression' to 'type required'
#endif


typedef HMODULE (*GetUpdatedModule_FUNC)(const char*, const char*);

#ifdef SUPPORT_UPDATE_LOADER_LOGGING
#ifdef PX_X86
typedef void (*setLogging_FUNC)(PXUL_ErrorCode, pt2LogFunc);

static void LogMessage(PXUL_ErrorCode messageType, char* message)
{
	switch(messageType)
	{
	case PXUL_ERROR_MESSAGES:
		getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, 
			"PhysX Update Loader Error: %s.", message);
		break;
	case PXUL_WARNING_MESSAGES:
		getFoundation().error(PX_WARN, "PhysX Update Loader Warning: %s.", message);
		break;
	case PXUL_INFO_MESSAGES:
		getFoundation().error(PX_INFO, "PhysX Update Loader Information: %s.", message);
		break;
	default:
		getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__,
			"Unknown message type from update loader.");
		break;
	}
}
#endif
#endif /* SUPPORT_UPDATE_LOADER_LOGGING */

PsModuleUpdateLoader::PsModuleUpdateLoader(const char* updateLoaderDllName)
	: mGetUpdatedModuleFunc(NULL)
{
	mUpdateLoaderDllHandle = loadLibrary(updateLoaderDllName);

	if (mUpdateLoaderDllHandle != NULL)
	{
		mGetUpdatedModuleFunc = GetProcAddress(mUpdateLoaderDllHandle, "GetUpdatedModule");

#ifdef SUPPORT_UPDATE_LOADER_LOGGING
#ifdef PX_X86
		setLogging_FUNC setLoggingFunc;
		setLoggingFunc = (setLogging_FUNC)GetProcAddress(mUpdateLoaderDllHandle, "setLoggingFunction");
		if(setLoggingFunc != NULL)		
		{
           setLoggingFunc(PXUL_ERROR_MESSAGES, LogMessage);
        }
#endif
#endif /* SUPPORT_UPDATE_LOADER_LOGGING */
	}
}

PsModuleUpdateLoader::~PsModuleUpdateLoader()
{
	if (mUpdateLoaderDllHandle != NULL)
	{
		FreeLibrary(mUpdateLoaderDllHandle);
		mUpdateLoaderDllHandle = NULL;
	}
}

HMODULE PsModuleUpdateLoader::LoadModule(const char* moduleName, const char* appGUID)
{
	HMODULE result = NULL;

	if (mGetUpdatedModuleFunc != NULL)
	{
		// Try to get the module through PhysXUpdateLoader
		GetUpdatedModule_FUNC getUpdatedModuleFunc = (GetUpdatedModule_FUNC)mGetUpdatedModuleFunc;
		result = getUpdatedModuleFunc(moduleName, appGUID);
	}
	else
	{
		// If no PhysXUpdateLoader, just load the DLL directly
		result = loadLibrary(moduleName);
	}

	return result;
}

}; // end of namespace
}; // end of namespace
