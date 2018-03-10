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


#ifndef PXV_LINUX_LOAD_PHYSX_GPU_H
#define PXV_LINUX_LOAD_PHYSX_GPU_H

#include "PxPhysXGpu.h"
#include "PxCudaContextManager.h"
#include <dlfcn.h>

namespace physx
{

//sschirm: temporary code to load PhysXGpu dll.
//it's assumed that this is called after the right nvcuda.dll has been loaded.

	PxPhysXGpu* PxvLoadPhysXGpu()
	{
		PxPhysXGpu* physXGpu = NULL;
		void* physXGpuDll = PxLoadPhysxGPUModule();
		if( physXGpuDll )
		{
			typedef PxPhysXGpu* (createPhysXGpu_FUNC)();
			createPhysXGpu_FUNC* createPhysXGpu;
			*reinterpret_cast<void**>(&createPhysXGpu) = dlsym(physXGpuDll, "PxCreatePhysXGpu");
			if (createPhysXGpu)
				physXGpu = createPhysXGpu();
		}
		return physXGpu;
	}
}

#endif // PXV_LINUX_LOAD_PHYSX_GPU_H
