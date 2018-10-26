#include "App.h"
#include <engine/core/util/PathUtil.h>

int main(int argc, char* argv[])
{
	Echo::String exeFullPath = argv[0];
	Echo::PathUtil::FormatPath(exeFullPath);
	Echo::String rootPath = Echo::PathUtil::GetFileDirPath(exeFullPath);

	Echo::App app(rootPath);
	app.start();

	return EXIT_SUCCESS;
}
