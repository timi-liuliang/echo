#include "Any.hpp"
#include "engine/core/log/Log.h"

namespace Echo
{
	// error
	void any::error(const char* msg)
	{
		EchoLogError(msg);
	}
}