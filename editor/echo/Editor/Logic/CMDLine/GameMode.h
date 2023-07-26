#pragma once

#include <engine/core/util/StringUtil.h>

namespace Echo
{
	/**
	 * GameMode
	 */
	class GameMode
	{
	public:
		// Exec command
		bool exec(int argc, char* argv[]);
	};
}