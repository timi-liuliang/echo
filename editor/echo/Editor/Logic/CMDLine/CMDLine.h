#pragma once

#include <engine/core/util/StringUtil.h>

namespace Echo
{
	/**
	 * Command Parser 2012-8-16 Liang
	 */
	class CMDLine
	{
	public:
		// Parse. Returns the process exit code (0 = success): GUI modes always return 0,
		// while cmd reports the outcome of the command it ran.
		static int Parser(int argc, char* argv[]);
	};
}