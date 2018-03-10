/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "Px.h"
#include "PsFoundation.h"
#include "PxPhysics.h"

#if defined(PX_WINDOWS) && PX_SUPPORT_GPU_PHYSX
#include "windows/PsWindowsInclude.h"
#include "windows/PsWindowsModuleUpdateLoader.h"
#include "PxCudaContextManager.h"
#endif // PX_WINDOWS

#ifdef PX_PHYSX_DLL_NAME_POSTFIX
#define STRINGIFY(x) #x
#define GETSTRING(x) STRINGIFY(x)
#ifdef PX_WINDOWS
#ifdef PX_X86
static const char*	gPhysXGpuLibraryName = "PhysX3Gpu" GETSTRING(PX_PHYSX_DLL_NAME_POSTFIX) "_x86.dll";
#elif defined(PX_X64)
static const char*	gPhysXGpuLibraryName = "PhysX3Gpu" GETSTRING(PX_PHYSX_DLL_NAME_POSTFIX) "_x64.dll";
#endif
#endif  // PX_WINDOWS
#undef GETSTRING
#undef STRINGIFY
#else
#ifdef PX_WINDOWS
#ifdef PX_X86
static const char*	gPhysXGpuLibraryName = "PhysX3Gpu_x86.dll";
#elif defined(PX_X64)
static const char*	gPhysXGpuLibraryName = "PhysX3Gpu_x64.dll";
#endif
#endif  // PX_WINDOWS
#endif

namespace physx
{
#ifdef PX_VC
#pragma warning(disable: 4191)	//'operator/operation' : unsafe conversion from 'type of expression' to 'type required'
#endif

	class PxProfileZoneManager;
	class PxFoundation;
	class PxErrorCallback;

	class PxCudaContextManager;
	class PxCudaContextManagerDesc;

#if PX_SUPPORT_GPU_PHYSX
	#define DEFAULT_PHYSX_GPU_GUID    "2D61CA5B-494A-45c7-85B7-4E898F07037E"

	typedef int (getOrdinal_FUNC) ( PxErrorCallback& errc );
	typedef PxCudaContextManager* (createContext_FUNC) ( PxFoundation& foundation, const PxCudaContextManagerDesc& desc, physx::PxProfileZoneManager* mgr );
	typedef void (setFoundation_FUNC) ( PxFoundation& foundation );

	getOrdinal_FUNC*     g_getSuggestedCudaDeviceOrdinalFunc = NULL;
	createContext_FUNC*  g_createCudaContextManagerFunc = NULL;
	setFoundation_FUNC*	 g_setFoundationFunc = NULL;
	static HMODULE       s_library;

	void* PxLoadPhysxGPUModule(const char* appGUID)
	{	
		if( s_library == NULL )
		 s_library = GetModuleHandle(gPhysXGpuLibraryName);
		
		 if(s_library == NULL)
		 {
			shdfnd::PsModuleUpdateLoader moduleLoader(UPDATE_LOADER_DLL_NAME);
			s_library = moduleLoader.LoadModule(gPhysXGpuLibraryName, appGUID == NULL ? DEFAULT_PHYSX_GPU_GUID : appGUID );
		 }

		if( s_library )
		{
			g_createCudaContextManagerFunc = (createContext_FUNC*) GetProcAddress( s_library, "createCudaContextManagerDLL" );
			g_getSuggestedCudaDeviceOrdinalFunc = (getOrdinal_FUNC*) GetProcAddress( s_library, "getSuggestedCudaDeviceOrdinalDLL" );
			g_setFoundationFunc = (setFoundation_FUNC*) GetProcAddress( s_library, "setFoundationInstance" );
		}

		return s_library;
	}

#endif
} // end physx namespace

