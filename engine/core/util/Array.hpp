/* The following code declares class array,
 * an STL container (as wrapper) for arrays of constant size.
 *
 * See
 *      http://www.boost.org/libs/array/
 * for documentation.
 *
 * The original author site is at: http://www.josuttis.com/
 *
 * (C) Copyright Nicolai M. Josuttis 2001.
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * 14 Apr 2012 - (mtc) Added support for boost::hash
 * 28 Dec 2010 - (mtc) Added cbegin and cend (and crbegin and crend) for C++Ox compatibility.
 * 10 Mar 2010 - (mtc) fill method added, matching resolution of the standard library working group.
 *      See <http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#776> or Trac issue #3168
 *      Eventually, we should remove "assign" which is now a synonym for "fill" (Marshall Clow)
 * 10 Mar 2010 - added workaround for SUNCC and !STLPort [trac #3893] (Marshall Clow)
 * 29 Jan 2004 - c_array() added, BOOST_NO_PRIVATE_IN_AGGREGATE removed (Nico Josuttis)
 * 23 Aug 2002 - fix for Non-MSVC compilers combined with MSVC libraries.
 * 05 Aug 2001 - minor update (Nico Josuttis)
 * 20 Jan 2001 - STLport fix (Beman Dawes)
 * 29 Sep 2000 - Initial Revision (Nico Josuttis)
 *
 * Jan 29, 2004
 */
#ifndef BOOST_ARRAY_HPP
#define BOOST_ARRAY_HPP

#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include "engine/core/util/AssertX.h"
#include "engine/core/util/Exception.h"

namespace Echo {

	template<class T, std::size_t N>
	class array {
	public:
		T elems[N];    // fixed-size array of elements of type T

	public:
		// type definitions
		typedef T              value_type;
		typedef T*             iterator;
		typedef const T*       const_iterator;
		typedef T&             reference;
		typedef const T&       const_reference;
		typedef std::size_t    size_type;
		typedef std::ptrdiff_t difference_type;

		// iterator support
		iterator        begin()       { return elems; }
		const_iterator  begin() const { return elems; }
		const_iterator cbegin() const { return elems; }

		iterator        end()       { return elems+N; }
		const_iterator  end() const { return elems+N; }
		const_iterator cend() const { return elems+N; }

		// reverse iterator support
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		reverse_iterator rbegin() { return reverse_iterator(end()); }

		const_reverse_iterator rbegin() const 
		{
			return const_reverse_iterator(end());
		}

		const_reverse_iterator crbegin() const 
		{
			return const_reverse_iterator(end());
		}

		reverse_iterator rend() { return reverse_iterator(begin()); }

		const_reverse_iterator rend() const
		{
			return const_reverse_iterator(begin());
		}

		const_reverse_iterator crend() const 
		{
			return const_reverse_iterator(begin());
		}

		// operator[]
		reference operator[](size_type i) 
		{ 
			EchoAssertX( i < N, "out of range" );
			return elems[i];
		}

		const_reference operator[](size_type i) const 
		{     
			EchoAssertX( i < N, "out of range" );
			return elems[i]; 
		}

		// at() with range check
		reference at(size_type i) { rangecheck(i); return elems[i]; }

		const_reference at(size_type i) const { rangecheck(i); return elems[i]; }

		// front() and back()
		reference front() 
		{ 
			return elems[0]; 
		}

		const_reference front() const 
		{
			return elems[0];
		}

		reference back() 
		{ 
			return elems[N-1]; 
		}

		const_reference back() const 
		{ 
			return elems[N-1]; 
		}

		// size is constant
		static size_type size() { return N; }
		static bool empty() { return false; }
		static size_type max_size() { return N; }
		enum { static_size = N };

		// swap (note: linear complexity)
		void swap(array<T, N>& y);

		// direct access to data (read-only)
		const T* data() const { return elems; }
		T* data() { return elems; }

		// use array as C array (direct read/write access to data)
		T* c_array() { return elems; }

		// assignment with type conversion
		template <typename T2>
		array<T,N>& operator= (const array<T2,N>& rhs) 
		{
			std::copy(rhs.begin(),rhs.end(), begin());
			return *this;
		}

		// assign one value to all elements
		void assign (const T& value) { fill ( value ); }    // A synonym for fill
		void fill   (const T& value)
		{
			std::fill_n(begin(),size(),value);
		}

		// check range (may be private because it is static)
		static void rangecheck (size_type i) 
		{
			if (i >= size()) 
			{
				std::out_of_range e("array<>: index out of range");
			}
		}

	};

	template< class T >
	class array< T, 0 > {

	public:
		// type definitions
		typedef T              value_type;
		typedef T*             iterator;
		typedef const T*       const_iterator;
		typedef T&             reference;
		typedef const T&       const_reference;
		typedef std::size_t    size_type;
		typedef std::ptrdiff_t difference_type;

		// iterator support
		iterator        begin()       { return       iterator( reinterpret_cast<       T * >( this ) ); }
		const_iterator  begin() const { return const_iterator( reinterpret_cast< const T * >( this ) ); }
		const_iterator cbegin() const { return const_iterator( reinterpret_cast< const T * >( this ) ); }

		iterator        end()       { return  begin(); }
		const_iterator  end() const { return  begin(); }
		const_iterator cend() const { return cbegin(); }

		// reverse iterator support
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		reverse_iterator rbegin() { return reverse_iterator(end()); }

		const_reverse_iterator rbegin() const 
		{
			return const_reverse_iterator(end());
		}

		const_reverse_iterator crbegin() const 
		{
			return const_reverse_iterator(end());
		}

		reverse_iterator rend() { return reverse_iterator(begin()); }

		const_reverse_iterator rend() const 
		{
			return const_reverse_iterator(begin());
		}

		const_reverse_iterator crend() const 
		{
			return const_reverse_iterator(begin());
		}

		// operator[]
		reference operator[](size_type /*i*/)
		{
			return failed_rangecheck();
		}

		const_reference operator[](size_type /*i*/) const
		{
			return failed_rangecheck();
		}

		// at() with range check
		reference at(size_type /*i*/)               {   return failed_rangecheck(); }
		const_reference at(size_type /*i*/) const   {   return failed_rangecheck(); }

		// front() and back()
		reference front()
		{
			return failed_rangecheck();
		}

		const_reference front() const
		{
			return failed_rangecheck();
		}

		reference back()
		{
			return failed_rangecheck();
		}

		const_reference back() const
		{
			return failed_rangecheck();
		}

		// size is constant
		static size_type size() { return 0; }
		static bool empty() { return true; }
		static size_type max_size() { return 0; }
		enum { static_size = 0 };

		void swap (array<T,0>& /*y*/) 
		{
		}

		// direct access to data (read-only)
		const T* data() const { return 0; }
		T* data() { return 0; }

		// use array as C array (direct read/write access to data)
		T* c_array() { return 0; }

		// assignment with type conversion
		template <typename T2>
		array<T,0>& operator= (const array<T2,0>& ) 
		{
			return *this;
		}

		// assign one value to all elements
		void assign (const T& value) { fill ( value ); }
		void fill   (const T& ) {}

		// check range (may be private because it is static)
		static reference failed_rangecheck () {
			// std::out_of_range e("attempt to access element of an empty array");

			Echo::EchoException("attempt to access element of an empty array");
			//
			// We need to return something here to keep
			// some compilers happy: however we will never
			// actually get here....
			//
			// static T placeholder;
			// return placeholder;
		}
	};


	// comparisons
	template<class T, std::size_t N>
	bool operator== (const array<T,N>& x, const array<T,N>& y) 
	{
		return std::equal(x.begin(), x.end(), y.begin());
	}

	template<class T, std::size_t N>
	bool operator< (const array<T,N>& x, const array<T,N>& y) 
	{
		return std::lexicographical_compare(x.begin(),x.end(),y.begin(),y.end());
	}

	template<class T, std::size_t N>
	bool operator!= (const array<T,N>& x, const array<T,N>& y) 
	{
		return !(x==y);
	}

	template<class T, std::size_t N>
	bool operator> (const array<T,N>& x, const array<T,N>& y) 
	{
		return y<x;
	}

	template<class T, std::size_t N>
	bool operator<= (const array<T,N>& x, const array<T,N>& y) 
	{
		return !(y<x);
	}

	template<class T, std::size_t N>
	bool operator>= (const array<T,N>& x, const array<T,N>& y) 
	{
		return !(x<y);
	}

	// global swap()
	template<class T, std::size_t N>
	inline void swap (array<T,N>& x, array<T,N>& y) 
	{
		x.swap(y);
	}

	// Specific for boost::array: simply returns its elems data member.
	template <typename T, std::size_t N>
	T(&get_c_array(Echo::array<T,N>& arg))[N]
	{
		return arg.elems;
	}

	// Const version.
	template <typename T, std::size_t N>
	const T(&get_c_array(const Echo::array<T,N>& arg))[N]
	{
		return arg.elems;
	}

	//for Clang: Clang can not parse whole template file
	template <typename T, size_t N>
	void array<T, N>::swap(array<T, N>& y)
	{
		for (size_type i = 0; i < N; ++i)
			Echo::swap(elems[i], y.elems[i]);
	}
} /* namespace Echo */

#endif /*BOOST_ARRAY_HPP*/
