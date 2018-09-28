// boost heap: wrapper for stl heap
//
// Copyright (C) 2010 Tim Blechmann
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_HEAP_PRIORITY_QUEUE_HPP
#define BOOST_HEAP_PRIORITY_QUEUE_HPP

#include <algorithm>
#include <queue>
#include <utility>
#include <vector>

#include <boost/assert.hpp>

#include <boost/heap/detail/stable_heap.hpp>

namespace boost {
namespace heap  {
namespace detail {

}

/**
 * \class priority_queue
 * \brief priority queue, based on stl heap functions
 *
 * The priority_queue class is a wrapper for the stl heap functions.<br>
 * The template parameter T is the type to be managed by the container.
 * The user can specify additional options and if no options are provided default options are used.
 *
 * The container supports the following options:
 * - \c boost::heap::compare<>, defaults to \c compare<std::less<T> >
 * - \c boost::heap::stable<>, defaults to \c stable<false>
 * - \c boost::heap::stability_counter_type<>, defaults to \c stability_counter_type<boost::uintmax_t>
 * - \c boost::heap::allocator<>, defaults to \c allocator<std::allocator<T> >
 *
 */
#ifndef BOOST_DOXYGEN_INVOKED
	template<class T, class ...Options>
#else
	template <typename T,
		class A0 /*= boost::parameter::void_*/,
		class A1 /*= boost::parameter::void_*/,
		class A2 /*= boost::parameter::void_*/,
		class A3 /*= boost::parameter::void_*/
	>
#endif
class priority_queue
{

#ifndef BOOST_DOXYGEN_INVOKED
	struct implementation_defined// :
	//	detail::extract_allocator_types<typename heap_base_maker::allocator_argument>
	{
	};
#endif

public:
	typedef T value_type;
	typedef size_t size_type;
	//typedef typename implementation_defined::const_reference const_reference;

public:

	/**
	 * \b Effects: Returns true, if the priority queue contains no elements.
	 *
	 * \b Complexity: Constant.
	 *
	 * */
	bool empty(void) const BOOST_NOEXCEPT
	{
		return false;
	}

	/**
	 * \b Effects: Returns the number of elements contained in the priority queue.
	 *
	 * \b Complexity: Constant.
	 *
	 * */
	size_type size(void) const BOOST_NOEXCEPT
	{
		return 0;
	}

	/**
	* \b Effects: Returns a const_reference to the maximum element.
	*
	* \b Complexity: Constant.
	*
	* */
	//const_reference top(void) const
	//{
	//	BOOST_ASSERT(!empty());
	//	return super_t::get_value(q_.front());
	//}

	/**
	 * \b Effects: Adds a new element to the priority queue.
	 *
	 * \b Complexity: Logarithmic (amortized). Linear (worst case).
	 *
	 * */
	void push(value_type const & v)
	{
		//q_.push_back(super_t::make_node(v));
		//std::push_heap(q_.begin(), q_.end(), static_cast<super_t const &>(*this));
	}
};

} /* namespace heap */
} /* namespace boost */

#endif /* BOOST_HEAP_PRIORITY_QUEUE_HPP */