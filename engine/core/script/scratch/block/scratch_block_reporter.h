#pragma once

#include "scratch_block.h"

namespace Echo
{
	// https://en.scratch-wiki.info/wiki/Reporter_Block
	class ScratchBlockReporter : public ScratchBlock
	{
	public:
		ScratchBlockReporter() {}
		virtual ~ScratchBlockReporter() {}
	};
}