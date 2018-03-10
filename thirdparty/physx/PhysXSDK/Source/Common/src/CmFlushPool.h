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


#ifndef PX_PHYSICS_COMMON_FLUSHPOOL
#define PX_PHYSICS_COMMON_FLUSHPOOL

#include "Px.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PsMutex.h"
#include "PsArray.h"

/*
Pool used to allocate variable sized tasks. It's intended to be cleared after a short period (time step).
*/

namespace physx
{
namespace Cm
{
	static const PxU32 sSpareChunkCount = 2;

	class FlushPool
	{
		PX_NOCOPY(FlushPool)
	public:
		FlushPool(PxU32 chunkSize) : mChunks(PX_DEBUG_EXP("FlushPoolChunk")), mChunkIndex(0), mOffset(0), mChunkSize(chunkSize)
		{
			mChunks.pushBack(static_cast<PxU8*>(PX_ALLOC(mChunkSize, PX_DEBUG_EXP("PxU8"))));
		}

		~FlushPool()
		{
			for (PxU32 i = 0; i < mChunks.size(); ++i)
				PX_FREE(mChunks[i]);
		}

		// alignment must be a power of two
		void* allocate(PxU32 size, PxU32 alignment=16)
		{
			Ps::Mutex::ScopedLock lock(mMutex);
			return allocateNotThreadSafe(size, alignment);
		}

		// alignment must be a power of two
		void* allocateNotThreadSafe(PxU32 size, PxU32 alignment=16)
		{
			PX_ASSERT(shdfnd::isPowerOfTwo(alignment));
			PX_ASSERT(size <= mChunkSize && !mChunks.empty());
			
			// padding for alignment
			size_t unalignedStart = reinterpret_cast<size_t>(mChunks[mChunkIndex]+mOffset);
			PxU32 pad = PxU32(((unalignedStart+alignment-1)&~(size_t(alignment)-1)) - unalignedStart);

			if (mOffset + size + pad > mChunkSize)
			{
				mChunkIndex++;
				mOffset = 0;
				if (mChunkIndex >= mChunks.size())
					mChunks.pushBack(static_cast<PxU8*>(PX_ALLOC(mChunkSize, PX_DEBUG_EXP("PxU8"))));

				// update padding to ensure new alloc is aligned
				unalignedStart = reinterpret_cast<size_t>(mChunks[mChunkIndex]);
				pad = PxU32(((unalignedStart+alignment-1)&~(size_t(alignment)-1)) - unalignedStart);
			}

			void* ptr = mChunks[mChunkIndex] + mOffset + pad;
			PX_ASSERT((reinterpret_cast<size_t>(ptr)&(size_t(alignment)-1)) == 0);
			mOffset += size + pad;
			return ptr;
		}

		void clear(PxU32 spareChunkCount = sSpareChunkCount)
		{
			Ps::Mutex::ScopedLock lock(mMutex);
			
			clearNotThreadSafe(spareChunkCount);
		}

		void clearNotThreadSafe(PxU32 spareChunkCount = sSpareChunkCount)
		{
			PX_UNUSED(spareChunkCount);

			//release memory not used previously
			PxU32 targetSize = mChunkIndex+sSpareChunkCount;
			while (mChunks.size() > targetSize)
				PX_FREE(mChunks.popBack());

			mChunkIndex = 0;
			mOffset = 0;
		}

		void resetNotThreadSafe()
		{
			PxU8* firstChunk = mChunks[0];

			for (PxU32 i = 1; i < mChunks.size(); ++i)
				PX_FREE(mChunks[i]);

			mChunks.clear();
			mChunks.pushBack(firstChunk);
			mChunkIndex = 0;
			mOffset = 0;
		}

		void lock()
		{
			mMutex.lock();
		}

		void unlock()
		{
			mMutex.unlock();	
		}

	private:
		Ps::Mutex mMutex;
		Ps::Array<PxU8*> mChunks;
		PxU32 mChunkIndex;
		PxU32 mOffset;
		PxU32 mChunkSize;
	};

	
} // namespace Cm

}

#endif
