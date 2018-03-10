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


#ifndef PS_WINDOWS_MODULEUPDATELOADER_H
#define PS_WINDOWS_MODULEUPDATELOADER_H

#include "foundation/PxPreprocessor.h"

#include "windows/PsWindowsInclude.h"

namespace physx
{
namespace shdfnd
{

#ifdef PX_X64
#define UPDATE_LOADER_DLL_NAME "PhysXUpdateLoader64.dll"
#else
#define UPDATE_LOADER_DLL_NAME "PhysXUpdateLoader.dll"
#endif

class PX_FOUNDATION_API PsModuleUpdateLoader
{
public:
	PsModuleUpdateLoader(const char* updateLoaderDllName);

	~PsModuleUpdateLoader();

	// Loads the given module through the update loader. Loads it from the path if 
	// the update loader doesn't find the requested module. Returns NULL if no
	// module found.
	HMODULE LoadModule(const char* moduleName, const char* appGUID);

protected:
	HMODULE mUpdateLoaderDllHandle;
	FARPROC mGetUpdatedModuleFunc;
};
} // namespace shdfnd
} // namespace physx


#endif	// PS_WINDOWS_MODULEUPDATELOADER_H
