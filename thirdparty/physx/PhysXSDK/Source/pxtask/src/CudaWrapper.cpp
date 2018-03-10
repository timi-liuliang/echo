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
#include "PxCudaContextManager.h"

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

	extern getOrdinal_FUNC*     g_getSuggestedCudaDeviceOrdinalFunc;
	extern createContext_FUNC*  g_createCudaContextManagerFunc;
	extern setFoundation_FUNC*	g_setFoundationFunc;

	int PxGetSuggestedCudaDeviceOrdinal(PxErrorCallback& errc)
	{
		if( !g_getSuggestedCudaDeviceOrdinalFunc )
			PxLoadPhysxGPUModule();

		if( g_getSuggestedCudaDeviceOrdinalFunc )
			return g_getSuggestedCudaDeviceOrdinalFunc(errc);
		else
			return -1;
	}

	PxCudaContextManager* PxCreateCudaContextManager(PxFoundation& foundation, const PxCudaContextManagerDesc& desc, physx::PxProfileZoneManager* mgr)
	{
		if( !g_createCudaContextManagerFunc )
			PxLoadPhysxGPUModule(desc.appGUID);

		if (g_setFoundationFunc)
			g_setFoundationFunc( foundation );

		if( g_createCudaContextManagerFunc )
			return g_createCudaContextManagerFunc( foundation, desc, mgr );
		else
			return NULL;
	}
} // end physx namespace

#endif

