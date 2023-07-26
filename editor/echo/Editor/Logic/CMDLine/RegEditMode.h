#pragma once

#include <engine/core/util/StringUtil.h>

namespace Echo
{
	/**
	 * Link echo file
	 */
	class RegEditMode
	{
	public:
		// Exec command
		bool exec(int argc, char* argv[]);
	};
}