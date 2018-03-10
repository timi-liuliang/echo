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

#if PX_SUPPORT_GPU_PHYSX
#include <dlfcn.h>
#include "PxCudaContextManager.h"

#ifdef PX_PHYSX_DLL_NAME_POSTFIX
#define STRINGIFY(x) #x
#define GETSTRING(x) STRINGIFY(x)

#ifdef PX_X86
static const char*	gPhysXGpuLibraryName = "./libPhysX3Gpu" GETSTRING(PX_PHYSX_DLL_NAME_POSTFIX) "_x86.so";
#elif defined(PX_X64)
static const char*	gPhysXGpuLibraryName = "./libPhysX3Gpu" GETSTRING(PX_PHYSX_DLL_NAME_POSTFIX) "_x64.so";
#endif

#undef GETSTRING
#undef STRINGIFY
#else

#ifdef PX_X86
static const char*	gPhysXGpuLibraryName = "./libPhysX3Gpu_x86.so";
#elif defined(PX_X64)
static const char*	gPhysXGpuLibraryName = "./libPhysX3Gpu_x64.so";
#endif

#endif //PX_PHYSX_DLL_NAME_POSTFIX

namespace physx
{
	class PxProfileZoneManager;
	class PxFoundation;
	class PxErrorCallback;

	class PxCudaContextManager;
	class PxCudaContextManagerDesc;

	typedef int (getOrdinal_FUNC) ( PxErrorCallback& errc );
	typedef PxCudaContextManager* (createContext_FUNC) ( PxFoundation& foundation, const PxCudaContextManagerDesc& desc, physx::PxProfileZoneManager* mgr );
	typedef void (setFoundation_FUNC) ( PxFoundation& foundation );

	getOrdinal_FUNC*    g_getSuggestedCudaDeviceOrdinalFunc = NULL;
	createContext_FUNC* g_createCudaContextManagerFunc = NULL;
	setFoundation_FUNC*	g_setFoundationFunc = NULL;
	static void*		s_library;

	void* PxLoadPhysxGPUModule(const char*)
	{		
		if( s_library == NULL )
		{
			// load libcuda.so here since gcc configured with --as-needed won't link to it
			// if there is no call from the binary to it.
			void* hLibCuda = dlopen("libcuda.so", RTLD_NOW|RTLD_GLOBAL);
			if(hLibCuda)
				s_library = dlopen(gPhysXGpuLibraryName, RTLD_NOW);
		}

		// no UpdateLoader
		
		if( s_library )
		{
			*reinterpret_cast<void**>(&g_createCudaContextManagerFunc) = dlsym( s_library, "createCudaContextManagerDLL" );
			*reinterpret_cast<void**>(&g_getSuggestedCudaDeviceOrdinalFunc) = dlsym( s_library, "getSuggestedCudaDeviceOrdinalDLL" );
			*reinterpret_cast<void**>(&g_setFoundationFunc) = dlsym( s_library, "setFoundationInstance" );
		}

		return s_library;
	}

} // end physx namespace

#endif


