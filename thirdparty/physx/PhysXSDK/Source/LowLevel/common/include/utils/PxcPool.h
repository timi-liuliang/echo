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


#ifndef PXC_POOL_H
#define PXC_POOL_H

#include "PsArray.h"
#include "PsSort.h"
#include "PsMutex.h"
#include "PsBasicTemplates.h"

#include "CmBitMap.h"
#include "CmPhysXCommon.h"

namespace physx
{

/*!
Allocator for pools of data structures
Also decodes indices (which can be computed from handles) into objects. To make this
faster, the EltsPerSlab must be a power of two
*/
template <class T, class ArgumentType> 
class PxcPoolList : public Ps::AllocatorTraits<T>::Type
{
	typedef typename Ps::AllocatorTraits<T>::Type Alloc;
	PX_NOCOPY(PxcPoolList)
public:
	PX_INLINE PxcPoolList(const Alloc& alloc, ArgumentType* argument, PxU32 eltsPerSlab, PxU32 maxSlabs)
		: Alloc(alloc),
		mEltsPerSlab(eltsPerSlab), 
		mMaxSlabs(maxSlabs), 
		mSlabCount(0),
		mFreeList(0), 
		mFreeCount(0), 
		mSlabs(reinterpret_cast<T**>(Alloc::allocate(maxSlabs * sizeof(T*), __FILE__, __LINE__))),
		mArgument(argument)
	{
		PX_ASSERT(mEltsPerSlab>0);
		// either maxSlabs = 1 (non-resizable pool), or elts per slab must be a power of two
		PX_ASSERT((maxSlabs==1) || ((maxSlabs < 8192) && (mEltsPerSlab & (mEltsPerSlab-1))) == 0);		
		mLog2EltsPerSlab = 0;

		if(mMaxSlabs>1)
		{
			for(mLog2EltsPerSlab=0; mEltsPerSlab!=(PxU32)1<<mLog2EltsPerSlab; mLog2EltsPerSlab++)
				;
		}
	}

	PX_INLINE ~PxcPoolList()
	{
		destroy();
	}

	PX_INLINE void destroy()
	{
		// Run all destructors
		for(PxU32 i=0;i<mSlabCount;i++)
		{
			PX_ASSERT(mSlabs);
			T* slab = mSlabs[i];
			for(PxU32 j=0;j<mEltsPerSlab;j++)
			{
				slab[j].~T();
			}
		}

		//Deallocate
		for(PxU32 i=0;i<mSlabCount;i++)
		{
			PX_FREE(mSlabs[i]);
			mSlabs[i] = NULL;
		}
		mSlabCount = 0;

		if(mFreeList)
			PX_FREE(mFreeList);
		mFreeList = NULL;
		if(mSlabs)
		{
			PX_FREE(mSlabs);
			mSlabs = NULL;
		}
	}

	// TODO: would be nice to add templated construct/destroy methods like PxcObjectPool

	PX_INLINE T* get()
	{
		if(mFreeCount == 0 && !extend())
			return 0;
		T* element = mFreeList[--mFreeCount];
		mUseBitmap.set(element->getIndex());
		return element;
	}

	PX_INLINE void put(T* element)
	{
		PxU32 i = element->getIndex();
		mUseBitmap.reset(i);
		mFreeList[mFreeCount++] = element;
	}

	/*
		WARNING: Unlike findByIndexFast below, this method is NOT safe to use if another thread 
		is concurrently updating the pool (e.g. through put/get/extend/getIterator), since the
		safety boundedTest uses mSlabCount and mUseBitmap.
	*/
	PX_FORCE_INLINE T* findByIndex(PxU32 index) const
	{
		if(index>=mSlabCount*mEltsPerSlab || !(mUseBitmap.boundedTest(index)))
			return 0;
		return mMaxSlabs==1 ? mSlabs[0]+index : mSlabs[index>>mLog2EltsPerSlab] + (index&(mEltsPerSlab-1));
	}

	/*
		This call is safe to do while other threads update the pool.
	*/
	PX_FORCE_INLINE T* findByIndexFast(PxU32 index) const
	{
		PX_ASSERT(mMaxSlabs != 1);
		return mSlabs[index>>mLog2EltsPerSlab] + (index&(mEltsPerSlab-1));
	}

	bool extend()
	{
		if(mSlabCount == mMaxSlabs)
			return false;
		T * mAddr = reinterpret_cast<T*>(Alloc::allocate(mEltsPerSlab * sizeof(T), __FILE__, __LINE__));
		if(!mAddr)
			return false;
		mSlabs[mSlabCount++] = mAddr;

		if(mFreeList)
			PX_FREE(mFreeList);
		mFreeList = reinterpret_cast<T**>(Alloc::allocate(mSlabCount * mEltsPerSlab * sizeof(T*), __FILE__, __LINE__));

		// Make sure the usage bitmap is up-to-size
		mUseBitmap.resize(mSlabCount*mEltsPerSlab); //set last element as not used
	
		// Add to free list in descending order so that lowest indices get allocated first - 
		// the FW context code currently *relies* on this behavior to grab the zero-index volume
		// which can't be allocated to the user. TODO: fix this

		for(PxI32 i=PxI32(mEltsPerSlab-1);i>=0;i--)
			mFreeList[mFreeCount++] = new(mAddr+i) T(mArgument,(mSlabCount-1) * mEltsPerSlab + i);

		return true;
	}

	PX_INLINE PxU32 getMaxUsedIndex()	const
	{
		return mUseBitmap.findLast();
	}

	PX_INLINE Cm::BitMap::Iterator getIterator() const
	{
		return Cm::BitMap::Iterator(mUseBitmap);
	}

private:
	const PxU32				mEltsPerSlab;
	const PxU32				mMaxSlabs;
	PxU32					mSlabCount;
	PxU32					mLog2EltsPerSlab;
	T**						mFreeList;
	PxU32					mFreeCount;
	T**						mSlabs;
	ArgumentType*			mArgument;
	Cm::BitMap				mUseBitmap;
};




/*!
Same as PxcPoolList, but with Mutex guards for allocation.
*/
template <class T, class ArgumentType> 
class PxcPoolListThreadSafe : public Ps::AllocatorTraits<T>::Type
{
	typedef typename Ps::AllocatorTraits<T>::Type Alloc;
public:
	PX_INLINE PxcPoolListThreadSafe(const Alloc& alloc, ArgumentType* argument, PxU32 eltsPerSlab, PxU32 maxSlabs)
		: Alloc(alloc),
		  mEltsPerSlab(eltsPerSlab), 
		  mMaxSlabs(maxSlabs), 
		  mSlabCount(0),
		  mFreeList(0), 
		  mFreeCount(0), 
		  mSlabs(reinterpret_cast<T**>(Alloc::allocate(maxSlabs * sizeof(T*), __FILE__, __LINE__))),
		  mArgument(argument)
	  {
		  PX_ASSERT(mEltsPerSlab>0);
		  // either maxSlabs = 1 (non-resizable pool), or elts per slab must be a power of two
		  PX_ASSERT(maxSlabs==1 || maxSlabs < 8192 && (mEltsPerSlab & (mEltsPerSlab-1)) == 0);		
		  mLog2EltsPerSlab = 0;

		  if(mMaxSlabs>1)
		  {
			  for(mLog2EltsPerSlab=0; mEltsPerSlab!=(PxU32)1<<mLog2EltsPerSlab; mLog2EltsPerSlab++)
				  ;
		  }
	  }

	  PX_INLINE ~PxcPoolListThreadSafe()
	  {
		  destroy();
	  }

	  PX_INLINE void destroy()
	  {
		  // Run all destructors
		  for(PxU32 i=0;i<mSlabCount;i++)
		  {
			  PX_ASSERT(mSlabs);
			  T* slab = mSlabs[i];
			  for(PxU32 j=0;j<mEltsPerSlab;j++)
			  {
				  slab[j].~T();
			  }
		  }

		  //Deallocate
		  for(PxU32 i=0;i<mSlabCount;i++)
		  {
			  PX_FREE(mSlabs[i]);
			  mSlabs[i] = NULL;
		  }
		  mSlabCount = 0;

		  if(mFreeList)
			  PX_FREE(mFreeList);
		  mFreeList = NULL;
		  if(mSlabs)
		  {
			  PX_FREE(mSlabs);
			  mSlabs = NULL;
		  }
	  }

	  // TODO: would be nice to add templated construct/destroy methods like PxcObjectPool

	  PX_INLINE T* get()
	  {
		  Ps::Mutex::ScopedLock lock(mMutex);
		  if(mFreeCount == 0 && !extend())
			  return 0;
		  T* element = mFreeList[--mFreeCount];
		  PxU32 i = element->getIndex();
		  mUseBitmap.growAndSet(i);
		  return element;
	  }

	  PX_INLINE void put(T* element)
	  {
		  Ps::Mutex::ScopedLock lock(mMutex);
		  PxU32 i = element->getIndex();
		  mUseBitmap.growAndReset(i);
		  mFreeList[mFreeCount++] = element;
	  }
	  /*
	  This call is safe to do while other threads update the pool.
	  */
	  PX_FORCE_INLINE T* findByIndexFast(PxU32 index) const
	  {
		  PX_ASSERT(mMaxSlabs != 1);
		  return mSlabs[index>>mLog2EltsPerSlab] + (index&(mEltsPerSlab-1));
	  }

	  bool extend()
	  {
		  if(mSlabCount == mMaxSlabs)
			  return false;
		  T * mAddr = reinterpret_cast<T*>(Alloc::allocate(mEltsPerSlab * sizeof(T), __FILE__, __LINE__));
		  if(!mAddr)
			  return false;
		  mSlabs[mSlabCount++] = mAddr;

		  if(mFreeList)
			  PX_FREE(mFreeList);
		  mFreeList = reinterpret_cast<T**>(Alloc::allocate(mSlabCount * mEltsPerSlab * sizeof(T*), __FILE__, __LINE__));

		  // Make sure the usage bitmap is up-to-size
		  mUseBitmap.resize(mSlabCount*mEltsPerSlab-1); //set last element as not used

		  // Add to free list in descending order so that lowest indices get allocated first - 
		  // the FW context code currently *relies* on this behavior to grab the zero-index volume
		  // which can't be allocated to the user. TODO: fix this.

		  for(PxI32 i=mEltsPerSlab-1;i>=0;i--)
			  mFreeList[mFreeCount++] = new(mAddr+i) T(mArgument,(mSlabCount-1) * mEltsPerSlab + i);

		  return true;
	  }

	  PX_INLINE PxU32 getMaxUsedIndex()	const
	  {
		  return mUseBitmap.findLast();
	  }

	  PX_INLINE Cm::BitMap::Iterator getIterator() const
	  {
		  return Cm::BitMap::Iterator(mUseBitmap);
	  }

private:
	const PxU32				mEltsPerSlab;
	const PxU32				mMaxSlabs;
	PxU32					mSlabCount;
	PxU32					mLog2EltsPerSlab;
	T**						mFreeList;
	PxU32					mFreeCount;
	T**						mSlabs;
	ArgumentType*			mArgument;
	Cm::BitMap				mUseBitmap;
	Ps::Mutex				mMutex;
};

}

#endif
