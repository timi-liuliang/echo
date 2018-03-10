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


#include "windows/PsWindowsInclude.h"
#include "PsAllocator.h"
#include "PsSList.h"

namespace physx
{
namespace shdfnd
{
	namespace 
	{
		template <typename T>
		SLIST_HEADER* getDetail(T* impl)
		{
			return reinterpret_cast<SLIST_HEADER*>(impl);
		}
	}

	SListImpl::SListImpl()
	{
		InitializeSListHead(getDetail(this));
	}

	SListImpl::~SListImpl()
	{
	}

	void SListImpl::push(SListEntry* entry)
	{
		InterlockedPushEntrySList(getDetail(this), reinterpret_cast<SLIST_ENTRY*>(entry));
	}

	SListEntry* SListImpl::pop()
	{
		return reinterpret_cast<SListEntry*>(InterlockedPopEntrySList(getDetail(this)));
	}

	SListEntry* SListImpl::flush()
	{
		return reinterpret_cast<SListEntry*>(InterlockedFlushSList(getDetail(this)));
	}

	static const PxU32 gSize = sizeof(SLIST_HEADER);

	const PxU32& SListImpl::getSize()
	{
		return gSize;
	}

} // namespace shdfnd
} // namespace physx