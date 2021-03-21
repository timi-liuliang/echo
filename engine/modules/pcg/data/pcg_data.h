#pragma once

#include "engine/core/memory/MemAllocDef.h"

class PCGData
{
public:
	PCGData();
	virtual ~PCGData();

	// Type
	virtual const char* getType() { return "UnKnown"; }
};