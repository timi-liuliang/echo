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

#include "foundation/PxPreprocessor.h"
#include <assert.h>
#include <stdio.h>
#include "CmFreeListAllocator.h"

using namespace physx;

/* Free list allocator. 
 * 
 * The allocator allocates an 8K block at a time and suballocates
 * memory which it returns aligned on 16-byte boundaries for 32-bit
 * arch's. 64-bit should be 32-byte boundaries, but it's untested.
 * 
 * the structure of a block is as follows:
 * block header
 *   a pointer to the previous chunk
 *   a pointer to the next chunk
 *
 * A sequence of chunks. Each chunk is located at alignment MIN_CHUNK_SIZE*n-4
 *   the first four bytes of each chunk are 
 *     16 bits: 
 *        0-8 for the length of this chunk in 16-byte sections
 *        9-17 for the length of the immediately preceding chunk in 16 byte sections
 *        29 if this is the first chunk
 *        30 if this is the last chunk
 *        31 if this chunk is in use
 *     
 *
 * in addition, the first two words of an unused chunk are pointers to the previous
 * and next chunks of this size in the list. This means that the minimum allocation
 * size on 32 bit architectures is 12 bytes and on 64 bit architectures, 24 bytes
 */

Cm::FreeListAllocator::FreeListAllocator(PxAllocatorCallback& allocator)
:	mBaseAllocator(allocator)
, 	mMutex(MutexAllocator(mBaseAllocator))
,	mBlockList(0)
{
	for(PxU32 i=0;i<Block::QWORD_CAPACITY;i++) 
		mFreeList[i] = 0;
}

Cm::FreeListAllocator::~FreeListAllocator()
{
	// just dump all the blocks for now

	while(mBlockList)
	{
		Block *b = mBlockList;
		b->finalize(mBlockList);
		mBaseAllocator.deallocate(b);
	}
}

PxU32 Cm::FreeListAllocator::getQWSize(size_t size)
{
	return Chunk::getQWSizeFromBytes(size);
}

Cm::Chunk * Cm::FreeListAllocator::findBestFit(Chunk::QWSize qwSize, Chunk::QWSize &bestQWSize)
{
	assert(qwSize<=Block::QWORD_CAPACITY);

	bestQWSize = mFreeMap.findFirst(qwSize);
	return bestQWSize == FastBitMap::NONE ? 0 : popFromFreeList(bestQWSize);
}

void * Cm::FreeListAllocator::directAlloc(size_t size)
{
	void *m = mBaseAllocator.allocate(size+16, 0, __FILE__, __LINE__);
	size_t addr = reinterpret_cast<size_t>(m);
	assert(!(addr & 15));								// 8 byte aligned, so
	PxU32 pad = 16;				// pad to 16-byte align with space at the front

	PxU32 *userMem = reinterpret_cast<PxU32 *>(addr + pad);
	userMem[-1] = pad;
	userMem[-2] = (PxU32)size;					// bit 31 is unset so we know it's not a freelist alloc

	return userMem;
}

void Cm::FreeListAllocator::directFree(void *mem)
{
	PxU32 *m = reinterpret_cast<PxU32 *>(mem);

    //warning #68-D on WIIU:  integer conversion resulted in a change of sign
	assert(!((int)m[-1]&(1<<31)));					// not marked as a used chunk
	mBaseAllocator.deallocate(addAddr(mem,-(int)m[-1]));
}

void Cm::FreeListAllocator::pushOnFreeList(Chunk *chunk, Chunk::QWSize qwSize)
{
	assert(chunk->getQWSize() == qwSize);
	chunk->link(mFreeList[qwSize]);
	mFreeMap.set(qwSize);
}


Cm::Chunk* Cm::FreeListAllocator::popFromFreeList(Chunk::QWSize qwSize)
{
	Chunk *chunk = mFreeList[qwSize];
	assert(chunk);
	chunk->unlink(mFreeList[qwSize]);
	assert(chunk->getQWSize() == qwSize);
	if(mFreeList[qwSize]==0)
		mFreeMap.reset(qwSize);
	return chunk;
}


void Cm::FreeListAllocator::unlinkFromFreeList(Chunk *chunk)
{
	Chunk::QWSize qwSize = chunk->getQWSize();
	chunk->unlink(mFreeList[qwSize]);
	if(!mFreeList[qwSize])
		mFreeMap.reset(qwSize);
}



void* Cm::FreeListAllocator::allocChunk(Chunk::QWSize requiredQWSize)
{
	Chunk::QWSize foundQWSize;
	Chunk *found = findBestFit(requiredQWSize,foundQWSize);

	if(!found)
	{
		Block *b = reinterpret_cast<Block *>(mBaseAllocator.allocate(Block::ALLOCATION_SIZE, 0, __FILE__, __LINE__));
		if(!b)
			return 0;

		b->init(mBlockList);
		found = b->firstChunk();
		foundQWSize = Block::QWORD_CAPACITY;
	}

	assert(found->getQWSize()==foundQWSize);

	if(foundQWSize>requiredQWSize)
		pushOnFreeList(found->split(requiredQWSize), foundQWSize - requiredQWSize);

	found->acquire();
	return found->memAddr();
}


void Cm::FreeListAllocator::freeChunk(Chunk *chunk)
{
	chunk->release();
	Chunk *prev = chunk->getPrevInBlock(), *next = chunk->getNextInBlock();

	if(prev && !prev->isUsed())
	{
		unlinkFromFreeList(prev);
		prev->merge(*chunk);
		chunk = prev;
	}

	if(next && !next->isUsed())
	{
		unlinkFromFreeList(next);
		chunk->merge(*next);
	}

	Chunk::QWSize qwSize = chunk->getQWSize();
	if(qwSize == Block::QWORD_CAPACITY)
	{
		assert(chunk->getPrevInBlock()==0);
		Block *block = Block::fromFirstChunk(chunk);

		block->finalize(mBlockList);
		mBaseAllocator.deallocate(block);
	}
	else
		pushOnFreeList(chunk,qwSize);
}

void* Cm::FreeListAllocator::allocate(size_t size, const char* /*typeName*/, const char* /*file*/, int /*line*/)
{
	if(!size)
		return 0;

	Chunk::QWSize qwSize = Chunk::getQWSizeFromBytes(size);
	if(qwSize>Block::QWORD_CAPACITY)
		return directAlloc(size);

	BaseMutex::ScopedLock lock(mMutex);
	return allocChunk(qwSize);
}


void Cm::FreeListAllocator::deallocate(void* ptr)
{	
	if(!ptr)
		return;

	Chunk *chunk = Chunk::fromMemAddr(ptr);

	if(!chunk->isUsed())
	{
		directFree(ptr);
		return;
	}

	BaseMutex::ScopedLock lock(mMutex);
	freeChunk(chunk);
}


void Cm::dumpFreeListAllocatorState(FreeListAllocator &a)
{
	puts("==================================");
	if(a.mBlockList==0)
	{
		puts("EMPTY");
		return;
	}
	for(Block *b = a.mBlockList; b; b = b->mLink.getNext())
	{
		printf("Block: %p\n",(void*)b);
		for(Chunk *c = b->firstChunk();c;c = c->getNextInBlock())
		{
			printf("Chunk %p, size %u, prevSize %u, %s %s %s\n",
					(void*)c, c->getQWSize(),c->getPrevChunkQWSize(), c->isUsed()?"USED":"",
					c->mHeader&c->FIRST?"FIRST":"",	c->mHeader&c->LAST?"LAST":"");
		}
	}
}

Cm::FreeListAllocator* Cm::createFreeListAllocator(PxAllocatorCallback& baseAllocator)
{
	void* a = baseAllocator.allocate(sizeof(Cm::FreeListAllocator), "FreeListAllocator", __FILE__, __LINE__);
	return a ? new(a) Cm::FreeListAllocator(baseAllocator) : NULL;
}

void Cm::destroyFreeListAllocator(Cm::FreeListAllocator& a)
{
	Cm::FreeListAllocator *p = &a;
	PxAllocatorCallback& baseAllocator = a.getBaseAllocator();

	a.~FreeListAllocator();
	baseAllocator.deallocate(p);
}


