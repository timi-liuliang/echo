/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __REDUCE_CUH__
#define __REDUCE_CUH__

#include "common.cuh"
#include <float.h>

template <typename T>
struct AddOP
{
	static __device__ T apply(T a, T b) { return a + b; }
	static __device__ T identity() { return T(); }
};

typedef AddOP<unsigned int> AddOPu;
typedef AddOP<float> AddOPf;

struct MinOPf
{
	static __device__ float apply(float a, float b) { return fmin(a, b); }
	static __device__ float identity() { return +FLT_MAX; }
};

struct MaxOPf
{
	static __device__ float apply(float a, float b) { return fmax(a, b); }
	static __device__ float identity() { return -FLT_MAX; }
};

struct MaxOPu
{
	static __device__ float apply(float a, float b) { return umax(a, b); }
	static __device__ float identity() { return 0; }
};

struct AddAbsOPf
{
	static __device__ float apply(float a, float b) { return fabs(a) + fabs(b); }
	static __device__ float identity() { return 0.f; }
};

struct AddAbsOPi
{
	static __device__ int apply(int a, int b) { return abs(a) + abs(b); }
	static __device__ unsigned int identiy() { return 0.f; }
};

template <typename T, class OP>
__device__ void reduceWarp(volatile T* sdata)
{
	unsigned int idx = threadIdx.x;
	if ((idx & (WARP_SIZE-1)) < 16)
	{
		sdata[idx] = OP::apply(sdata[idx], sdata[idx + 16]); 
		sdata[idx] = OP::apply(sdata[idx], sdata[idx +  8]);
		sdata[idx] = OP::apply(sdata[idx], sdata[idx +  4]);
		sdata[idx] = OP::apply(sdata[idx], sdata[idx +  2]);
		sdata[idx] = OP::apply(sdata[idx], sdata[idx +  1]);
	}
}

template <typename T, class OP>
__device__ void reduceBlock(volatile T* sdata, T* g_odata)
{
	const unsigned int WarpsPerBlock = (blockDim.x >> LOG2_WARP_SIZE);
	const unsigned int idx = threadIdx.x;
	if (idx < WARP_SIZE) 
	{
		sdata[idx] = (idx < WarpsPerBlock) ? sdata[idx << LOG2_WARP_SIZE] : OP::identity();
		reduceWarp<T, OP>(sdata);
	}
	if (idx == 0) {
		g_odata[blockIdx.x] = sdata[0];
	}
}

template <typename T, class OP, unsigned int WarpsPerBlock>
__device__ void reduceSingleBlock(volatile T* sdata)
{
	unsigned int idx = threadIdx.x;
	if (idx < WARP_SIZE) 
	{
		sdata[idx] = (idx < WarpsPerBlock) ? sdata[idx << LOG2_WARP_SIZE] : OP::identity();
		reduceWarp<T, OP>(sdata);
	}
}

template <typename T>
struct ReduceTypeTraits;

template <> struct ReduceTypeTraits<unsigned int>
{
	typedef uint2 Type2;

	static __device__ uint2 makeType2(unsigned int v) { return make_uint2(v, v); }
};

template <> struct ReduceTypeTraits<float>
{
	typedef float2 Type2;

	static __device__ float2 makeType2(float v) { return make_float2(v, v); }
};



template <typename T, class OP>
__device__ void reduceGrid(volatile T* sdata, const T* g_data, unsigned int gridSize = gridDim.x)
{
	//gridSize can be > WARP_SIZE, so we use x2 reduce below to support gridSize up to 64!!!
#if MAX_BOUND_BLOCKS > 64
#error MAX_BOUND_BLOCKS > 64 is not supported
#endif
	typedef typename ReduceTypeTraits<T>::Type2 T2;

	unsigned int idx = threadIdx.x;
	if (idx < WARP_SIZE)
	{
#if 1
		T2 val2 = (idx << 1) < gridSize ? ((T2*)g_data)[idx] : ReduceTypeTraits<T>::makeType2(OP::identity());
		sdata[idx] = OP::apply(val2.x, (idx << 1) + 1 < gridSize ? val2.y : OP::identity());
#else
		sdata[idx] = (idx < gridSize) ? g_data[idx] : OP::identity();
		if (gridSize > WARP_SIZE)
		{
			sdata[idx] = OP::apply(sdata[idx], (idx + WARP_SIZE < gridSize) ? g_data[idx + WARP_SIZE] : OP::identity());
		}
#endif
		reduceWarp<T, OP>(sdata);
	}
}

#endif __REDUCE_CUH__
