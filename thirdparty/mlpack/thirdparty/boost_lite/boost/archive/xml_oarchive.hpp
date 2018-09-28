#pragma once

#include <ostream>
#include <cstddef>

namespace boost {
namespace archive {

class xml_oarchive
{
public:
	xml_oarchive(std::ostream& os, unsigned int flags = 0)
	{}
	~xml_oarchive(){}
};

} // namespace archive
} // namespace boost