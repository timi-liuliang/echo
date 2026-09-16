#include "terminal.h"

namespace Echo
{
	Terminal::Terminal()
	{

	}

	Terminal::~Terminal()
	{

	}

	Terminal* Terminal::instance()
	{
		static Terminal* inst = EchoNew(Terminal);
		return inst;
	}

	void Terminal::bindMethods()
	{

	}

	bool Terminal::execCmd(const String& cmd)
	{
		return execCmd(StringUtil::Split(cmd, " "), nullptr);
	}

	bool Terminal::execCmd(const StringArray& args, bool* succeeded)
	{
		if (succeeded)
			*succeeded = false;

		if (!args.empty())
		{
			StringArray commandClasses;
			Class::getChildClasses(commandClasses, ECHO_CLASS_NAME(Command), true);

			for (const String& className : commandClasses)
			{
				String prefix = StringUtil::Replace(className, "Command", "");
				if (StringUtil::Equal(prefix, args[0], false))
				{
					Command* commandObj = ECHO_DOWN_CAST<Command*>(Class::create(className));
					if (commandObj)
					{
						// keep the command own result: a failing command must not look like success
						bool ok = commandObj->exec(args);
						if (succeeded)
							*succeeded = ok;

						EchoSafeDelete(commandObj, Command);
						return true;
					}
				}
			}

		}

		return false;
	}
}