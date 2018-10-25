#include "App.h"
#include <engine/core/util/PathUtil.h>

int main(int argc, char* argv[])
{
	Echo::String currentPath = Echo::PathUtil::GetFileDirPath(argv[0]);
	SetCurrentDirectory(currentPath.c_str());

	if (argc > 1)
	{
		Echo::String projectFile = argv[1];
		Echo::String sceneName = argv[2];

		LORD::App app(projectFile, sceneName);
		app.run();

		return EXIT_SUCCESS;
	}
}
