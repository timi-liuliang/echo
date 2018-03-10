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


#ifndef PX_SLIST_H
#define PX_SLIST_H

#include "foundation/Px.h"
#include "PsAlignedMalloc.h"

#if defined(PX_P64)
	#define PX_SLIST_ALIGNMENT 16
#else
	#define PX_SLIST_ALIGNMENT 8
#endif

namespace physx
{
namespace shdfnd
{

#if defined(PX_VC) 
    #pragma warning(push)   
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

#if !defined(PX_GNUC) && !defined(PX_GHS)
	__declspec(align(PX_SLIST_ALIGNMENT))
#endif
	class SListEntry
	{
		friend struct SListImpl;
	public:

		SListEntry() : mNext(NULL) 
		{
			PX_ASSERT((size_t(this) & (PX_SLIST_ALIGNMENT-1)) == 0);
		}

		// Only use on elements returned by SList::flush()
		// because the operation is not atomic.
		SListEntry* next() { return mNext; }
	private:

		SListEntry *mNext;
	}
#if defined(PX_GNUC) || defined(PX_GHS)
	__attribute__ ((aligned(PX_SLIST_ALIGNMENT)));
#else
	;
#endif

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

	// template-less implementation
	struct PX_FOUNDATION_API SListImpl
	{
		SListImpl();
		~SListImpl();
		void push(SListEntry* entry);
		SListEntry* pop();
		SListEntry* flush();
		static const PxU32& getSize();
	}
	;

	template <typename Alloc = ReflectionAllocator<SListImpl> >
	class SListT : protected Alloc
	{
	public:

		SListT(const Alloc& alloc = Alloc())
			: Alloc(alloc)
		{
			mImpl = (SListImpl*)Alloc::allocate(SListImpl::getSize(), __FILE__, __LINE__);
			PX_ASSERT((size_t(mImpl) & (PX_SLIST_ALIGNMENT-1)) == 0);
			PX_PLACEMENT_NEW(mImpl, SListImpl)();
		}
		~SListT()
		{
			mImpl->~SListImpl();
			Alloc::deallocate(mImpl);
		}

		// pushes a new element to the list
		void push( SListEntry& entry )
		{
			mImpl->push(&entry);
		}

		// pops an element from the list
		SListEntry* pop()
		{
			return mImpl->pop();
		}

		// removes all items from list, returns pointer to first element
		SListEntry* flush()
		{
			return mImpl->flush();
		}
	private:
		SListImpl *mImpl;
	};
	
	typedef SListT<> SList;


} // namespace shdfnd
} // namespace physx

#endif
