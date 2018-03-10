#include "MemDefaultAlloc.h"

#if ECHO_MEMORY_ALLOCATOR==ECHO_MEMORY_ALLOCATOR_DEFAULT

namespace Echo
{
	//////////////////////////////////////////////////////////////////////////
    
	void* DefaultImpl::allocBytes(size_t count, const char* file, int line, const char* func)
	{
		void* ptr = malloc(count);
#if ECHO_MEMORY_TRACKER
		MemTracker::get()._recordAlloc(ptr, count, 0, file, line, func);
#else
		// avoid unused params warning
		file = func = "";
		line = 0;
#endif
		return ptr;
	}

    void* DefaultImpl::reallocBytes( void* ptr, size_t count, const char* file, int line, const char* func)
    {
        void* ret = realloc(ptr, count);
#if ECHO_MEMORY_TRACKER
		MemTracker::get()._recordAlloc(ret, count, 0, file, line, func);
#else
		// avoid unused params warning
		file = func = "";
		line = 0;
#endif
		return ret;
    }
    
	void DefaultImpl::deallocBytes(void* ptr)
	{
		// deal with null
		if (!ptr)
			return;
#if ECHO_MEMORY_TRACKER
		MemTracker::get()._recordDealloc(ptr);
#endif
		free(ptr);
	}
    
	void* DefaultImpl::allocBytesAligned(size_t align, size_t count, const char* file, int line, const char* func)
	{
		// default to platform SIMD alignment if none specified
#if defined(ECHO_PLATFORM_WINDOWS)
		// on win32 we only have 8-byte alignment guaranteed, but the CRT provides special aligned allocation fns
		void * result = _aligned_malloc(count, align);
#elif defined(ECHO_PLATFORM_ANDROID)
		void * result = ::memalign(align, count);
#else
		// on all other platforms we get 16-byte alignment by default
		void * result = ::malloc(count);
#endif
#if ECHO_MEMORY_TRACKER
		MemTracker::get()._recordAlloc(ptr, count, 0, file, line, func);
#else
		// avoid unused params warning
		file = func = "";
		line = 0;
#endif
		return result;
	}

	void DefaultImpl::deallocBytesAligned(size_t align, void* ptr)
	{
		// deal with null
		if (!ptr)
			return;
#if ECHO_MEMORY_TRACKER
		MemTracker::get()._recordDealloc(ptr);
#endif
#if defined(ECHO_PLATFORM_WINDOWS)
		_aligned_free(ptr);
#elif defined(ECHO_PLATFORM_ANDROID)
		::free(ptr);
#else
		::free(ptr);
#endif
	}
    
	//////////////////////////////////////////////////////////////////////////
    
	void* DefaultImplNoMemTrace::allocBytes(size_t count)
	{
		return malloc(count);
	}

     void* DefaultImplNoMemTrace::reallocBytes( void* ptr, size_t count)
    {
        return realloc(ptr, count);
    }
    
	void DefaultImplNoMemTrace::deallocBytes(void* ptr)
	{
		// deal with null
		if (!ptr)
			return;
        
		free(ptr);
	}
    
	void* DefaultImplNoMemTrace::allocBytesAligned(size_t align, size_t count)
	{
#if defined(ECHO_PLATFORM_WINDOWS)
		// on win32 we only have 8-byte alignment guaranteed, but the CRT provides special aligned allocation fns
		return _aligned_malloc(count, align);
#elif defined(ECHO_PLATFORM_ANDROID)
		return ::memalign(align, count);
#else
		// on all other platforms we get 16-byte alignment by default
		return ::malloc(count);
#endif
	}
    
	void DefaultImplNoMemTrace::deallocBytesAligned(size_t align, void* ptr)
	{
		// deal with null
		if (!ptr)
			return;

#if defined(ECHO_PLATFORM_WINDOWS)
		_aligned_free(ptr);
#elif defined(ECHO_PLATFORM_ANDROID)
		::free(ptr);
#else
		::free(ptr);
#endif
	}
}

#endif
