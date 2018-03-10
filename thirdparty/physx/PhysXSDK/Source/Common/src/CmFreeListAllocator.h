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


#ifndef FREELIST_ALLOCATOR_H
#define FREELIST_ALLOCATOR_H

#include "PxAllocatorCallback.h"
#include "PsMutex.h"
#include "CmFreeListAllocatorInternals.h"

namespace physx
{
namespace Cm
{

	// memory is allocated from the chunk in 16-byte pieces. The last 4 bytes in the chunk
	// are:
	// two bytes offset of the current chunk in the block 
	// two bytes length of the next chunk (top bit is set if the block is allocated)
	//
	// The block wraps around in this regard, so the last two bytes are the length of
	// the first chunk in the block.
	//
	// for a free block, the first word is a pointer to the previous block of this size
	// in the free list, the second word is a pointer to the next block of this size
	//
	// on allocating a block, we find the best-fit chunk and split it if necessary
	// 
	// on freeing the block we aggregate it with the previous and next blocks


	class Chunk;
	class Block;

	// todo: port to new format of Ps::Allocator
	class FreeListAllocator : public PxAllocatorCallback
	{
		PX_NOCOPY(FreeListAllocator)
	public:
		static const PxU32 MAX_CHUNK_SIZE = Block::QWORD_CAPACITY * 16;
		static const PxU32 STATS_COUNT = Block::QWORD_CAPACITY;

		FreeListAllocator(PxAllocatorCallback& allocator);
		~FreeListAllocator();

		// return the number of allocation slots an allocation will require
		static PxU32		getQWSize(size_t size);

		void*				allocate(size_t size, const char* typeName, const char* file, int line);
		void				deallocate(void* ptr);

		PxAllocatorCallback&
							getBaseAllocator() const { return mBaseAllocator; }

		void				dumpState();

	private:
		PxAllocatorCallback&	mBaseAllocator;		// must be before mutex

		class MutexAllocator
		{
		public:
			MutexAllocator(PxAllocatorCallback &allocator): mAllocator(allocator)	{}
			void* allocate(size_t size, const char* filename, int line)	{	return mAllocator.allocate(size, "FreeListAllocatorMutex", filename, line); }
			void deallocate(void* ptr)									{	mAllocator.deallocate(ptr); }
		private:
			MutexAllocator& operator=(const MutexAllocator&);
			PxAllocatorCallback &mAllocator;
		};

		typedef Ps::MutexT<MutexAllocator> BaseMutex;

		BaseMutex			mMutex;
		FastBitMap			mFreeMap;
		Chunk *				mFreeList[Block::QWORD_CAPACITY];
		Block *				mBlockList;

		Chunk *			findBestFit(PxU32 requestSize, PxU32 &fitSize);

		void			pushOnFreeList(Chunk *chunk, Chunk::QWSize qwSize);
		Chunk *			popFromFreeList(Chunk::QWSize qwSize);
		void			unlinkFromFreeList(Chunk *chunk);

		void *			allocChunk(Chunk::QWSize qwSize);
		void			freeChunk(Chunk *chunk);

		void *			directAlloc(size_t size);
		void			directFree(void *mem);

		void			getStats(PxU32 *size, PxU32 *count);

		friend void dumpFreeListAllocatorState(class FreeListAllocator &a);

	};
	
	void dumpFreeListAllocatorState(FreeListAllocator &a);

	FreeListAllocator* createFreeListAllocator(PxAllocatorCallback&);
	void destroyFreeListAllocator(FreeListAllocator&);

} // namespace Cm

}

#endif
