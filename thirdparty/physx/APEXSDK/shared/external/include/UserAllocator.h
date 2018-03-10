/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USERALLOCATOR_H
#define USERALLOCATOR_H

#include "NxApexDefs.h"
#include "foundation/PxAllocatorCallback.h"
#include "PsShare.h"

#pragma warning(push)
#pragma warning(disable:4512)

#if defined(PX_CHECKED) || defined(_DEBUG)
#if defined(PX_WINDOWS)
#define USE_MEM_TRACKER
#endif
#endif

class NxUserAllocator;
class PhysX28Allocator;

namespace MEM_TRACKER
{
	class MemTracker;
};

/* User allocator for APEX and 3.0 PhysX SDK */
class UserPxAllocator : public physx::PxAllocatorCallback
{
public:
	UserPxAllocator(const char* context, const char* dllName, bool useTrackerIfSupported = true);
	virtual		   ~UserPxAllocator();

	physx::PxU32	getHandle(const char* name);


	void*			allocate(size_t size, const char* typeName, const char* filename, int line);
	void			deallocate(void* ptr);

	size_t			getAllocatedMemoryBytes()
	{
		return mMemoryAllocated;
	}

	NxUserAllocator* get28PhysXAllocator();

	static bool dumpMemoryLeaks(const char* filename);

private:
	bool				trackerEnabled() const { return mUseTracker && (NULL != mMemoryTracker); }

	const char*			mContext;
	size_t				mMemoryAllocated;
	PhysX28Allocator*	mNxAllocator;
	const bool			mUseTracker;

	static MEM_TRACKER::MemTracker	*mMemoryTracker;
	static int gMemoryTrackerClients;

	// Poor man's memory leak check
	static unsigned int mNumAllocations;
	static unsigned int mNumFrees;

	friend class PhysX28Allocator;
};

#pragma warning(pop)

#endif
