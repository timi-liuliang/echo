#pragma once

#include "MemDef.h"
#include <limits>
#include <algorithm>

#if ECHO_MEMORY_ALLOCATOR  == ECHO_MEMORY_ALLOCATOR_BINNED

#include <stddef.h>
#include <new>

namespace memx { class memx; }

namespace Echo
{
	memx::memx& memx();
	void release_memx();

    enum { DEFAULT_ALIGNMENT = 0 };
	class MallocInterface;
    class MallocBinnedMgr
    {
    public:
        static void* Malloc( size_t count, unsigned int aligment = DEFAULT_ALIGNMENT);
        static void* Realloc( void* Ptr, size_t NewSize, unsigned int Alignment = DEFAULT_ALIGNMENT);
        static void Free(void* ptr);
		static void CheckLeak();
		static MallocInterface* CreateInstance();
		static void ReleaseInstance();
		static void ReplaceInstance(MallocInterface* mallocInterface);
    };

    class BinnedAllocPolicy
    {
    public:
        static inline void* allocateBytes(size_t count, const char* file = 0, int line = 0, const char* func = 0)
        {
            return MallocBinnedMgr::Malloc(count);
        }
        
        static inline void* reallocBytes( void* Ptr, size_t NewSize, const char* file = 0, int line = 0, const char* func = 0)
        {
            return MallocBinnedMgr::Realloc(Ptr, NewSize);
        }

        static inline void deallocateBytes(void* ptr)
        {
            MallocBinnedMgr::Free(ptr);
        }
        /// Get the maximum size of a single allocation
        static inline size_t getMaxAllocationSize()
        {
            return (std::numeric_limits<size_t>::max)();
        }
    private:
        // No instantiation
        BinnedAllocPolicy()
        { }
    };

    template <size_t Alignment = 0>
    class BinnedAlignedAllocPolicy
    {
    public:
        // compile-time assert alignment is available.
        typedef int IsValidAlignment
            [Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

        static inline void* allocateBytes(size_t count,
            const char* file = 0, int line = 0, const char* func = 0)
        {
            return MallocBinnedMgr::Malloc(count, Alignment);
        }

        static inline void* reallocBytes( void* Ptr, size_t NewSize, const char* file = 0, int line = 0, const char* func = 0)
        {
            return MallocBinnedMgr::Realloc(Ptr, NewSize, Alignment);
        }

        static inline void deallocateBytes(void* ptr)
        {
            MallocBinnedMgr::Free(ptr);
        }

        /// Get the maximum size of a single allocation
        static inline size_t getMaxAllocationSize()
        {
            return (std::numeric_limits<size_t>::max)();
        }
    private:
        // no instantiation allowed
        BinnedAlignedAllocPolicy()
        { }
    };

    class BinnedAllocPolicyNoMemTrace
    {
    public:
        static inline void* allocateBytes(size_t count,
            const char* file = 0, int line = 0, const char* func = 0)
        {
            return MallocBinnedMgr::Malloc(count);
        }
        static inline void* reallocBytes( void* Ptr, size_t NewSize, const char* file = 0, int line = 0, const char* func = 0)
        {
            return MallocBinnedMgr::Realloc(Ptr, NewSize);
        }
        static inline void deallocateBytes(void* ptr)
        {
            MallocBinnedMgr::Free(ptr);
        }
        /// Get the maximum size of a single allocation
        static inline size_t getMaxAllocationSize()
        {
            return (std::numeric_limits<size_t>::max)();
        }
    private:
        // No instantiation
        BinnedAllocPolicyNoMemTrace()
        { }
    };

    template <size_t Alignment = 0>
    class BinnedAlignedAllocPolicyNoMemTrace
    {
    public:
        // compile-time assert alignment is available.
        typedef int IsValidAlignment
            [Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

        static inline void* allocateBytes(size_t count,
            const char* file = 0, int line = 0, const char* func = 0)
        {
            return MallocBinnedMgr::Malloc(count, Alignment);
        }
        static inline void* reallocBytes( void* Ptr, size_t NewSize, const char* file = 0, int line = 0, const char* func = 0)
        {
            return MallocBinnedMgr::Realloc(Ptr, NewSize);
        }
        static inline void deallocateBytes(void* ptr)
        {
            MallocBinnedMgr::Free(ptr);
        }

        /// Get the maximum size of a single allocation
        static inline size_t getMaxAllocationSize()
        {
            return (std::numeric_limits<size_t>::max)();
        }
    private:
        // no instantiation allowed
        BinnedAlignedAllocPolicyNoMemTrace()
        { }
    };
}
#endif
