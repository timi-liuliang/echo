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


#include "PsAllocator.h"
#include "PsFoundation.h"
#include "PsMutex.h"

namespace physx
{
namespace shdfnd
{

#if defined(PX_DEBUG) || defined(PX_CHECKED) // NamedAllocator is slow and should only be used in debug mode

	namespace {
		typedef HashMap<const NamedAllocator*, const char*, 
			Hash<const NamedAllocator*>, NonTrackingAllocator> AllocNameMap;

		PX_INLINE AllocNameMap& getMap() { return getFoundation().getNamedAllocMap(); }
		PX_INLINE Foundation::Mutex& getMutex() { return getFoundation().getNamedAllocMutex(); }
	}

NamedAllocator::NamedAllocator(const PxEMPTY&)
{
	Foundation::Mutex::ScopedLock lock(getMutex());
	getMap().insert(this, 0);
}

NamedAllocator::NamedAllocator(const char* name) 
{	
	Foundation::Mutex::ScopedLock lock(getMutex());
	getMap().insert(this, name);
}

NamedAllocator::NamedAllocator(const NamedAllocator& other)
{
	Foundation::Mutex::ScopedLock lock(getMutex());
	const AllocNameMap::Entry* e = getMap().find(&other); PX_ASSERT(e);
	const char* name = e->second;  // The copy is important because insert might invalidate the referenced hash entry
	getMap().insert(this, name);
}

NamedAllocator::~NamedAllocator() 
{
	Foundation::Mutex::ScopedLock lock(getMutex());
	bool erased = getMap().erase(this);
	PX_UNUSED(erased);
	PX_ASSERT(erased);
}

NamedAllocator& NamedAllocator::operator=(const NamedAllocator& other)
{
	Foundation::Mutex::ScopedLock lock(getMutex());
	const AllocNameMap::Entry* e = getMap().find(&other); PX_ASSERT(e);
	getMap()[this] = e->second;
	return *this;
}

void* NamedAllocator::allocate(size_t size, const char* filename, int line)
{
	if(!size)
		return 0;
	Foundation::Mutex::ScopedLock lock(getMutex());
	const AllocNameMap::Entry* e = getMap().find(this); PX_ASSERT(e);
	return getAllocator().allocate(size, e->second, filename, line);
}

void NamedAllocator::deallocate(void* ptr)
{
	if(ptr)
		getAllocator().deallocate(ptr);
}

#endif // PX_DEBUG

void* Allocator::allocate(size_t size, const char* file, int line)
{
	if(!size)
		return 0;
	return getAllocator().allocate(size, "", file, line);
}
void Allocator::deallocate(void* ptr)
{
	if(ptr)
		getAllocator().deallocate(ptr);
}

} // namespace shdfnd
} // namespace physx
