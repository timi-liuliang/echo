#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/memory/MemAllocDef.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class ObjectOperation
	{
	public:
		ObjectOperation() {}
		virtual ~ObjectOperation() {}

		// tick
		virtual void tick(const set<Echo::ui32>::type& objects) {}
	};
}
#endif