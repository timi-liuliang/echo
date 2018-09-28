// boost heap: helper classes for stable priority queues
//
// Copyright (C) 2010 Tim Blechmann
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_HEAP_DETAIL_STABLE_HEAP_HPP
#define BOOST_HEAP_DETAIL_STABLE_HEAP_HPP

#include <boost/throw_exception.hpp>

#include <boost/heap/policies.hpp>

namespace boost  {
namespace heap   {
namespace detail {

template <typename Alloc>
struct extract_allocator_types
{
	typedef typename Alloc::size_type size_type;
	typedef typename Alloc::difference_type difference_type;
	typedef typename Alloc::reference reference;
	typedef typename Alloc::const_reference const_reference;
	typedef typename Alloc::pointer pointer;
	typedef typename Alloc::const_pointer const_pointer;
};

} /* namespace detail */
} /* namespace heap */
} /* namespace boost */

#endif /* BOOST_HEAP_DETAIL_STABLE_HEAP_HPP */