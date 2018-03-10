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


#ifndef PX_FOUNDATION_PSPOOL_H
#define PX_FOUNDATION_PSPOOL_H

#include "PsArray.h"
#include "PsSort.h"
#include "PsBasicTemplates.h"
#include "PsUtilities.h"

namespace physx
{
namespace shdfnd
{

#define AUTO_RELEASE_SLAB_NUM	50

	/*!
	Simple allocation pool
	*/
	template<class T, class Alloc = typename AllocatorTraits<T>::Type>
	class PoolBase : public UserAllocated, public Alloc
	{
		PX_NOCOPY(PoolBase)
	protected:
		PoolBase (const Alloc& alloc, PxU32 elementsPerSlab, PxU32 slabSize) 
		: Alloc(alloc)
		, mSlabs(alloc)
		, mElementsPerSlab(elementsPerSlab)
		, mUsed(0)
		, mUnReleasedFree(0)
		, mSlabSize(slabSize)
		, mFreeElement(0)
		{
			PX_COMPILE_TIME_ASSERT(sizeof(T) >= sizeof(size_t));
		}

	public:

		~PoolBase ()
		{
			if(mUsed)
				disposeElements();

			for(void** slabIt = mSlabs.begin(), *slabEnd = mSlabs.end(); slabIt!=slabEnd; ++slabIt)
				Alloc::deallocate(*slabIt);
		}

		// Allocate space for single object
		PX_INLINE T* allocate()
		{
			if(mFreeElement == 0)
				allocateSlab();
			T* p = reinterpret_cast<T*>(mFreeElement);			
			mFreeElement = mFreeElement->mNext;
			mUsed++;
			mUnReleasedFree--;
/**
Mark a specified amount of memory with 0xcd pattern. This is used to check that the meta data 
definition for serialized classes is complete in checked builds.
*/
#if defined(PX_CHECKED)
			for (PxU32 i = 0; i < sizeof(T); ++i)
               reinterpret_cast<PxU8*>(p)[i] = 0xcd;
#endif
			return p;
		}

		// Put space for a single element back in the lists
		PX_INLINE void deallocate(T* p)
		{
			if(p)
			{
				PX_ASSERT(mUsed);
				mUsed--;
				push(reinterpret_cast<FreeList*>(p));
			}
			
			if( mUnReleasedFree > (PxI32)(AUTO_RELEASE_SLAB_NUM * mElementsPerSlab))
			{
				releaseEmptySlabs();
				mUnReleasedFree = 0;				
			}			
		}


		PX_INLINE T* construct()
		{
			T* t = allocate();
			return t ? new (t) T() : 0;
		}

		template<class A1>
		PX_INLINE T* construct(A1& a)
		{
			T* t = allocate();
			return t ? new (t) T(a) : 0;
		}

		template<class A1, class A2>
		PX_INLINE T* construct(A1& a, A2& b)
		{
			T* t = allocate();
			return t ? new (t) T(a,b) : 0;
		}

		template<class A1, class A2, class A3>
		PX_INLINE T* construct(A1& a, A2& b, A3& c)
		{
			T* t = allocate();
			return t ? new (t) T(a,b,c) : 0;
		}
		
		template<class A1, class A2, class A3>
		PX_INLINE T* construct(A1* a, A2& b, A3& c)
		{
			T* t = allocate();
			return t ? new (t) T(a,b,c) : 0;
		}

		template<class A1, class A2, class A3, class A4>
		PX_INLINE T* construct(A1& a, A2& b, A3& c, A4& d)
		{
			T* t = allocate();
			return t ? new (t) T(a,b,c,d) : 0;
		}

		template<class A1, class A2, class A3, class A4, class A5>
		PX_INLINE T* construct(A1& a, A2& b, A3& c, A4& d, A5& e)
		{
			T* t = allocate();
			return t ? new (t) T(a,b,c,d,e) : 0;
		}

		PX_INLINE void destroy(T* const p)
		{
			if(p)
			{
				p->~T();
				deallocate(p);
			}
		}

	protected:
		struct FreeList
		{
			FreeList* mNext;
		};

		// All the allocated slabs, sorted by pointer
		InlineArray<void*, 64, Alloc> mSlabs;

		PxU32 mElementsPerSlab;
		PxU32 mUsed;
		PxI32 mUnReleasedFree;
		PxU32 mSlabSize;

		FreeList* mFreeElement; // Head of free-list

		// Helper function to get bitmap of allocated elements

		void push(FreeList* p)
		{
			p->mNext = mFreeElement;
			mFreeElement = p;
			mUnReleasedFree++;
		}

		// Allocate a slab and segregate it into the freelist
		void allocateSlab ()
		{
			T* slab = reinterpret_cast<T*>(Alloc::allocate(mSlabSize, __FILE__, __LINE__));

			mSlabs.pushBack(slab);

			// Build a chain of nodes for the freelist
			T* it = slab + mElementsPerSlab;
			while(--it >= slab)
				push(reinterpret_cast<FreeList*>(it));
		}

		/*
		Cleanup method. Go through all active slabs and call destructor for live objects,
		then free their memory
		*/
		void disposeElements ()
		{
			Array<void*, Alloc> freeNodes(*this);
			while(mFreeElement)
			{
				freeNodes.pushBack(mFreeElement);
				mFreeElement = mFreeElement->mNext;
			}
			Alloc& alloc( *this );
			sort(freeNodes.begin(), freeNodes.size(), Less<void *>(),alloc);
			sort(mSlabs.begin(), mSlabs.size(), Less<void *>(),alloc);

 			typename Array<void*, Alloc>::Iterator slabIt = mSlabs.begin(), slabEnd = mSlabs.end();
			for(typename Array<void*, Alloc>::Iterator freeIt = freeNodes.begin(); slabIt != slabEnd; ++slabIt)
			{
				for(T *tIt = reinterpret_cast<T*>(*slabIt), *tEnd = tIt + mElementsPerSlab; tIt != tEnd; ++tIt)
				{
					if(freeIt != freeNodes.end() && *freeIt == tIt)
						++freeIt;
					else
						tIt->~T();
				}
			}
		}

		/*
		Go through all slabs and call destructor if the slab is empty		
		*/
		void releaseEmptySlabs()
		{
			Array<void*, Alloc> freeNodes(*this);
			Array<void*, Alloc> slabNodes(mSlabs, *this);
			while(mFreeElement)
			{
				freeNodes.pushBack(mFreeElement);
				mFreeElement = mFreeElement->mNext;
			}

			typename Array<void*, Alloc>::Iterator freeIt = freeNodes.begin(), freeEnd = freeNodes.end(), lastCheck = freeNodes.end() - mElementsPerSlab;

			if(freeNodes.size() > mElementsPerSlab)
			{
				Alloc& alloc( *this );
				sort(freeNodes.begin(), freeNodes.size(), Less<void *>(),alloc);
				sort(slabNodes.begin(), slabNodes.size(), Less<void *>(),alloc);

				mSlabs.clear();			
				for(void** slabIt = slabNodes.begin(), *slabEnd = slabNodes.end(); slabIt!=slabEnd; ++slabIt)
				{					
					while((freeIt < lastCheck) && (*slabIt > (*freeIt)))
					{
						push(reinterpret_cast<FreeList*>(*freeIt));
						freeIt++;						
					}

					if(*slabIt == (*freeIt))  //the slab's first element in freeList
					{
						size_t endSlabAddress = (size_t)(*slabIt) + mSlabSize;
						size_t endFreeAddress = (size_t)(*(freeIt + mElementsPerSlab -1));
						if(endFreeAddress + sizeof(T) == endSlabAddress)						
						{   //all slab's element in freeList
							Alloc::deallocate(*slabIt);							
							freeIt += mElementsPerSlab;
							continue;
						}
					}

					mSlabs.pushBack(*slabIt);					
				}
			}

			while(freeIt != freeEnd)
			{
				push(reinterpret_cast<FreeList*>(*freeIt));	
				++freeIt;
			}				
		}
	};

	// original pool implementation
	template<class T, class Alloc = typename AllocatorTraits<T>::Type>
	class Pool : public PoolBase<T, Alloc>
	{
	public:
		Pool (const Alloc& alloc = Alloc(), PxU32 elementsPerSlab = 32):
		  PoolBase<T, Alloc>(alloc, elementsPerSlab, elementsPerSlab * sizeof(T)) 
		  {
		  }

	};


	// allows specification of the slab size instead of the occupancy
	template<class T, PxU32 slabSize, class Alloc = typename AllocatorTraits<T>::Type>
	class Pool2 : public PoolBase<T, Alloc>
	{
	public:
		Pool2 (const Alloc& alloc = Alloc()):
		  PoolBase<T, Alloc>(alloc, slabSize/sizeof(T), slabSize) 
		  {
		  }
	};


} // namespace shdfnd
} // namespace physx


#endif
