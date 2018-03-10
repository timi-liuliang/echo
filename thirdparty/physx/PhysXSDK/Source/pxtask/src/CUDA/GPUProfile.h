/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef GPU_PROFILE_H
#define GPU_PROFILE_H

/*!! Changing any parameter in this file requires a full rebuild of all CUDA code !!*/

///< Enables per-warp logging to external memory
#ifdef PX_PROFILE
#define ENABLE_WARP_PROFILING					1
#else
#define ENABLE_WARP_PROFILING					0
#endif

///< Defines the size of the profiling buffers allocated on both the GPU
///< and host pinned memory.  A single buffer is used to collect all
///< of the profile data for an entire push-buffer.  A warning will be
///< emitted by the GpuDispatcher if this limit is reached.
#define NUM_WARPS_PER_PROFILE_BUFFER			(4*1024*1024)

#include "Ps.h"
#include "cuda.h"

namespace physx
{
#if defined(__CUDACC__)
#define CUDA_ALIGN_16 __align__(16)
#else
#define CUDA_ALIGN_16
#endif

typedef struct CUDA_ALIGN_16
{
	PxU16 block;
	PxU8  warp;
	PxU8  mpId;
	PxU8  hwWarpId;
	PxU8  userDataCfg;
	PxU16 eventId;
	PxU32 startTime;
	PxU32 endTime;
} warpProfileEvent;

#if ENABLE_WARP_PROFILING && defined(__CUDA_ARCH__) && __CUDA_ARCH__ > 100  // atomicAdd needs SM11

// extMem is the void* returned by GpuDispatcher::getCurrentProfileBuffer()
// kernelEnum is the value returned by GpuDispatcher::registerKernelNames() plus your local index

__device__
void fillKernelEvent(physx::warpProfileEvent& ev, PxU16 id, PxU32 threadID)
{
	PxU32 smidVal;
	PxU32 hwWarpId;
	PxU32 smclock;
	asm volatile("mov.u32 %0, %clock;" : "=r"(smclock));
	asm volatile("mov.u32 %0, %smid;" : "=r"(smidVal));
	asm volatile("mov.u32 %0, %warpid;" : "=r"(hwWarpId));
	ev.block		= (blockIdx.x + gridDim.x * blockIdx.y);
	ev.warp			= (threadID >> 5);
	ev.mpId			= smidVal;
	ev.eventId		= id;
	ev.hwWarpId		= hwWarpId;
	ev.startTime	= smclock;
	ev.endTime		= smclock;
	ev.userDataCfg	= 0;
}

#define KERNEL_START_EVENT_NO_SHARED_VARIABLE(extMem, kernelEnum)											\
	if( extMem ) {																							\
		physx::PxU32 threadID = threadIdx.x + blockDim.x*(threadIdx.y + blockDim.y*threadIdx.z);			\
		physx::warpProfileEvent ev;																	\
		physx::fillKernelEvent( ev, kernelEnum, threadID );											\
		if (threadID == 0)																					\
			_prof_offset_ = 1 + atomicAdd( extMem, ((blockDim.x * blockDim.y * blockDim.z) + 0x1f) >> 5 );	\
		__syncthreads();																					\
		if ((threadID & 0x1f) == 0) {																		\
			physx::PxU32 index = _prof_offset_ + (threadID >> 5);											\
			if (index < NUM_WARPS_PER_PROFILE_BUFFER)														\
				((int4 *) extMem)[ index ] = *(int4*)&ev;													\
		}																									\
	}

#define KERNEL_START_EVENT(extMem, kernelEnum)																\
	__shared__ volatile physx::PxU32 _prof_offset_;															\
	KERNEL_START_EVENT_NO_SHARED_VARIABLE(extMem, kernelEnum)

#define KERNEL_STOP_EVENT(extMem, kernelEnum)																\
	if( extMem ) {																							\
		physx::PxU32 threadID = threadIdx.x + blockDim.x*(threadIdx.y + blockDim.y*threadIdx.z);			\
		if ((threadID & 0x1f) == 0) {																		\
			physx::PxU32 index = _prof_offset_ + (threadID >> 5);											\
			physx::PxU32 regVal;																			\
			asm("mov.u32" " %0, %clock;" : "=r" (regVal));													\
			if (index < NUM_WARPS_PER_PROFILE_BUFFER)														\
				((int4 *)extMem)[ index ].w = regVal;														\
		}																									\
	}

#else // !ENABLE_WARP_PROFILING (or device portion of nvcc)

#define KERNEL_START_EVENT_NO_SHARED_VARIABLE(extMem, kernelEnum)
#define KERNEL_START_EVENT(extMem, kernelEnum)
#define KERNEL_STOP_EVENT(extMem, kernelEnum)

#endif

}

#endif /** GPU_PROFILE_H */
