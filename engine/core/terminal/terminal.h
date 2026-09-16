#pragma once

#include "command.h"

namespace Echo
{
	/**
	 * Terminal - some times we don't need make a ui for all functional.
	 * We use terminal singleton to run command
	 */
	class Terminal : public Object
	{
		ECHO_SINGLETON_CLASS(Terminal, Object)

	public:
		virtual ~Terminal();

		// instance
		static Terminal* instance();

		// execute command. The command line is split on spaces, so an argument can not
		// contain a space - use the StringArray overload when passing file paths.
		bool execCmd(const String& cmd);

		// execute command from already split arguments. Nothing is re-split here, so an
		// argument may contain spaces (e.g. D:/My Projects/game). Command line front ends
		// such as echo.exe cmd ... forward their argv through this overload.
		//
		// On success (return value true = a matching command was found and run) the
		// optional out parameter receives the command's own result, so a failing command
		// can be turned into a non-zero process exit code instead of being swallowed.
		bool execCmd(const StringArray& args, bool* succeeded = nullptr);

	private:
		Terminal();
	};
}