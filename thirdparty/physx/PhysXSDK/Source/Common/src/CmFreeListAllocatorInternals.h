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


#ifndef FREELIST_ALLOCATOR_INTERNALS_H
#define FREELIST_ALLOCATOR_INTERNALS_H

#include "CmPhysXCommon.h"
#include "PsBitUtils.h"
#include "PsIntrinsics.h"
#include "PxMath.h"

namespace physx
{
namespace Cm
{

	class FastBitMap
	{
	public:
		static const PxU32 NONE = 0xffff;
		static const PxU32 MAX_BITS = 512;

		FastBitMap(): mHMap(0)
		{
			for(PxU32 i=0;i<32;i++)
				mMap[i] = 0;
		}

		PX_INLINE void set(PxU32 val)
		{
			PxU32 high = val>>5, low = val & 31;
			mMap[high] |= (1<<low);
			mHMap |= 1<<high;
		}

		PX_INLINE void reset(PxU32 val)
		{
			PxU32 high = val>>5, low = val & 31;
			mMap[high]&= ~(1<<low);
			if(!mMap[high]) 
				mHMap &= ~(1<<high);
		}

		// find the index of the first bit set which is >= val
		PX_INLINE PxU32 findFirst(PxU32 val)
		{
			PxU32 high = val>>5, low = val & 31;
			
			PxU32 p = mMap[high] & ((PxU32)-1)<<low;		// check the word which includes the val'th bit
			if(!p)
			{
				PxU32 q = mHMap & ((PxU32)-1)<<(high+1);	// find the first non-empty word > high
				if(q==0)
					return NONE;
			 
				high = Ps::lowestSetBit(q);
				p = mMap[high];
			}

			return high<<5 | Ps::lowestSetBit(p);
		}
	private:
		PxU32 mMap[32];
		PxU32 mHMap;
	};


template
<typename T>
T *addAddr(T *addr, int offset)
{
	return reinterpret_cast<T*>(reinterpret_cast<char *>(addr)+offset);
}

template <typename T>
class Link
{
private:
    T *mNext;
	T *mPrev;
public:
	PX_INLINE	void init()						{	mNext = mPrev = 0;	}
	PX_INLINE	T	*getNext()					{	return mNext;		}
	PX_INLINE	T	*getPrev()					{	return mPrev;		}
	
	PX_INLINE	void link(T *&head, T *self)	
	{
		assert(mNext == 0 && mPrev == 0);
		mNext = head;
		if(head)
			head->mLink.mPrev = self;
		head = self;
	}

	PX_INLINE	void unlink(T *&head)
	{
		if(mNext)
			mNext->mLink.mPrev = mPrev;
		if(mPrev)
			mPrev->mLink.mNext = mNext;
		else
			head = mNext;

		mNext = mPrev = 0;
	}
};


class Chunk
{
public:
	typedef PxU32 QWSize;

	static const QWSize MIN_QWORD_SIZE = 1;

	PX_INLINE void init(QWSize qwSize)				{	mHeader = qwSize | FIRST | LAST;	mLink.init();	}
	PX_INLINE QWSize getQWSize()		const		{	return mHeader&QWORD_SIZE_MASK;						}
	PX_INLINE void acquire()						{	mHeader |= USED;								}
	PX_INLINE void release()						{	mHeader &= ~USED;	mLink.init();				}
	PX_INLINE bool isUsed()				const		{	return (mHeader&USED)!=0;						}

	PX_INLINE void link(Chunk *&head)				{	mLink.link(head, this);	}
	PX_INLINE void unlink(Chunk *&head)				{	mLink.unlink(head);		}

	PX_INLINE Chunk *getPrevInBlock()				{	return (mHeader&FIRST)?0:addAddr(this,-(int)getPrevChunkQWSize()*16);	}
	PX_INLINE Chunk *getNextInBlock()				{	return (mHeader&LAST)?0:addAddr(this,(int)getQWSize()*16);	}

	PX_INLINE void *memAddr()						{	return addAddr(this,4); 	}
	PX_INLINE static Chunk *fromMemAddr(void *m)	{  	return reinterpret_cast<Chunk *>(addAddr(m,-4));}

	static PX_INLINE QWSize getQWSizeFromBytes(size_t s)	
	{	
		QWSize h = (QWSize)(s+4+16-1)/16;
		return PxMax(h,MIN_QWORD_SIZE);
	}

	// merges the next-in-block chunk into this one
	void merge(Chunk &nib)
	{
		assert(!isUsed() && !nib.isUsed());
		QWSize firstQWSize = getQWSize();
		mHeader = ((mHeader&(PREV_QWORD_SIZE_MASK|FIRST)) | (nib.mHeader&LAST) | getQWSize() )+ nib.getQWSize();
		
		if(!(mHeader&LAST))
			getNextInBlock()->mHeader += firstQWSize<<PREV_QWORD_SIZE_SHIFT;
	}

	// splits this chunk down to requiredQWSize and returns the remainder chunk
	Chunk *split(QWSize requiredQWSize)
	{
		assert(!isUsed());

		QWSize rmdrQWSize = getQWSize() - requiredQWSize;
		assert(rmdrQWSize);

		Chunk *rmdr = addAddr(this,(int)requiredQWSize*16);
		rmdr->mHeader = rmdrQWSize | (mHeader&LAST) | (requiredQWSize<<PREV_QWORD_SIZE_SHIFT);
		rmdr->mLink.init();
		if(!(mHeader&LAST))
			getNextInBlock()->mHeader -= requiredQWSize<<PREV_QWORD_SIZE_SHIFT;

		mHeader = (mHeader&(PREV_QWORD_SIZE_MASK|FIRST)) | requiredQWSize;

		return rmdr;
	}

private:
	PxU32				mHeader;		// must be 4 bytes!!
	Link<Chunk>			mLink;

	friend class Link<Chunk>;

	static const PxU32 QWORD_SIZE_BITS				= 10;
	static const PxU32 QWORD_SIZE_MASK				= (1<<QWORD_SIZE_BITS) - 1;
	static const PxU32 PREV_QWORD_SIZE_SHIFT		= QWORD_SIZE_BITS;
	static const PxU32 PREV_QWORD_SIZE_MASK			= QWORD_SIZE_MASK << PREV_QWORD_SIZE_SHIFT;

	static const PxU32 LAST							= (PxU32)1<<29;
	static const PxU32 FIRST						= (PxU32)1<<30;
	static const PxU32 USED							= (PxU32)1<<31;
	
	// assumption is that this chunk is going to be immediately used,
	// so we don't need to fill in the link, just the header

	PX_INLINE QWSize getPrevChunkQWSize()		{	return (mHeader>>PREV_QWORD_SIZE_SHIFT)&QWORD_SIZE_MASK;	}

	friend void dumpFreeListAllocatorState(class FreeListAllocator &);
};

class Block
{
public:
	static const PxU32 ALLOCATION_SIZE	= 16384;

	// header size must be big enough for a link node and aligned at 16n+12 - so 28 for 64-bit platforms

#if defined(PX_P64)
	static const PxU32 HEADER_SIZE		= 28;
	static const PxU32 QWORD_CAPACITY	= 1022;
#else	
	static const PxU32 HEADER_SIZE		= 12;
	static const PxU32 QWORD_CAPACITY	= 1023;
#endif

	friend class Link<Block>;

	void			init(Block *&head)	
	{ 
		mLink.init(); 
		mLink.link(head,this);
		firstChunk()->init(QWORD_CAPACITY);
	}

	void			finalize(Block *&head)
	{
		mLink.unlink(head);
	}

	Chunk *			firstChunk()				{ return reinterpret_cast<Chunk *>(addAddr(this,HEADER_SIZE)); 		}
	static Block *	fromFirstChunk(Chunk *c)	{ return reinterpret_cast<Block *>(addAddr(c,-(int)HEADER_SIZE));	}
private:
	Link<Block>		mLink;

	friend void dumpFreeListAllocatorState(class FreeListAllocator &);

};

}

}

#endif
