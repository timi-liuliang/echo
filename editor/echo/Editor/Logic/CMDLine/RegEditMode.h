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

	public:
		// Set default value
		static void setDefaultValue(const String& regPath, const String& value);

		// Add right menu
		static void addRightMenu(const String& shellPath, const String& name, const String& desc, const String& icon, const String& command);

		// Is registered
		static void check(const char* echoExe);
	};
}
