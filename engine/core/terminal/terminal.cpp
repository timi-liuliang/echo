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

	bool Terminal::execCmd()
	{
		return false;
	}
}