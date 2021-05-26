#pragma once

#include "scratch_block.h"

namespace Echo
{
	// https://en.scratch-wiki.info/wiki/Stack_Block
	class ScratchBlockStack : public ScratchBlock
	{
	public:
		ScratchBlockStack() {}
		virtual ~ScratchBlockStack() {}
	};
}