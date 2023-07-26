#pragma once

#include <engine/core/util/StringUtil.h>

namespace Echo
{
	/**
	 * EditorMode
	 */
	class EditorMode
	{
	public:
		// Exec command
		bool exec(int argc, char* argv[]);
	};
}