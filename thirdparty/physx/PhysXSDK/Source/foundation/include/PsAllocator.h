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


#ifndef PX_FOUNDATION_PSALLOCATOR_H
#define PX_FOUNDATION_PSALLOCATOR_H

#include "foundation/PxAllocatorCallback.h"
#include "foundation/PxFoundation.h"
#include "Ps.h"

#if (defined(PX_WINDOWS) || defined (PX_WINMODERN) || defined(PX_X360) || defined(PX_XBOXONE))
#include <exception>
#include <typeinfo.h>
#endif
#if (defined(PX_APPLE))
#include <typeinfo>
#endif


#ifdef PX_WIIU
#pragma ghs nowarning 193 //warning #193-D: zero used for undefined preprocessing identifier
#endif

#include <new>

#ifdef PX_WIIU
#pragma ghs endnowarning
#endif

// Allocation macros going through user allocator 
#ifdef PX_DEBUG
#define PX_ALLOC(n, name)		 physx::shdfnd::NamedAllocator(name).allocate(n, __FILE__, __LINE__)
#else
#define PX_ALLOC(n, name)        physx::shdfnd::Allocator().allocate(n, __FILE__, __LINE__)
#endif
#define PX_ALLOC_TEMP(n, name)   PX_ALLOC(n, name)
#define PX_FREE(x)              physx::shdfnd::Allocator().deallocate(x)
#define PX_FREE_AND_RESET(x)    { PX_FREE(x); x=0; }


// The following macros support plain-old-types and classes derived from UserAllocated.
#define PX_NEW(T)               new(physx::shdfnd::ReflectionAllocator<T>(), __FILE__, __LINE__) T
#define PX_NEW_TEMP(T)          PX_NEW(T)
#define PX_DELETE(x)            delete x
#define PX_DELETE_AND_RESET(x)  { PX_DELETE(x); x=0; }
#define PX_DELETE_POD(x)        { PX_FREE(x); x=0; }
#define PX_DELETE_ARRAY(x)      { PX_DELETE([]x); x=0; }

// aligned allocation
#define PX_ALIGNED16_ALLOC(n) 	physx::shdfnd::AlignedAllocator<16>().allocate(n, __FILE__, __LINE__)
#define PX_ALIGNED16_FREE(x)	physx::shdfnd::AlignedAllocator<16>().deallocate(x) 

//! placement new macro to make it easy to spot bad use of 'new'
#define PX_PLACEMENT_NEW(p, T)  new(p) T

// Don't use inline for alloca !!!
#if defined (PX_WINDOWS) || defined(PX_WINMODERN)
    #include <malloc.h>
    #define PxAlloca(x) _alloca(x)
#elif defined(PX_LINUX) || defined(PX_ANDROID)
    #include <malloc.h>
    #define PxAlloca(x) alloca(x)
#elif defined(PX_PSP2)
    #include <alloca.h>
    #define PxAlloca(x) alloca(x)
#elif defined(PX_APPLE)
    #include <alloca.h>
    #define PxAlloca(x) alloca(x)
#elif defined(PX_PS3)
    #include <alloca.h>
    #define PxAlloca(x) alloca(x)
#elif defined(PX_X360)
    #include <malloc.h>
    #define PxAlloca(x) _alloca(x)
#elif defined(PX_WIIU)
    #include <alloca.h>
    #define PxAlloca(x) alloca(x)
#elif defined(PX_PS4)
    #include <memory.h>
    #define PxAlloca(x) alloca(x)
#elif defined(PX_XBOXONE)
	#include <malloc.h>
	#define PxAlloca(x) alloca(x)
#endif

#define PxAllocaAligned(x, alignment) ((size_t(PxAlloca(x + alignment)) + (alignment - 1)) & ~size_t(alignment - 1))

namespace physx
{
namespace shdfnd
{
	PX_FOUNDATION_API PxAllocatorCallback& getAllocator();

	/*
	 * Bootstrap allocator using malloc/free.
	 * Don't use unless your objects get allocated before foundation is initialized.
	 */
	class RawAllocator
	{
	public:
		RawAllocator(const char* = 0) {}
		void* allocate(size_t size, const char*, int) 
		{
			// malloc returns valid pointer for size==0, no need to check
			return ::malloc(size); 
		}
		void deallocate(void* ptr) 
		{ 
			// free(0) is guaranteed to have no side effect, no need to check
			::free(ptr); 
		}
	};

	/*
	 * Allocator that simply calls straight back to the application without tracking.
	 * This is used by the heap (Foundation::mNamedAllocMap) that tracks allocations
	 * because it needs to be able to grow as a result of an allocation.
	 * Making the hash table re-entrant to deal with this may not make sense.
	 */
	class NonTrackingAllocator 
	{
	public:
		NonTrackingAllocator(const char* = 0) {}
		void* allocate(size_t size, const char* file, int line) 
		{
			return PxGetFoundation().getAllocatorCallback().allocate(size, "NonTrackedAlloc", file, line);
		}
		void deallocate(void* ptr) 
		{ 
			PxGetFoundation().getAllocatorCallback().deallocate(ptr);
		}
	};

	/**
	Allocator used to access the global PxAllocatorCallback instance without providing additional information.
	*/
	class PX_FOUNDATION_API Allocator
	{
	public:
		Allocator(const char* = 0) {}
		void* allocate(size_t size, const char* file, int line);
		void deallocate(void* ptr);
	};

	/**
	Allocator used to access the global PxAllocatorCallback instance using a dynamic name.
	*/
#if defined(PX_DEBUG) || defined(PX_CHECKED) // see comment in cpp
	class PX_FOUNDATION_API NamedAllocator
	{
	public:
		NamedAllocator(const PxEMPTY&);
		NamedAllocator(const char* name = 0); // todo: should not have default argument!
		NamedAllocator(const NamedAllocator&);
		~NamedAllocator();
		NamedAllocator& operator=(const NamedAllocator&);
		void* allocate(size_t size, const char* filename, int line);
		void deallocate(void* ptr);
	};
#else
	class NamedAllocator;
#endif // PX_DEBUG

	/**
    Allocator used to access the global PxAllocatorCallback instance using a static name derived from T.
	*/
	template <typename T>
	class ReflectionAllocator
	{
		static const char* getName()
		{
			if(!PxGetFoundation().getReportAllocationNames())
				return "<allocation names disabled>";
#if defined(PX_GNUC) || defined(PX_GHS)
			return __PRETTY_FUNCTION__;
#else
			// name() calls malloc(), raw_name() wouldn't
			return typeid(T).name();
#endif
		}
	public:
		ReflectionAllocator(const PxEMPTY&)	{}
		ReflectionAllocator(const char* =0) {}
		inline ReflectionAllocator(const ReflectionAllocator& ) { }
		void* allocate(size_t size, const char* filename, int line)
		{
			return size ? getAllocator().allocate(size, getName(), filename, line) : 0;
		}
		void deallocate(void* ptr)
		{
			if(ptr)
				getAllocator().deallocate(ptr);
		}
	};

	template <typename T>
	struct AllocatorTraits
	{
#if defined(PX_CHECKED) // checked and debug builds
		typedef NamedAllocator Type;
#else
		typedef ReflectionAllocator<T> Type;
#endif
	};

    // if you get a build error here, you are trying to PX_NEW a class
    // that is neither plain-old-type nor derived from UserAllocated
	template <typename T, typename X>
	union EnableIfPod
	{
		int i; T t;
		typedef X Type;
	};

} // namespace shdfnd
} // namespace physx

// Global placement new for ReflectionAllocator templated by
// plain-old-type. Allows using PX_NEW for pointers and built-in-types.
//
// ATTENTION: You need to use PX_DELETE_POD or PX_FREE to deallocate
// memory, not PX_DELETE. PX_DELETE_POD redirects to PX_FREE.
//
// Rationale: PX_DELETE uses global operator delete(void*), which we dont' want to overload.
// Any other definition of PX_DELETE couldn't support array syntax 'PX_DELETE([]a);'.
// PX_DELETE_POD was preferred over PX_DELETE_ARRAY because it is used
// less often and applies to both single instances and arrays.
template <typename T>
PX_INLINE void* operator new(size_t size, physx::shdfnd::ReflectionAllocator<T> alloc, const char* fileName, typename physx::shdfnd::EnableIfPod<T, int>::Type line)
{
	return alloc.allocate(size, fileName, line);
}

template <typename T>
PX_INLINE void* operator new[](size_t size, physx::shdfnd::ReflectionAllocator<T> alloc, const char* fileName, typename physx::shdfnd::EnableIfPod<T, int>::Type line)
{
	return alloc.allocate(size, fileName, line);
}

// If construction after placement new throws, this placement delete is being called.
template <typename T>
PX_INLINE void  operator delete(void* ptr, physx::shdfnd::ReflectionAllocator<T> alloc, const char* fileName, typename physx::shdfnd::EnableIfPod<T, int>::Type line)
{
	PX_UNUSED(fileName);
	PX_UNUSED(line);
	alloc.deallocate(ptr);
}

// If construction after placement new throws, this placement delete is being called.
template <typename T>
PX_INLINE void  operator delete[](void* ptr, physx::shdfnd::ReflectionAllocator<T> alloc, const char* fileName, typename physx::shdfnd::EnableIfPod<T, int>::Type line)
{
	PX_UNUSED(fileName);
	PX_UNUSED(line);
	alloc.deallocate(ptr);
}

#endif
