#include "App.h"
#include <engine/core/util/PathUtil.h>

int main(int argc, char* argv[])
{
	Echo::String currentPath = Echo::PathUtil::GetFileDirPath(argv[0]);
	SetCurrentDirectory(currentPath.c_str());

	Echo::App app;
	app.start();

	return EXIT_SUCCESS;
}
