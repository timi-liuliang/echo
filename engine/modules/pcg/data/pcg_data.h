#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/resource/ResRef.h"

namespace Echo
{
	class PCGData : public Refable
	{
	public:
		PCGData();
		virtual ~PCGData();

		// Type
		virtual String getType() { return "UnKnown"; }
	};
	typedef ResRef<PCGData> PCGDataPtr;
}
