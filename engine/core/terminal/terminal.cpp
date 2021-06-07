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
		StringArray args = StringUtil::Split(cmd, " ");
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
						commandObj->exec(args);

						EchoSafeDelete(commandObj, Command);
						return true;
					}
				}
			}

		}

		return false;
	}
}