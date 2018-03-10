/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_CUDA_MODULE_SCENE_H
#define NX_CUDA_MODULE_SCENE_H

#include <NxApexDefs.h>

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief A scene encapsulation for CUDA scenes
*/
class NxCudaModuleScene
{
public:
	/**
	\brief Get the CuModule from the given function name
	*/
	virtual void* getCuModule(const char* functionName) = 0;

	/**
	\brief Get the CuFunction from the given function name
	*/
	virtual void* getCuFunction(const char* functionName) = 0;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_CUDA_MODULE_SCENE_H
