#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class PCGData
	{
	public:
		PCGData();
		virtual ~PCGData();

		// Type
		virtual String getType() { return "UnKnown"; }
	};
}
