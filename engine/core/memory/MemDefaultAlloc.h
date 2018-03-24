#pragma once

#include <engine/core/Memory/MemDef.h>
#include <algorithm>

#if ECHO_MEMORY_ALLOCATOR==ECHO_MEMORY_ALLOCATOR_DEFAULT

namespace Echo
{
	class DefaultImpl
	{
	public:
		static void*	allocBytes(size_t count, const char* file, int line, const char* func);
        static void*    reallocBytes( void* Ptr, size_t NewSize, const char* file = 0, int line = 0, const char* func = 0);
		static void		deallocBytes(void* ptr);
		static void*	allocBytesAligned(size_t align, size_t count, const char* file, int line, const char* func);
		static void		deallocBytesAligned(size_t align, void* ptr);
	};
    
	class DefaultImplNoMemTrace
	{
	public:
		static void*	allocBytes(size_t count);
		static void		deallocBytes(void* ptr);
        static void*    reallocBytes( void* Ptr, size_t NewSize);
		static void*	allocBytesAligned(size_t align, size_t count);
		static void		deallocBytesAligned(size_t align, void* ptr);
	};
    
	class DefaultPolicy
	{
	public:
		static inline void* allocateBytes(size_t count, const char* file = NULL, int line = 0, const char* func = NULL)
		{
			return DefaultImpl::allocBytes(count, file, line, func);
		}
        static inline void* reallocBytes( void* ptr, size_t count, const char* file = 0, int line = 0, const char* func = 0)
        {
            return DefaultImpl::reallocBytes(ptr, count, file, line, func);
        }
		static inline void deallocateBytes(void* ptr)
		{
			DefaultImpl::deallocBytes(ptr);
		}
		// Get the maximum size of a single allocation
		static inline size_t getMaxAllocationSize()
		{
			return (std::numeric_limits<size_t>::max)();
		}
        
	private:
		// No instantiation
		DefaultPolicy()
		{
		}
	};
    
	class DefaultNoMemTracePolicy
	{
	public:
		static inline void* allocateBytes(size_t count)
		{
			return DefaultImplNoMemTrace::allocBytes(count);
		}
        static inline void* reallocBytes( void* ptr, size_t count)
        {
            return DefaultImplNoMemTrace::reallocBytes(ptr, count);
        }
		static inline void deallocateBytes(void* ptr)
		{
			DefaultImplNoMemTrace::deallocBytes(ptr);
		}
		// Get the maximum size of a single allocation
		static inline size_t getMaxAllocationSize()
		{
			return (std::numeric_limits<size_t>::max)();
		}
        
	private:
		// No instantiation
		DefaultNoMemTracePolicy()
		{
		}
	};
    
    template <size_t Alignment = 0>
	class DefaultAlignedPolicy
	{
	public:
		// compile-time check alignment is available.
		typedef int IsValidAlignment[Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];
        
		static inline void* allocateBytes(size_t count, const char* file = NULL, int line = 0, const char* func = NULL)
		{
			return DefaultImpl::allocBytesAligned(Alignment, count, file, line, func);
		}
        static inline void* reallocBytesAligned( void* ptr, size_t count, const char* file = 0, int line = 0, const char* func = 0)
        {
            return DefaultImpl::reallocBytes(ptr, count);
        }
		static inline void deallocateBytes(void* ptr)
		{
			DefaultImpl::deallocBytesAligned(Alignment, ptr);
		}
        
		// Get the maximum size of a single allocation
		static inline size_t getMaxAllocationSize()
		{
			return (std::numeric_limits<size_t>::max)();
		}
	private:
		// no instantiation allowed
		DefaultAlignedPolicy()
		{
		}
	};
    
	template <size_t Alignment = 0>
	class DefaultAlignedNoMemTracePolicy
	{
	public:
		// compile-time check alignment is available.
		typedef int IsValidAlignment[Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];
        
		static inline void* allocateBytes(size_t count)
		{
			return DefaultImplNoMemTrace::allocBytesAligned(Alignment, count);
		}
        static inline void* reallocBytesAligned( void* ptr, size_t count)
        {
            return DefaultImplNoMemTrace::reallocBytes(ptr, count);
        }
		static inline void deallocateBytes(void* ptr)
		{
			DefaultImplNoMemTrace::deallocBytesAligned(Alignment, ptr);
		}
        
		// Get the maximum size of a single allocation
		static inline size_t getMaxAllocationSize()
		{
			return (std::numeric_limits<size_t>::max)();
		}
	private:
		// no instantiation allowed
		DefaultAlignedNoMemTracePolicy()
		{
		}
	};
}
#endif
