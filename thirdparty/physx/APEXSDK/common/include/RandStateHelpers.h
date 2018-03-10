/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RAND_STATE_HELPERS_H
#define RAND_STATE_HELPERS_H

#include "PxTask.h"
#include "ApexMirroredArray.h"

namespace physx
{
namespace apex
{

// For CUDA PRNG: host part
PX_INLINE void InitDevicePRNGs(
    NiApexScene& scene,
    unsigned int blockSize,
    LCG_PRNG& threadLeap,
    LCG_PRNG& gridLeap,
    ApexMirroredArray<LCG_PRNG>& blockPRNGs)
{
	threadLeap = LCG_PRNG::getDefault().leapFrog(16);

	LCG_PRNG randBlock = LCG_PRNG::getIdentity();
	LCG_PRNG randBlockLeap = threadLeap.leapFrog(blockSize);

	const PxU32 numBlocks = 32; //Max Multiprocessor count
	blockPRNGs.setSize(numBlocks, ApexMirroredPlace::CPU_GPU);
	for (PxU32 i = 0; i < numBlocks; ++i)
	{
		blockPRNGs[i] = randBlock;
		randBlock *= randBlockLeap;
	}
	gridLeap = randBlock;

	{
		physx::PxTaskManager* tm = scene.getTaskManager();
		physx::PxCudaContextManager* ctx = tm->getGpuDispatcher()->getCudaContextManager();

		physx::PxScopedCudaLock s(*ctx);

		PxGpuCopyDesc desc;
		blockPRNGs.copyHostToDeviceDesc(desc, 0, 0);
		tm->getGpuDispatcher()->launchCopyKernel(&desc, 1, 0);
	}
}

}
} // physx::apex::

#endif
