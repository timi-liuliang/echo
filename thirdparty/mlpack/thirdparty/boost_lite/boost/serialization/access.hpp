#ifndef BOOST_SERIALIZATION_ACCESS_HPP
#define BOOST_SERIALIZATION_ACCESS_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// access.hpp: interface for serialization system.

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

namespace boost {
namespace serialization {

// use an "accessor class so that we can use: 
// "friend class boost::serialization::access;" 
// in any serialized class to permit clean, safe access to private class members
// by the serialization system

class access {
public:
};
}
}

#endif
