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


#ifndef PX_PHYSICS_NP_PTRTABLESTORAGEMANAGER_H
#define PX_PHYSICS_NP_PTRTABLESTORAGEMANAGER_H

#include "CmPhysXCommon.h"
#include "PsMutex.h"
#include "PsUserAllocated.h"
#include "CmPtrTable.h"
#include "PsBitUtils.h"

namespace physx
{
class NpPtrTableStorageManager : public Cm::PtrTableStorageManager, public Ps::UserAllocated
{
	PX_NOCOPY(NpPtrTableStorageManager)

public:

	NpPtrTableStorageManager() {}
	~NpPtrTableStorageManager() {}

	void**	allocate(PxU32 capacity)
	{
		PX_ASSERT(Ps::isPowerOfTwo(capacity));

		Ps::Mutex::ScopedLock lock(mMutex);

		return capacity<=4*sizeof(void*)  ? reinterpret_cast<void**>(mPool4.construct())
			 : capacity<=16*sizeof(void*) ? reinterpret_cast<void**>(mPool16.construct())
			 : capacity<=64*sizeof(void*) ? reinterpret_cast<void**>(mPool64.construct())
			 : reinterpret_cast<void**>(PX_ALLOC(capacity*sizeof(void*), "CmPtrTable pointer array"));							
	}

	void deallocate(void** addr, PxU32 capacity)
	{
		PX_ASSERT(Ps::isPowerOfTwo(capacity));

		Ps::Mutex::ScopedLock lock(mMutex);

		if(capacity<=4*sizeof(void*))			mPool4.destroy(reinterpret_cast< PtrBlock<4>*>(addr));
		else if(capacity<=16*sizeof(void*))		mPool16.destroy(reinterpret_cast< PtrBlock<16>*>(addr));
		else if(capacity<=64*sizeof(void*))		mPool64.destroy(reinterpret_cast< PtrBlock<64>*>(addr));
		else									PX_FREE(addr);
	}

	// originalCapacity is the only way we know which pool the alloc request belongs to,
	// so if those are no longer going to match, we need to realloc.

	bool canReuse(PxU32 originalCapacity, PxU32 newCapacity)
	{
		PX_ASSERT(Ps::isPowerOfTwo(originalCapacity));
		PX_ASSERT(Ps::isPowerOfTwo(newCapacity));

		return poolId(originalCapacity) == poolId(newCapacity) && newCapacity<=64;
	}

private:
	Ps::Mutex mMutex;

	int poolId(PxU32 size)
	{
		return size<=4	? 0
			 : size<=16 ? 1
			 : size<=64 ? 2
			 : 3;
	}

	template<int N> class PtrBlock { void* ptr[N]; };

	Ps::Pool2<PtrBlock<4>, 4096 >		mPool4; 
	Ps::Pool2<PtrBlock<16>, 4096 >		mPool16;
	Ps::Pool2<PtrBlock<64>, 4096 >		mPool64;
};

}
#endif
