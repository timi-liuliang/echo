#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	class Command : public Object
	{
		ECHO_CLASS(Command, Object)

	public:
		Command();
		virtual ~Command();

		// exec
		virtual bool exec(const StringArray& args) { return false; }
	};
}