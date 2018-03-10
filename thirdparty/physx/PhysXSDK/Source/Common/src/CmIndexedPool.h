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


#ifndef PX_PHYSICS_COMMON_INDEXEDPOOL
#define PX_PHYSICS_COMMON_INDEXEDPOOL

#include "PsArray.h"
#include "CmBitMap.h"
#include "PsBasicTemplates.h"

namespace physx
{
namespace Cm
{
	class IndexedPoolEntry
	{
	public:
		PxU32 getPoolIndex()							const	{ return mIndex; }
		IndexedPoolEntry()										{}
	private:
		PxU32 mIndex;
		IndexedPoolEntry(PxU32 index): mIndex(index)			{}
		template <class, PxU32> friend class IndexedPool;
	};


	/*!
	Allocator for pools of data structures
	Also decodes indices (which can be computed from handles) 
	EltsPerSlab must be a power of two, and T must inherit publicly from IndexedPoolEntry
	*/
	template <class T, PxU32 EltsPerSlab> 
	class IndexedPool
	{
		PX_NOCOPY(IndexedPool)

	public:
		static const PxU32 LOG2_ELTS_PER_SLAB = Ps::LogTwo<EltsPerSlab>::value;

		IndexedPool() : mSlabs(PX_DEBUG_EXP("indexedPoolSlabs")), mFreeList(PX_DEBUG_EXP("indexedPoolFreeList"))
		{
			PX_COMPILE_TIME_ASSERT(EltsPerSlab && !(EltsPerSlab & (EltsPerSlab-1)));	// non-0 power of 2
			Cm::IndexedPoolEntry* dummy = static_cast<Cm::IndexedPoolEntry *>(static_cast<T *>(0));					// must inherit from IndexedPoolEntry
			PX_UNUSED(dummy);
		}

		~IndexedPool()
		{
			for(PxU32 i=0;i<mSlabs.size();i++)
			{
				for(PxU32 j=0;j<EltsPerSlab;j++)
				{
					if(mUsedBitmap.boundedTest(i*EltsPerSlab+j))
						mSlabs[i][j].~T();
				}
				PX_FREE_AND_RESET(mSlabs[i]);
			}
		}

		PX_FORCE_INLINE T* construct()
		{
			T* t = reinterpret_cast<T*>(get());
			if(t)
				new (t) T();
			return t;
		}

		template<class A1>
		PX_FORCE_INLINE T* construct(const A1& a)
		{
			T* t = reinterpret_cast<T*>(get());
			if(t)
				new (t) T (a);
			return t;
		}

		template<class A1, class A2>
		PX_FORCE_INLINE T* construct(const A1& a, const A2& b)
		{
			T* t = reinterpret_cast<T*>(get());
			if(t)
				new (t) T (a, b);

			return t;
		}

		template<class A1, class A2, class A3>
		PX_FORCE_INLINE T* construct(const A1& a, const A2& b, const A3& c)
		{
			T* t = reinterpret_cast<T*>(get());
			if(t)
				new (t) T (a, b, c);
			return t;
		}

		template<class A1, class A2, class A3, class A4>
		PX_FORCE_INLINE T* construct(const A1& a, const A2& b, const A3& c, const A4& d)
		{
			T* t = reinterpret_cast<T*>(get());
			if(t)
				new (t) T (a, b, c, d);
			return t;
		}


		void destroy(T* element)
		{
			PxU32 i = element->getPoolIndex();
			mUsedBitmap.growAndReset(i);
			mFreeList.pushBack(element);
		}

		PX_FORCE_INLINE const T& operator[](PxU32 index) const
		{
			PX_ASSERT(index<mSlabs.size()*EltsPerSlab && mUsedBitmap.boundedTest(index));
			return mSlabs[index>>LOG2_ELTS_PER_SLAB][index&(EltsPerSlab-1)];
		}

		PX_FORCE_INLINE T& operator[](PxU32 index)
		{
			PX_ASSERT(index<mSlabs.size()*EltsPerSlab && mUsedBitmap.boundedTest(index));
			return mSlabs[index>>LOG2_ELTS_PER_SLAB][index&(EltsPerSlab-1)];
		}

		class Iterator
		{
		public:
			PX_INLINE Iterator(Cm::IndexedPool<T, EltsPerSlab>& pool) : mPool(pool), mBitmapIter(mPool.mUsedBitmap)
			{
				mIndex = mBitmapIter.getNext();
			}

			PX_INLINE T& operator*() { return mPool[mIndex]; }
			PX_INLINE const T& operator*() const { return mPool[mIndex]; }
			PX_INLINE Iterator& operator++() { mIndex = mBitmapIter.getNext(); return *this; }
			PX_INLINE bool done() const { return mIndex == Cm::BitMap::Iterator::DONE; }

		private:
			Iterator& operator=(const Iterator&);
			Cm::IndexedPool<T, EltsPerSlab>& mPool;
			Cm::BitMap::Iterator mBitmapIter;
			PxU32 mIndex;
		};


	private:
		PX_INLINE T* get()
		{
			if(mFreeList.size() == 0 && !extend())
				return 0;
			T* element = mFreeList.popBack();
			mUsedBitmap.growAndSet(element->getPoolIndex());
			return element;
		}


		bool extend()
		{
			T* mAddr = reinterpret_cast<T*>(PX_ALLOC(EltsPerSlab * sizeof(T), PX_DEBUG_EXP("char")));
			if(!mAddr)
				return false;

			mFreeList.reserve(EltsPerSlab);	
			for(PxI32 i=EltsPerSlab-1;i>=0;i--)
			{
				IndexedPoolEntry *e = static_cast<IndexedPoolEntry *>(reinterpret_cast<T *>(mAddr+i));
				new(e)IndexedPoolEntry(mSlabs.size()*EltsPerSlab+i);
				mFreeList.pushBack(mAddr+i);
			}

			mSlabs.pushBack(mAddr);
			mUsedBitmap.growAndReset(mSlabs.size()*EltsPerSlab-1); //make sure the bitmap is up to size

			return true;
		}

		Ps::Array<T*>			mSlabs;
		Ps::Array<T*>			mFreeList;
		Cm::BitMap				mUsedBitmap;

		friend class Iterator;
	};


} // namespace Cm

}

#endif
