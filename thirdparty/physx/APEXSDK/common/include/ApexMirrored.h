/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_MIRRORED_H
#define APEX_MIRRORED_H

#include "NxApexDefs.h"

#include "NxApex.h"
#include "ApexCutil.h"
#include "NiApexScene.h"

#include "PxTaskManager.h"
#include "PxGpuDispatcher.h"
#include "PxGpuCopyDesc.h"
#include "PxGpuCopyDescQueue.h"
#include "PxCudaContextManager.h"
#include "PxCudaMemoryManager.h"
//#include <cuda.h>

#if defined(__CUDACC__)
#error "Mirrored arrays should not be visible to CUDA code.  Send device pointers to CUDA kernels."
#endif

namespace physx
{
namespace apex
{

struct ApexMirroredPlace
{
	enum Enum
	{
		DEFAULT = 0,
		CPU     = 0x01,
#if defined(APEX_CUDA_SUPPORT)
		GPU     = 0x02,
		CPU_GPU = (CPU | GPU),
#endif
	};
};

template <class T>
class ApexMirrored
{
	PX_NOCOPY(ApexMirrored);

public:
	ApexMirrored(NiApexScene& scene, NV_ALLOC_INFO_PARAMS_DECL(NULL, 0, NULL, UNASSIGNED))
		: mCpuPtr(0)
		, mByteCount(0)
		, mPlace(ApexMirroredPlace::CPU)
		, mAllocInfo(NV_ALLOC_INFO_PARAMS_INPUT())
#if defined(APEX_CUDA_SUPPORT)
		, mCpuBuffer(NULL)
		, mGpuPtr(0)
		, mGpuBuffer(NULL)
#endif
	{
		PX_UNUSED(scene);
#if defined(APEX_CUDA_SUPPORT)
		physx::PxGpuDispatcher* gd = scene.getTaskManager()->getGpuDispatcher();
		if (gd)
		{
			mCtx = gd->getCudaContextManager();
		}
		else
		{
			mCtx = NULL;
			return;
		}
#endif
	};

	~ApexMirrored()
	{
	}

	//Operators for accessing the data pointed to on the host. Using these operators is guaranteed
	//to maintain the class invariants.  Note that these operators are only ever called on the host.
	//The GPU never sees this class as instances are converted to regular pointers upon kernel
	//invocation.

	PX_INLINE T& operator*()
	{
		return *getCpuPtr();
	}

	PX_INLINE const T& operator*() const
	{
		return *getCpuPtr();
	}

	PX_INLINE T* operator->()
	{
		return getCpuPtr();
	}

	PX_INLINE const T* operator->() const
	{
		return getCpuPtr();
	}

	PX_INLINE T& operator[](unsigned int i)
	{
		return getCpuPtr()[i];
	}

	//Methods for converting the pointer to a regular pointer for use on
	//the CPU After a pointer has been obtained with these methods, the
	//data can be accessed multiple times with no extra cost. This is the
	//fastest method for accessing the data on the cpu.

	PX_INLINE T* getCpuPtr() const
	{
		return mCpuPtr;
	}

	/*!
	\return
	returns whether CPU buffer has been allocated for this array
	*/
	PX_INLINE bool cpuPtrIsValid() const
	{
		return mCpuPtr != 0;
	}

	PX_INLINE size_t* getCpuHandle() const
	{
		return reinterpret_cast<size_t*>(&mCpuPtr);
	}

	PX_INLINE size_t getByteSize() const
	{
		return mByteCount;
	}

#if defined(APEX_CUDA_SUPPORT)
	/*!
	\return
	returns whether GPU buffer has been allocated for this array
	*/
	PX_INLINE bool gpuPtrIsValid() const
	{
		return mGpuPtr != 0;
	}

	PX_INLINE T* getGpuPtr() const
	{
		return mGpuPtr;
	}

	/*!
	Get opaque handle to the underlying gpu or cpu memory These must not
	be cast to a pointer or derefernced, they should only be used to
	identify the memory region to the allocator
	*/
	PX_INLINE size_t* getGpuHandle() const
	{
		return reinterpret_cast<size_t*>(&mGpuPtr);
	}

	PX_INLINE void copyDeviceToHostDesc(PxGpuCopyDesc& desc, size_t byteSize, size_t byteOffset) const
	{
		PX_ASSERT(mCpuPtr && mGpuPtr && mByteCount);
		desc.type = PxGpuCopyDesc::DeviceToHost;
		desc.bytes = byteSize;
		desc.source = ((size_t) mGpuPtr) + byteOffset;
		desc.dest = ((size_t) mCpuPtr) + byteOffset;
	}

	PX_INLINE void copyHostToDeviceDesc(PxGpuCopyDesc& desc, size_t byteSize, size_t byteOffset) const
	{
		PX_ASSERT(mCpuPtr && mGpuPtr && mByteCount);
		desc.type = PxGpuCopyDesc::HostToDevice;
		desc.bytes = byteSize;
		desc.source = ((size_t) mCpuPtr) + byteOffset;
		desc.dest = ((size_t) mGpuPtr) + byteOffset;
	}

	PX_INLINE void mallocGpu(size_t byteSize)
	{
		physx::PxCudaBufferType bufferType(physx::PxCudaBufferMemorySpace::T_GPU, physx::PxCudaBufferFlags::F_READ_WRITE);
		physx::PxCudaBuffer* buffer = mCtx->getMemoryManager()->alloc(bufferType, (physx::PxU32)byteSize);
		if (buffer)
		{
			// in case of realloc
			if (mGpuBuffer)
			{
				mGpuBuffer->free();
			}
			mGpuBuffer = buffer;
			mGpuPtr = reinterpret_cast<T*>(mGpuBuffer->getPtr());
			PX_ASSERT(mGpuPtr);
		}
		else
		{
			PX_ASSERT(!"Out of GPU Memory!");
		}
	}

	PX_INLINE void freeGpu()
	{
		if (mGpuBuffer)
		{
			bool success = mGpuBuffer->free();
			mGpuBuffer = NULL;
			mGpuPtr = NULL;
			PX_UNUSED(success);
			PX_ASSERT(success);
		}
	}

	PX_INLINE void mallocHost(size_t byteSize)
	{
		physx::PxCudaBufferType bufferType(physx::PxCudaBufferMemorySpace::T_PINNED_HOST, physx::PxCudaBufferFlags::F_READ_WRITE);
		physx::PxCudaBuffer* buffer = mCtx->getMemoryManager()->alloc(bufferType, (physx::PxU32)byteSize);
		if (buffer)
		{
			// in case of realloc
			if (mCpuBuffer)
			{
				mCpuBuffer->free();
			}
			mCpuBuffer = buffer;
			mCpuPtr = reinterpret_cast<T*>(mCpuBuffer->getPtr());
			PX_ASSERT(mCpuPtr);
		}
		else
		{
			PX_ASSERT(!"Out of Pinned Host Memory!");
		}
	}
	PX_INLINE void freeHost()
	{
		if (mCpuBuffer)
		{
			bool success = mCpuBuffer->free();
			mCpuBuffer = NULL;
			mCpuPtr = NULL;
			PX_UNUSED(success);
			PX_ASSERT(success);
		}
	}
	PX_INLINE void swapGpuPtr(ApexMirrored<T>& other)
	{
		physx::swap(mGpuPtr, other.mGpuPtr);
		physx::swap(mGpuBuffer, other.mGpuBuffer);
	}

#endif

	PX_INLINE void mallocCpu(size_t byteSize)
	{
		mCpuPtr = (T*)getAllocator().allocate(byteSize, mAllocInfo.getAllocName(), mAllocInfo.getFileName(), mAllocInfo.getLine());
		PX_ASSERT(mCpuPtr && "Out of CPU Memory!");
	}
	PX_INLINE void freeCpu()
	{
		if (mCpuPtr)
		{
			getAllocator().deallocate(mCpuPtr);
			mCpuPtr = NULL;
		}
	}


	PX_INLINE const char* getName() const
	{
		return  mAllocInfo.getAllocName();
	}

	PX_INLINE const physx::PxAllocInfo&	getAllocInfo() const
	{
		return  mAllocInfo;
	}

	void realloc(size_t byteCount, ApexMirroredPlace::Enum place)
	{
		ApexMirroredPlace::Enum oldPlace = mPlace;
		ApexMirroredPlace::Enum newPlace = (place != ApexMirroredPlace::DEFAULT) ? place : oldPlace;
		if (oldPlace == newPlace && byteCount <= mByteCount)
		{
			return;
		}

		size_t newSize = physx::PxMax(byteCount, mByteCount);

#if defined(APEX_CUDA_SUPPORT)
		if (oldPlace != ApexMirroredPlace::CPU && newPlace != ApexMirroredPlace::CPU)
		{
			PX_ASSERT(oldPlace != ApexMirroredPlace::CPU);
			PX_ASSERT(newPlace != ApexMirroredPlace::CPU);

			if ((mCpuPtr != NULL && byteCount > mByteCount) ||
			        (mCpuPtr == NULL && (place & ApexMirroredPlace::CPU) != 0))
			{
				physx::PxCudaBuffer* oldCpuBuffer = mCpuBuffer;
				T* oldCpuPtr = mCpuPtr;

				mCpuBuffer = NULL;

				mallocHost(newSize);

				physx::PxCudaBuffer* newCpuBuffer = mCpuBuffer;
				T* newCpuPtr = mCpuPtr;


				if (oldCpuPtr != NULL && newCpuPtr != NULL && mByteCount > 0)
				{
					memcpy(mCpuPtr, oldCpuPtr, mByteCount);
				}

				mCpuBuffer = oldCpuBuffer;
				mCpuPtr = newCpuPtr;

				freeHost();

				mCpuBuffer = newCpuBuffer;
				mCpuPtr = newCpuPtr;
			}
			if ((mGpuPtr != NULL && byteCount > mByteCount) ||
			        (mGpuPtr == NULL && (place & ApexMirroredPlace::GPU) != 0))
			{
				// we explicitly do not move old data to the new buffer

				freeGpu();
				mallocGpu(newSize);
			}
		}
		else
#endif
		{
			T* oldCpuPtr = mCpuPtr;
#if defined(APEX_CUDA_SUPPORT)
			if (newPlace != ApexMirroredPlace::CPU)
			{
				if (newPlace == ApexMirroredPlace::CPU_GPU)
				{
					mallocHost(newSize);
				}
				else
				{
					mCpuPtr = NULL;
				}
				mallocGpu(newSize);
			}
			else
#endif
			{
				mallocCpu(newSize);
			}
			T* newCpuPtr = mCpuPtr;

			if (oldCpuPtr != NULL && newCpuPtr != NULL && mByteCount > 0)
			{
				memcpy(newCpuPtr, oldCpuPtr, mByteCount);
			}

			mCpuPtr = oldCpuPtr;
#if defined(APEX_CUDA_SUPPORT)
			if (oldPlace != ApexMirroredPlace::CPU)
			{
				if (oldPlace == ApexMirroredPlace::CPU_GPU)
				{
					freeHost();
				}
				freeGpu();
			}
			else
#endif
			{
				freeCpu();
			}
			mCpuPtr = newCpuPtr;
		}
		mByteCount = newSize;
		mPlace = newPlace;
	}

	void free()
	{
		PX_ASSERT(mPlace != ApexMirroredPlace::DEFAULT);
#if defined(APEX_CUDA_SUPPORT)
		if (mPlace != ApexMirroredPlace::CPU)
		{
			freeHost();
			freeGpu();
		}
		else
#endif
		{
			freeCpu();
		}
		mByteCount = 0;
	}

private:
	mutable T*                  mCpuPtr;
	size_t	                    mByteCount;

	ApexMirroredPlace::Enum     mPlace;
	physx::PxAllocInfo    mAllocInfo;

#if defined(APEX_CUDA_SUPPORT)
	mutable physx::PxCudaBuffer*   mCpuBuffer;
	mutable T*                             mGpuPtr;
	mutable physx::PxCudaBuffer*   mGpuBuffer;
	physx::PxCudaContextManager*     mCtx;
#endif
};


}
} // end namespace physx::apex

#endif
