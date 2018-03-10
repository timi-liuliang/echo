/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BLOCK_SYNC_CUH__
#define __BLOCK_SYNC_CUH__

#include "common.cuh"

__device__ volatile unsigned int blockSyncFlags[MULTIPROCESSOR_MAX_COUNT];

#define BLOCK_SYNC_BEGIN() \
	__shared__ volatile unsigned int s_blockSyncFlags[WARP_SIZE]; \
	if (blockIdx.x == 0) \
	{ \
		if (threadIdx.x < WARP_SIZE) { \
			do	{ \
				s_blockSyncFlags[threadIdx.x] = (threadIdx.x < MULTIPROCESSOR_MAX_COUNT) ? \
					blockSyncFlags[threadIdx.x] : 0; \
				if (threadIdx.x < 16) { \
					s_blockSyncFlags[threadIdx.x] += s_blockSyncFlags[threadIdx.x + 16]; \
					s_blockSyncFlags[threadIdx.x] += s_blockSyncFlags[threadIdx.x +  8]; \
					s_blockSyncFlags[threadIdx.x] += s_blockSyncFlags[threadIdx.x +  4]; \
					s_blockSyncFlags[threadIdx.x] += s_blockSyncFlags[threadIdx.x +  2]; \
					s_blockSyncFlags[threadIdx.x] += s_blockSyncFlags[threadIdx.x +  1]; \
				} \
			} \
			while (s_blockSyncFlags[0] + 1 < gridDim.x); \
		} \
		__syncthreads();

#define BLOCK_SYNC_END() \
		__syncthreads(); \
		if (threadIdx.x < MULTIPROCESSOR_MAX_COUNT) { \
			blockSyncFlags[threadIdx.x] = 0; \
			__threadfence(); \
		} \
		__syncthreads(); \
	} \
	else \
	{ \
		__syncthreads(); \
		if (threadIdx.x == blockIdx.x) { \
			blockSyncFlags[threadIdx.x] = 1; \
			__threadfence(); \
			while (blockSyncFlags[threadIdx.x] != 0) ; \
		} \
		__syncthreads(); \
	}

#endif //__BLOCK_SYNC_CUH__
