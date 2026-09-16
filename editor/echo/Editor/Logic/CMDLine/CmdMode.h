#pragma once

#include <engine/core/util/StringUtil.h>

namespace Echo
{
	/**
	 * CmdMode - run a Terminal command from the command line, without a window.
	 *
	 *   Echo.exe cmd <command> [args...]
	 *
	 * This is the command line front end of the shared command layer: the editor GUI is
	 * expected to drive the very same Command instances, so a human clicking a button and
	 * an agent running a command always take one code path and produce one result.
	 *
	 * It never creates a QApplication, a window or a renderer, so it also works in CI.
	 */
	class CmdMode
	{
	public:
		// Exec command. Returns the process exit code (0 = success).
		int exec(int argc, char* argv[]);

	private:
		// usage text
		static String usage();

		// list all registered commands, so that a typo explains itself
		static void listCommands();
	};
}