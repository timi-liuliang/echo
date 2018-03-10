/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __COMMON_CUH__
#define __COMMON_CUH__

#ifndef APEX_CUDA_MODULE_PREFIX
#error You need to define Cuda Module Prefix in "common.h" file and include it before "common.cuh"
#endif


#include <PsShare.h>
#include <GPUProfile.h>
#include <ApexCuda.h>
#include <InplaceTypes.h>


template <typename T>
inline __device__ void _forceParamRef(T var) { ; }

#define FREE_KERNEL_BEG(kernelName, ...) \
extern "C" __global__ void APEX_CUDA_NAME(kernelName)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCount, __VA_ARGS__ ) \
{ \
	const unsigned int BlockSize = blockDim.x; \
	const unsigned int WarpsPerBlock = (BlockSize >> LOG2_WARP_SIZE); \
	_forceParamRef(BlockSize); \
	_forceParamRef(WarpsPerBlock); \
	KERNEL_START_EVENT(_extMem, _kernelEnum) \

#define FREE_KERNEL_END() \
	KERNEL_STOP_EVENT(_extMem, _kernelEnum) \
} \

#define FREE_KERNEL_2D_BEG(kernelName, ...) \
extern "C" __global__ void APEX_CUDA_NAME(kernelName)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCountX, physx::PxU32 _threadCountY, __VA_ARGS__ ) \
{ \
	KERNEL_START_EVENT(_extMem, _kernelEnum) \

#define FREE_KERNEL_2D_END() \
	KERNEL_STOP_EVENT(_extMem, _kernelEnum) \
} \


#define FREE_KERNEL_3D_BEG(kernelName, ...) \
extern "C" __global__ void APEX_CUDA_NAME(kernelName)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCountX, physx::PxU32 _threadCountY, physx::PxU32 _threadCountZ, physx::PxU32 _blockCountY, __VA_ARGS__ ) \
{ \
	const unsigned int blockIdxZ = blockIdx.y / _blockCountY; \
	const unsigned int blockIdxY = blockIdx.y % _blockCountY; \
	const unsigned int blockIdxX = blockIdx.x; \
	const unsigned int idxX = blockIdxX * blockDim.x + threadIdx.x; \
	const unsigned int idxY = blockIdxY * blockDim.y + threadIdx.y; \
	const unsigned int idxZ = blockIdxZ * blockDim.z + threadIdx.z; \
	KERNEL_START_EVENT(_extMem, _kernelEnum) \

#define FREE_KERNEL_3D_END() \
	KERNEL_STOP_EVENT(_extMem, _kernelEnum) \
} \

#define FREE_S2_KERNEL_3D_BEG(kernelName, argseq) \
INPLACE_TEMPL_ARGS_DEF \
__device__ void APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCountX, physx::PxU32 _threadCountY, physx::PxU32 _threadCountZ, physx::PxU32 _blockCountY, __APEX_CUDA_FUNC_ARGS(argseq)); \
extern "C" __global__ void APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ$0)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCountX, physx::PxU32 _threadCountY, physx::PxU32 _threadCountZ, physx::PxU32 _blockCountY, __APEX_CUDA_FUNC_ARGS(argseq)) \
{ \
	APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ)<false>(_extMem, _kernelEnum, _threadCountX, _threadCountY, _threadCountZ, _blockCountY, __APEX_CUDA_FUNC_ARG_NAMES(argseq)); \
} \
extern "C" __global__ void APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ$1)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCountX, physx::PxU32 _threadCountY, physx::PxU32 _threadCountZ, physx::PxU32 _blockCountY, __APEX_CUDA_FUNC_ARGS(argseq)) \
{ \
	APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ)<true>(_extMem, _kernelEnum, _threadCountX, _threadCountY, _threadCountZ, _blockCountY, __APEX_CUDA_FUNC_ARG_NAMES(argseq)); \
} \
INPLACE_TEMPL_ARGS_DEF \
__device__ void APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCountX, physx::PxU32 _threadCountY, physx::PxU32 _threadCountZ, physx::PxU32 _blockCountY, __APEX_CUDA_FUNC_ARGS(argseq)) \
{ \
	const unsigned int blockIdxZ = blockIdx.y / _blockCountY; \
	const unsigned int blockIdxY = blockIdx.y % _blockCountY; \
	const unsigned int blockIdxX = blockIdx.x; \
	const unsigned int idxX = blockIdxX * blockDim.x + threadIdx.x; \
	const unsigned int idxY = blockIdxY * blockDim.y + threadIdx.y; \
	const unsigned int idxZ = blockIdxZ * blockDim.z + threadIdx.z; \
	KERNEL_START_EVENT(_extMem, _kernelEnum) \

#define FREE_S2_KERNEL_3D_END() \
	KERNEL_STOP_EVENT(_extMem, _kernelEnum) \
} \


#define BOUND_KERNEL_BEG(kernelName, ...) \
extern "C" __global__ void APEX_CUDA_NAME(kernelName)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCount, __VA_ARGS__ ) \
{ \
	const unsigned int BlockSize = blockDim.x; \
	const unsigned int WarpsPerBlock = (BlockSize >> LOG2_WARP_SIZE); \
	_forceParamRef(BlockSize); \
	_forceParamRef(WarpsPerBlock); \
	KERNEL_START_EVENT(_extMem, _kernelEnum) \

#define BOUND_KERNEL_END() \
	KERNEL_STOP_EVENT(_extMem, _kernelEnum) \
} \

#define BOUND_S2_KERNEL_BEG(kernelName, argseq) \
INPLACE_TEMPL_ARGS_DEF \
__device__ void APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCount, __APEX_CUDA_FUNC_ARGS(argseq)); \
extern "C" __global__ void APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ$0)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCount, __APEX_CUDA_FUNC_ARGS(argseq)) \
{ \
	APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ)<false>(_extMem, _kernelEnum, _threadCount, __APEX_CUDA_FUNC_ARG_NAMES(argseq)); \
} \
extern "C" __global__ void APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ$1)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCount, __APEX_CUDA_FUNC_ARGS(argseq)) \
{ \
	APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ)<true>(_extMem, _kernelEnum, _threadCount, __APEX_CUDA_FUNC_ARG_NAMES(argseq)); \
} \
INPLACE_TEMPL_ARGS_DEF \
__device__ void APEX_CUDA_CONCAT(APEX_CUDA_NAME(kernelName), _templ)(int* _extMem, physx::PxU16 _kernelEnum, physx::PxU32 _threadCount, __APEX_CUDA_FUNC_ARGS(argseq)) \
{ \
	const unsigned int BlockSize = blockDim.x; \
	const unsigned int WarpsPerBlock = (BlockSize >> LOG2_WARP_SIZE); \
	_forceParamRef(BlockSize); \
	_forceParamRef(WarpsPerBlock); \
	KERNEL_START_EVENT(_extMem, _kernelEnum) \

#define BOUND_S2_KERNEL_END() \
	KERNEL_STOP_EVENT(_extMem, _kernelEnum) \
} \


#define SYNC_KERNEL_BEG(kernelName, ...) \
extern "C" __global__ void APEX_CUDA_NAME(kernelName)(int* _extMem, physx::PxU16 _kernelEnum, __VA_ARGS__ ) \
{ \
	const unsigned int BlockSize = blockDim.x; \
	const unsigned int WarpsPerBlock = (BlockSize >> LOG2_WARP_SIZE); \
	_forceParamRef(BlockSize); \
	_forceParamRef(WarpsPerBlock); \
	KERNEL_START_EVENT(_extMem, _kernelEnum) \

#define SYNC_KERNEL_END() \
	KERNEL_STOP_EVENT(_extMem, _kernelEnum) \
} \


#define KERNEL_TEX_REF(name) APEX_CUDA_TEX_REF_NAME(name)
#define KERNEL_SURF_REF(name) APEX_CUDA_SURF_REF_NAME(name)

#ifdef APEX_CUDA_STORAGE_NAME

#define KERNEL_CONST_STORAGE InplaceHandleBase::StorageSelector INPLACE_TEMPL_ARGS_VAL (), APEX_CUDA_NAME(APEX_CUDA_CONCAT(APEX_CUDA_STORAGE_NAME, _ConstMem)), APEX_CUDA_NAME(APEX_CUDA_CONCAT(APEX_CUDA_STORAGE_NAME, _Texture))

#endif //APEX_CUDA_STORAGE_NAME

//const unsigned int LOG2_WARP_SIZE = 5;
//const unsigned int WARP_SIZE = (1U << LOG2_WARP_SIZE);

#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 200
const unsigned int LOG2_NUM_BANKS = 5;
#else
const unsigned int LOG2_NUM_BANKS = 4;
#endif

const unsigned int NUM_BANKS = (1U << LOG2_NUM_BANKS);

const unsigned int MULTIPROCESSOR_MAX_COUNT = 32;



#ifdef __CUDA_ARCH__
#if __CUDA_ARCH__ <= 300
#define GET_WARPS_PER_BLOCK(name) APEX_CUDA_CONCAT(APEX_CUDA_CONCAT(name, _), __CUDA_ARCH__)
#else
#define GET_WARPS_PER_BLOCK(name) APEX_CUDA_CONCAT(APEX_CUDA_CONCAT(name, _), 300) 
#endif
#else
#define GET_WARPS_PER_BLOCK(name) name ## _ ## 110
#endif


#endif //__COMMON_CUH__
