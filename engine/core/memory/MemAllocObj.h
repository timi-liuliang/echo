#ifndef __ECHO_MEMALLOCOBJ_H__
#define __ECHO_MEMALLOCOBJ_H__

#include "engine/core/base/echo_def.h"

namespace Echo
{
	/** Superclass for all objects that wish to use custom memory allocators
	when their new / delete operators are called.
	Requires a template parameter identifying the memory allocator policy
	to use (e.g. see StdAllocPolicy).
	*/
	template <class Alloc>
	class AllocatedObject
	{
	public:
		explicit AllocatedObject()
		{
		}

		~AllocatedObject()
		{
		}

		// operator new, with debug line info
		void* operator new(size_t sz, const char *file, int line, const char *func)
		{
			return Alloc::allocateBytes(sz, file, line, func);
		}

		void* operator new(size_t sz)
		{
			return Alloc::allocateBytes(sz);
		}

		/// placement operator new
		void* operator new(size_t sz, void *ptr)
		{
			(void) sz;
			return ptr;
		}

		/// array operator new, with debug line info
		void* operator new[] ( size_t sz, const char *file, int line, const char *func )
		{
			return Alloc::allocateBytes(sz, file, line, func);
		}

		void* operator new[] ( size_t sz )
		{
			return Alloc::allocateBytes(sz);
		}

		void operator delete( void *ptr )
		{
			Alloc::deallocateBytes(ptr);
		}

		// Corresponding operator for placement delete (second param same as the first)
		void operator delete( void *ptr, void* )
		{
			Alloc::deallocateBytes(ptr);
		}

		// only called if there is an exception in corresponding 'new'
		void operator delete( void *ptr, const char*, int , const char* )
		{
			Alloc::deallocateBytes(ptr);
		}

		void operator delete[] ( void *ptr )
		{
			Alloc::deallocateBytes(ptr);
		}


		void operator delete[] ( void* ptr, const char* , int , const char*  )
		{
			Alloc::deallocateBytes(ptr);
		}
	};
}

#endif
