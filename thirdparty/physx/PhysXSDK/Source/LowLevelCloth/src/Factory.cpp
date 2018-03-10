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

#include "SwFactory.h"

#ifdef PX_PHYSX_GPU_EXPORTS
#include "CuFactory.h"
#endif

namespace physx
{
	namespace cloth
	{
		uint32_t getNextFabricId()
		{
			static uint32_t sNextFabricId = 0;
			return sNextFabricId++;
		}
	}
}

using namespace physx;

cloth::Factory* cloth::Factory::createFactory(Platform platform, void* contextManager)
{
	PX_UNUSED(contextManager);

	if(platform == Factory::CPU)
		return new SwFactory;

#ifdef PX_PHYSX_GPU_EXPORTS 
	if(platform == Factory::CUDA)
		return new CuFactory((PxCudaContextManager*)contextManager);
#endif

	return 0;
}
