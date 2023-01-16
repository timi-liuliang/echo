#include <stdio.h>
#include "Cmd/CMDLine.h"

namespace Echo
{
	void registerModules()
	{

	}
}

int main(int argc, char* argv[])
{
	// parse & run
	Echo::CMDLine::Parser(argc, argv);
}
