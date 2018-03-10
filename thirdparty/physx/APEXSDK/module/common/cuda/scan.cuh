/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __SCAN_CUH__
#define __SCAN_CUH__

#include "blocksync.cuh"
#include "reduce.cuh"

template <typename T, typename OP>
__device__ void scanWarp(unsigned int scanIdx, volatile T* sdata)
{
	sdata[scanIdx] = OP::apply(sdata[scanIdx], sdata[scanIdx -  1]); 
	sdata[scanIdx] = OP::apply(sdata[scanIdx], sdata[scanIdx -  2]); 
	sdata[scanIdx] = OP::apply(sdata[scanIdx], sdata[scanIdx -  4]); 
	sdata[scanIdx] = OP::apply(sdata[scanIdx], sdata[scanIdx -  8]); 
	sdata[scanIdx] = OP::apply(sdata[scanIdx], sdata[scanIdx - 16]); 
}

template <typename T, typename OP>
__device__ T scanBlock(volatile T* sdata, T val)
{
	const unsigned int idx = threadIdx.x;
	const unsigned int idxInWarp = idx & (WARP_SIZE-1);
	const unsigned int warpIdx = (idx >> LOG2_WARP_SIZE);

	//setup scan
	int scanIdx = (warpIdx << (LOG2_WARP_SIZE + 1)) + idxInWarp;
	sdata[scanIdx] = OP::identity();
	scanIdx += WARP_SIZE;
	sdata[scanIdx] = val;

	scanWarp<T, OP>(scanIdx, sdata);

	__syncthreads();

	val = sdata[scanIdx - 1]; //-1 for exclusive
	T val2 = sdata[scanIdx]; 

	__syncthreads();

	if (idxInWarp == WARP_SIZE-1)
	{
		sdata[warpIdx + WARP_SIZE] = val2;
	}
	__syncthreads();

	if (idx < WARP_SIZE)
	{
		scanWarp<T, OP>(scanIdx, sdata);
	}
	__syncthreads();

	return OP::apply(val, sdata[warpIdx + WARP_SIZE - 1]); //-1 for exclusive
}

#define SCAN_LOOP(whole) \
	T val = OP::identity(); \
	if (whole || pos < warpEnd) val = g_input[pos]; \
	sdata[scanIdx] = val; \
	scanWarp<T, OP>(scanIdx, sdata); \
	if (whole || pos < warpEnd) g_output[pos] = OP::apply(sdata[scanIdx-1], prefix); \
	if (whole) prefix += sdata[(warpIdx << (LOG2_WARP_SIZE + 1)) + WARP_SIZE + WARP_SIZE-1];

// g_temp should have at least (WarpsPerBlock * gridDim.x) elements!!!
template<typename T, typename OP, unsigned int WarpsPerBlock>
__device__ void scan(unsigned int count, T const* g_input, T* g_output, T* g_temp)
{
	const unsigned int WarpsPerGrid = WarpsPerBlock * gridDim.x;

	const unsigned int DataWarpsPerGrid = ((count + WARP_SIZE-1) >> LOG2_WARP_SIZE);
	const unsigned int DataWarpsPerBlock = (DataWarpsPerGrid + gridDim.x-1) / gridDim.x;
	const unsigned int DataCountPerBlock = (DataWarpsPerBlock << LOG2_WARP_SIZE);

	const unsigned int WarpLimit = min(DataWarpsPerBlock, WarpsPerBlock);
	const unsigned int WarpBorder = DataWarpsPerBlock % WarpsPerBlock;
	const unsigned int WarpFactor = DataWarpsPerBlock / WarpsPerBlock;

	const unsigned int idx = threadIdx.x;

	const unsigned int warpIdx = (idx >> LOG2_WARP_SIZE);
	const unsigned int blockBeg = blockIdx.x * DataCountPerBlock;
	const unsigned int blockEnd = min(blockBeg + DataCountPerBlock, count);

	const unsigned int WarpSelect = (warpIdx < WarpBorder) ? 1 : 0;
	const unsigned int WarpCount = WarpFactor + WarpSelect;
	const unsigned int WarpOffset = warpIdx * WarpCount + WarpBorder * (1 - WarpSelect);

	const unsigned int warpBeg = blockBeg + (WarpOffset << LOG2_WARP_SIZE);
	const unsigned int warpEnd = min(warpBeg + (WarpCount << LOG2_WARP_SIZE), blockEnd);

	const unsigned int idxInWarp = idx & (WARP_SIZE-1);

	const unsigned int BlockSize = (WarpsPerBlock << LOG2_WARP_SIZE);
	__shared__ volatile unsigned int sdata[BlockSize * 2];

	if (warpIdx < WarpLimit)
	{
		//accum
		sdata[idx] = OP::identity();
		for (unsigned int pos = warpBeg + idxInWarp; pos < warpEnd; pos += WARP_SIZE)
		{
			sdata[idx] = OP::apply(sdata[idx], g_input[pos]);
		}
		//reduce warp
		reduceWarp<T, OP>(idx, sdata);
	}

	__syncthreads();

	if (idx < WarpsPerBlock)
	{
		g_temp[blockIdx.x * WarpsPerBlock + idx] = (idx < WarpLimit) ? sdata[idx << LOG2_WARP_SIZE] : OP::identity();
		__threadfence();
	}

	BLOCK_SYNC_BEGIN()

		T val = (idx < WarpsPerGrid) ? g_temp[idx] : OP::identity();

		val = scanBlock<T, OP>(sdata, val);

		if (idx < WarpsPerGrid) {
			g_temp[idx] = val;
			__threadfence();
		}

	BLOCK_SYNC_END()

	__shared__ unsigned int stemp[WarpsPerBlock];

	if (idx < WarpsPerBlock)
	{
		stemp[idx] = g_temp[blockIdx.x * WarpsPerBlock + idx];
	}
	__syncthreads();

	if (warpBeg < warpEnd)
	{
		T prefix = stemp[warpIdx];

		//setup scan
		int scanIdx = (warpIdx << (LOG2_WARP_SIZE + 1)) + idxInWarp;
		sdata[scanIdx] = OP::identity();
		scanIdx += WARP_SIZE;

		unsigned int pos;
		for (pos = warpBeg + idxInWarp; pos < (warpEnd & ~(WARP_SIZE-1)); pos += WARP_SIZE)
		{
			SCAN_LOOP(true)
		}
		if ((warpEnd & (WARP_SIZE-1)) > 0)
		{
			SCAN_LOOP(false)
		}
	}
}

#endif __SCAN_CUH__
