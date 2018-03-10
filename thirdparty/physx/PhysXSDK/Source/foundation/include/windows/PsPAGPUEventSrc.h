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


/* This file is obsolete!
 *
 * You should be including pxtask/CUDA/GPUProfile.h instead
 *
 */


#ifndef __PSPA_GPU_EVENT_SRC___
#define __PSPA_GPU_EVENT_SRC___

/*!! Changing any parameter in this file requires a full rebuild of all CUDA code !!*/

///< Enables per-warp logging to external memory
#define ENABLE_CTA_PROFILING					0

///< Defines the size of the profiling buffers allocated on both the GPU
///< and host pinned memory.  A single buffer is used to collect all
///< of the profile data for an entire push-buffer.  A warning will be
///< emitted by the GpuDispatcher if this limit is reached.
#define NUM_CTAS_PER_PROFILE_BUFFER             (4*1024*1024)

///< Total number of profile buffers that will be allocated by the Gpu
///< Dispatcher.  A good guess is the maximum number of addCompletionPrereq
///< calls you make in a single frame.  Having too few buffers is not harmful,
///< you will simply have unprofiled push-buffers (you will only see the
///< GpuTaskBatch bar).
#define NUM_CTA_PROFILE_BUFFERS					2

#include "Ps.h"
#include "cuda.h"

namespace physx
{
namespace shdfnd
{

#if defined(__CUDACC__)
	#define CUDA_ALIGN_16 __align__(16)
#else
	#define CUDA_ALIGN_16
#endif

typedef struct CUDA_ALIGN_16 kernelEvent_sd
{
	PxU16 block;
	PxU8  warp;
	PxU8  mpId;
	PxU8  hwWarpId;
	PxU8  userDataCfg;
	PxU16 eventId;
	PxU32 startTime;
	PxU32 endTime;
} kernelEvent_st;

#if ENABLE_CTA_PROFILING && __CUDA_ARCH__ > 100  // atomicAdd needs SM11

// extMem is the void* returned by GpuDispatcher::getCurrentProfileBuffer()
// kernelEnum is the value returned by GpuDispatcher::registerKernelNames() plus your local index

__device__
void fillKernelEvent( kernelEvent_sd& ev, PxU16 id, int threadID )
{
	int smidVal;
	int hwWarpId;
	int smclock;
	asm("mov.u32" " %0, %clock;" : "=r" (smclock));
	asm("mov.u32" " %0, %smid;" : "=r" (smidVal));
	asm("mov.u32" " %0, %warpid;" : "=r" (hwWarpId));
	ev.block		= (blockIdx.x + gridDim.x*blockIdx.y);
	ev.warp			= (threadID >> 5);
	ev.mpId			= smidVal;
	ev.eventId		= id;
	ev.hwWarpId		= hwWarpId;
	ev.startTime	= smclock;
	ev.endTime		= smclock;
	ev.userDataCfg	= 0;
}

#define KERNEL_START_EVENT(extMem, kernelEnum)															\
__shared__ volatile int _prof_offset_;																	\
if( extMem ) {																							\
	int threadID = threadIdx.x + blockDim.x*(threadIdx.y + blockDim.y*threadIdx.z);						\
	physx::shdfnd::kernelEvent_st ev;																	\
	physx::shdfnd::fillKernelEvent( ev, kernelEnum, threadID );											\
	if (threadID == 0)																					\
		_prof_offset_ = 1 + atomicAdd( extMem, ((blockDim.x * blockDim.y * blockDim.z) + 0x1f) >> 5 );	\
	__syncthreads();																					\
	if ((threadID & 0x1f) == 0) {																		\
		int index = _prof_offset_ + (threadID >> 5);													\
		if (index < NUM_CTAS_PER_PROFILE_BUFFER)														\
			((int4 *) extMem)[ index ] = *(int4*)&ev;													\
	}																									\
}

#define KERNEL_STOP_EVENT(extMem, kernelEnum)															\
if( extMem ) {																							\
	int threadID = threadIdx.x + blockDim.x*(threadIdx.y + blockDim.y*threadIdx.z);						\
	if ((threadID & 0x1f) == 0) {																		\
		int index = _prof_offset_ + (threadID >> 5);													\
		int regVal;																						\
		asm("mov.u32" " %0, %clock;" : "=r" (regVal));													\
		if (index < NUM_CTAS_PER_PROFILE_BUFFER)														\
			((int4 *)extMem)[ index ].w = regVal;														\
	}																									\
}

#else // !ENABLE_CTA_PROFILING

#define KERNEL_STOP_EVENT(extMem, kernelEnum)
#define KERNEL_START_EVENT(extMem, kernelEnum)

#endif

} // end namespace shdfnd
} // end namespace physx

#endif /** __PSPA_GPU_EVENT_SRC___ */
