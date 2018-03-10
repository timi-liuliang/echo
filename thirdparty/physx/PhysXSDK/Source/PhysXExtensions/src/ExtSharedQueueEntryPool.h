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


#ifndef PX_PHYSICS_EXTENSIONS_NP_SHARED_QUEUE_ENTRY_POOL_H
#define PX_PHYSICS_EXTENSIONS_NP_SHARED_QUEUE_ENTRY_POOL_H

#include "CmPhysXCommon.h"
#include "PsAllocator.h"
#include "PsAlignedMalloc.h"
#include "PsArray.h"
#include "PsSList.h"

namespace physx
{
namespace Ext
{
	class SharedQueueEntry : public Ps::SListEntry
	{
	public:
		SharedQueueEntry(void* objectRef) : mObjectRef(objectRef), mPooledEntry(false) {}
		SharedQueueEntry() : mObjectRef(NULL), mPooledEntry(true) {}

	public:
		void* mObjectRef;
		bool mPooledEntry; // True if the entry was preallocated in a pool
	};


#pragma warning(push)
#pragma warning(disable:4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
								// Because of the SList member I assume*/

	template<class Alloc = typename Ps::AllocatorTraits<SharedQueueEntry>::Type >
	class SharedQueueEntryPool : private Alloc
	{
	public:
		SharedQueueEntryPool(PxU32 poolSize, const Alloc& alloc = Alloc(PX_DEBUG_EXP("SharedQueueEntryPool")));
		~SharedQueueEntryPool();

		SharedQueueEntry* getEntry(void* objectRef);
		void putEntry(SharedQueueEntry& entry);

	private:
		SharedQueueEntry*					mTaskEntryPool;
		Ps::SList							mTaskEntryPtrPool;
	};

#pragma warning(pop)

} // namespace Ext


template <class Alloc>
Ext::SharedQueueEntryPool<Alloc>::SharedQueueEntryPool(PxU32 poolSize, const Alloc& alloc)
	: Alloc(alloc)
{
	Ps::AlignedAllocator<PX_SLIST_ALIGNMENT, Alloc> alignedAlloc("SharedQueueEntryPool");

	mTaskEntryPool = poolSize ? (SharedQueueEntry*)alignedAlloc.allocate(sizeof(SharedQueueEntry) * poolSize, __FILE__, __LINE__) : NULL;

	if (mTaskEntryPool)
	{
		for(PxU32 i=0; i < poolSize; i++)
		{
			PX_ASSERT((size_t(&mTaskEntryPool[i]) & (PX_SLIST_ALIGNMENT-1)) == 0);  // The SList entry must be aligned according to PX_SLIST_ALIGNMENT

			PX_PLACEMENT_NEW(&mTaskEntryPool[i], SharedQueueEntry)();
			PX_ASSERT(mTaskEntryPool[i].mPooledEntry == true);
			mTaskEntryPtrPool.push(mTaskEntryPool[i]);
		}
	}
}


template <class Alloc>
Ext::SharedQueueEntryPool<Alloc>::~SharedQueueEntryPool()
{
	if (mTaskEntryPool)
	{
		Ps::AlignedAllocator<PX_SLIST_ALIGNMENT, Alloc> alignedAlloc("SharedQueueEntryPool");
		alignedAlloc.deallocate(mTaskEntryPool);
	}
}


template <class Alloc>
Ext::SharedQueueEntry* Ext::SharedQueueEntryPool<Alloc>::getEntry(void* objectRef)
{
	SharedQueueEntry* e = static_cast<SharedQueueEntry*>(mTaskEntryPtrPool.pop());
	if (e)
	{
		PX_ASSERT(e->mPooledEntry == true);
		e->mObjectRef = objectRef;
		return e;
	}
	else
	{
		Ps::AlignedAllocator<PX_SLIST_ALIGNMENT, Alloc> alignedAlloc;
		e = (SharedQueueEntry*)alignedAlloc.allocate(sizeof(SharedQueueEntry), __FILE__, __LINE__);
		if (e)
		{
			PX_PLACEMENT_NEW(e, SharedQueueEntry)(objectRef);
			PX_ASSERT(e->mPooledEntry == false);
		}

		return e;
	}
}


template <class Alloc>
void Ext::SharedQueueEntryPool<Alloc>::putEntry(Ext::SharedQueueEntry& entry)
{
	if (entry.mPooledEntry)
	{
		entry.mObjectRef = NULL;
		mTaskEntryPtrPool.push(entry);
	}
	else
	{
		Ps::AlignedAllocator<PX_SLIST_ALIGNMENT, Alloc> alignedAlloc;
		alignedAlloc.deallocate(&entry);
	}
}

}

#endif
