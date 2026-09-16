#include "CmdMode.h"

#include <stdio.h>
#include <engine/core/base/class.h>
#include <engine/core/io/io.h>
#include <engine/core/main/engine.h>
#include <engine/core/terminal/terminal.h>

#ifdef ECHO_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace Echo
{
	// process exit codes of `Echo.exe cmd`
	enum CmdExitCode
	{
		CmdExitSuccess = 0,
		CmdExitCommandFailed = 1,		// a command was found and run, but reported failure
		CmdExitNoCommand = 2,			// no command name given
		CmdExitUnknownCommand = 3,		// no such command
	};

	int CmdMode::exec(int argc, char* argv[])
	{
		// argv[0] = exe, argv[1] = "cmd", argv[2] = command name, argv[3..] = arguments.
		// Arguments are forwarded untouched (never re-split on spaces), so a path such as
		// "D:/My Projects/game" survives.
		StringArray args;
		for (int i = 2; i < argc; i++)
			args.emplace_back(argv[i]);

		// Echo.exe is a windows subsystem binary, so it owns no console by default: when it
		// is started from a terminal its output would be dropped and a human (or an agent)
		// could not read it. Borrow the console of whoever started us, so `echo.exe cmd`
		// behaves like an ordinary command line tool.
		//
		// Only when stdout is NOT already redirected (e.g. `... > build.log`): in that case
		// the caller picked a destination and we must not steal it.
#ifdef ECHO_PLATFORM_WINDOWS
		HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!stdoutHandle || stdoutHandle == INVALID_HANDLE_VALUE ||
			GetFileType(stdoutHandle) == FILE_TYPE_UNKNOWN)
		{
			if (AttachConsole(ATTACH_PARENT_PROCESS))
			{
				FILE* dummy;
				freopen_s(&dummy, "CONOUT$", "w", stdout);
				freopen_s(&dummy, "CONOUT$", "w", stderr);
			}
		}
#endif

		if (args.empty())
		{
			printf("%s\n", usage().c_str());
			listCommands();

			return CmdExitNoCommand;
		}

		// Required by the registration below (it instantiates singletons and touches IO).
		IO::instance();

		// Pure type registration. Brings up the reflection table without reading a project
		// file, creating a window or touching the GPU - that is what makes a headless
		// command line possible at all.
		Engine::registerTypeInfos();

		// Dispatch through the shared command layer.
		bool succeeded = false;
		if (!Terminal::instance()->execCmd(args, &succeeded))
		{
			printf("cmd: unknown command [%s]\n", args[0].c_str());
			listCommands();

			return CmdExitUnknownCommand;
		}

		if (!succeeded)
		{
			printf("cmd: command [%s] failed\n", args[0].c_str());

			return CmdExitCommandFailed;
		}

		return CmdExitSuccess;
	}

	String CmdMode::usage()
	{
		return "usage: Echo.exe cmd <command> [args...]";
	}

	void CmdMode::listCommands()
	{
		StringArray commandClasses;
		Class::getChildClasses(commandClasses, ECHO_CLASS_NAME(Command), true);

		String names;
		for (const String& className : commandClasses)
		{
			if (!names.empty())
				names += ", ";

			names += StringUtil::Replace(className, "Command", "");
		}

		printf("cmd: available commands: %s\n", names.empty() ? "(none)" : names.c_str());
	}
}
