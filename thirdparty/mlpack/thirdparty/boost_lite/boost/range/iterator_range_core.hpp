// Boost.Range library
//
//  Copyright Neil Groves & Thorsten Ottosen & Pavol Droba 2003-2004.
//  Use, modification and distribution is subject to the Boost Software
//  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
// Credits:
// 'michel' reported Trac 9072 which included a patch for allowing references
// to function types.
//
#ifndef BOOST_RANGE_ITERATOR_RANGE_CORE_HPP_INCLUDED
#define BOOST_RANGE_ITERATOR_RANGE_CORE_HPP_INCLUDED

namespace boost
{
	//  iterator range template class -----------------------------------------//

		//! iterator_range class
		/*!
			An \c iterator_range delimits a range in a sequence by beginning and ending iterators.
			An iterator_range can be passed to an algorithm which requires a sequence as an input.
			For example, the \c toupper() function may be used most frequently on strings,
			but can also be used on iterator_ranges:

			\code
				boost::tolower( find( s, "UPPERCASE STRING" ) );
			\endcode

			Many algorithms working with sequences take a pair of iterators,
			delimiting a working range, as an arguments. The \c iterator_range class is an
			encapsulation of a range identified by a pair of iterators.
			It provides a collection interface,
			so it is possible to pass an instance to an algorithm requiring a collection as an input.
		*/
	template<class IteratorT>
	class iterator_range
	{

	};
}

#endif
