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


#include "PsTempAllocator.h"

#include "PsArray.h"
#include "PsMutex.h"
#include "PsAtomic.h"
#include "foundation/PxMath.h"
#include "PsIntrinsics.h"
#include "PsFoundation.h"

#pragma warning( disable : 4706 ) // assignment within conditional expression

namespace physx
{
namespace shdfnd
{
	namespace 
	{
		typedef TempAllocatorChunk Chunk;
		typedef Array<Chunk*, Allocator> AllocFreeTable;

		PX_INLINE AllocFreeTable& getFreeTable() { return getFoundation().getTempAllocFreeTable(); }
		PX_INLINE Foundation::Mutex& getMutex() { return getFoundation().getTempAllocMutex();	}

		const PxU32 sMinIndex = 8; // 256B min
		const PxU32 sMaxIndex = 17; // 128kB max
	}


	void* TempAllocator::allocate(size_t size, const char* filename, int line)
	{
		if(!size)
			return 0;

		PxU32 index = PxMax(highestSetBit(PxU32(size) + sizeof(Chunk) - 1), sMinIndex);

		Chunk* chunk = 0;
		if(index < sMaxIndex)
		{
			Foundation::Mutex::ScopedLock lock(getMutex());

			// find chunk up to 16x bigger than necessary
			Chunk **it=getFreeTable().begin() + index - sMinIndex;
			Chunk **end=PxMin(it+3, getFreeTable().end());
			while(it<end && !(*it))
				++it;

			if(it<end)
				// pop top off freelist
				chunk=*it, *it = chunk->mNext, index = PxU32(it - getFreeTable().begin() + sMinIndex);
			else
				// create new chunk
				chunk = (Chunk*)Allocator::allocate(size_t(2 << index), filename, line);

		} else {
			// too big for temp allocation, forward to base allocator
			chunk = (Chunk*)Allocator::allocate(size + sizeof(Chunk), filename, line);
		}

		chunk->mIndex = index;
		void* ret = chunk + 1;
		PX_ASSERT((size_t(ret) & 0xf) == 0); //SDK types require at minimum 16 byte allignment.
		return ret;
	}

	void TempAllocator::deallocate(void* ptr)
	{
		if(!ptr)
			return;

		Chunk* chunk = reinterpret_cast<Chunk*>(ptr) - 1;
		PxU32 index = chunk->mIndex;

		if(index >= sMaxIndex)
			return Allocator::deallocate(chunk);

		Foundation::Mutex::ScopedLock lock(getMutex());

		index -= sMinIndex;
		if(getFreeTable().size() <= index)
			getFreeTable().resize(index+1);

		chunk->mNext = getFreeTable()[index];
		getFreeTable()[index] = chunk;
	}

} // namespace shdfnd
} // namespace physx
