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

#pragma once

#include "Allocator.h"
#include "CuCheckSuccess.h"
#include <cuda.h>

#include "PxCudaContextManager.h"
#include "PxCudaMemoryManager.h"

namespace physx
{

namespace cloth
{

struct CuHostAllocator 
{
	CuHostAllocator(physx::PxCudaContextManager* ctx = NULL, unsigned int flags = cudaHostAllocDefault) 
	: mDevicePtr(0), mFlags(flags), mManager(0)
	{
		PX_ASSERT(ctx);

		if (ctx)
			mManager = ctx->getMemoryManager();
	}

	void* allocate(size_t n, const char*, int)
	{
		physx::PxCudaBufferPtr bufferPtr;

		PX_ASSERT(mManager);

		if (mFlags & cudaHostAllocWriteCombined)
			bufferPtr = mManager->alloc(PxCudaBufferMemorySpace::T_WRITE_COMBINED, n, NV_ALLOC_INFO("cloth::CuHostAllocator::T_WRITE_COMBINED", CLOTH));
		else if (mFlags & cudaHostAllocMapped)
			bufferPtr = mManager->alloc(PxCudaBufferMemorySpace::T_PINNED_HOST, n, NV_ALLOC_INFO("cloth::CuHostAllocator::T_PINNED_HOST", CLOTH));
		else
			bufferPtr = mManager->alloc(PxCudaBufferMemorySpace::T_HOST, n, NV_ALLOC_INFO("cloth::CuHostAllocator::T_HOST", CLOTH));

		if (mFlags & cudaHostAllocMapped)
			checkSuccess( cuMemHostGetDevicePointer(&mDevicePtr, reinterpret_cast<void*>(bufferPtr), 0) );

		return reinterpret_cast<void*>(bufferPtr);
	}

	void deallocate(void* p)
	{
		PX_ASSERT(mManager);

		if (mFlags & cudaHostAllocWriteCombined)
			mManager->free(PxCudaBufferMemorySpace::T_WRITE_COMBINED, physx::PxCudaBufferPtr(p));
		else if (mFlags & cudaHostAllocMapped)
			mManager->free(PxCudaBufferMemorySpace::T_PINNED_HOST, physx::PxCudaBufferPtr(p));
		else
			mManager->free(PxCudaBufferMemorySpace::T_HOST, physx::PxCudaBufferPtr(p));

		// don't reset mDevicePtr because Array::recreate deallocates last
	}

	CUdeviceptr mDevicePtr; // device pointer of last allocation
	unsigned int mFlags;
	physx::PxCudaMemoryManager* mManager;
};


template <typename T>
CuHostAllocator getMappedAllocator(physx::PxCudaContextManager* ctx)
{
	return CuHostAllocator(ctx, cudaHostAllocMapped | cudaHostAllocWriteCombined);
}

template <typename T>
struct CuPinnedVector
{
	// note: always use shdfnd::swap() instead of Array::swap()
	// in order to keep cached device pointer consistent
	typedef shdfnd::Array<T, typename physx::cloth::CuHostAllocator> Type;
};

template <typename T>
T* getDevicePointer(shdfnd::Array<T, typename physx::cloth::CuHostAllocator>& vector)
{
	// cached device pointer only valid if non-empty
	return vector.empty() ? 0 : reinterpret_cast<T*>(vector.getAllocator().mDevicePtr);
}

} // namespace cloth

namespace shdfnd
{
	template <typename T>
	void swap(Array<T, typename cloth::CuHostAllocator>& left,
		Array<T, typename cloth::CuHostAllocator>& right)
	{
		shdfnd::swap(left.getAllocator(), right.getAllocator());
		left.swap(right);
	}
}

} // namespace physx

