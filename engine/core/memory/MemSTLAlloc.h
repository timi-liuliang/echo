#ifndef _MEM_STL_ALLOC_H__
#define _MEM_STL_ALLOC_H__

#include <memory> // for std::allocator<> and std::ptrdiff_t
#include "MemAllocDef.h"

namespace Echo
{

	// Base STL allocator class.
	template<typename T>
	struct STLAllocatorBase
	{	// base class for generic allocators
		typedef T value_type;
	};

	// Base STL allocator class. (const T version).
	template<typename T>
	struct STLAllocatorBase<const T>
	{	// base class for generic allocators for const T
		typedef T value_type;
	};

	template
		<
		typename T,
		typename AllocPolicy
		>
	class SA : public STLAllocatorBase<T>
	{
	public :
		/// define our types, as per ISO C++
		typedef STLAllocatorBase<T>			Base;
		typedef typename Base::value_type	value_type;
		typedef value_type*					pointer;
		typedef const value_type*			const_pointer;
		typedef value_type&					reference;
		typedef const value_type&			const_reference;
		typedef std::size_t					size_type;
		typedef std::ptrdiff_t				difference_type;


		/// the standard rebind mechanism
		template<typename U>
		struct rebind
		{
			typedef SA<U, AllocPolicy> other;
		};

		/// ctor
		inline explicit SA()
		{ }

		/// dtor
		virtual ~SA()
		{ }

		/// copy ctor - done component wise
		inline SA( SA const& )
		{ }

		/// cast
		template <typename U>
		inline SA( SA<U, AllocPolicy> const& )
		{ }

		/// cast
		template <typename U, typename P>
		inline SA( SA<U, P> const& )
		{ }

		/// memory allocation (elements, used by STL)
		inline pointer allocate( size_type count,
			typename std::allocator<void>::const_pointer ptr = 0 )
		{
			(void)ptr;
			// convert request to bytes
			size_type sz = count*sizeof( T );
			pointer p  = static_cast<pointer>(AllocPolicy::allocateBytes(sz));
			return p;
		}

		/// memory deallocation (elements, used by STL)
		inline void deallocate( pointer ptr, size_type )
		{
			// convert request to bytes, but we can't use this?
			// size_type sz = count*sizeof( T );
			AllocPolicy::deallocateBytes(ptr);
		}

		pointer address(reference x) const
		{
			return &x;
		}

		const_pointer address(const_reference x) const
		{
			return &x;
		}

		size_type max_size() const throw()
		{
			// maximum size this can handle, delegate
			return AllocPolicy::getMaxAllocationSize();
		}

		void construct(pointer p, const T& val)
		{
			// call placement new
			new(static_cast<void*>(p)) T(val);
		}

		void destroy(pointer p)
		{
			// do we have to protect against non-classes here?
			// some articles suggest yes, some no
			p->~T();
		}

	};

	/// determine equality, can memory from another allocator
	/// be released by this allocator, (ISO C++)
	template<typename T, typename T2, typename P>
	inline bool operator == (SA<T,P> const&,
		SA<T2,P> const&)
	{
		// same alloc policy (P), memory can be freed
		return true;
	}

	/// determine equality, can memory from another allocator
	/// be released by this allocator, (ISO C++)
	template<typename T, typename P, typename OtherAllocator>
	inline bool operator == (SA<T,P> const&, OtherAllocator const&)
	{
		return false;
	}
	/// determine equality, can memory from another allocator
	/// be released by this allocator, (ISO C++)
	template<typename T, typename T2, typename P>
	inline bool operator != (SA<T,P> const&, SA<T2,P> const&)
	{
		// same alloc policy (P), memory can be freed
		return false;
	}

	/// determine equality, can memory from another allocator
	/// be released by this allocator, (ISO C++)
	template<typename T, typename P, typename OtherAllocator>
	inline bool operator != (SA<T,P> const&, OtherAllocator const&)
	{
		return true;
	}

}

#endif