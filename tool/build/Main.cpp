#include <stdio.h>
#include "Cmd/CMDLine.h"
#include <QSettings>
#ifdef ECHO_PLATFORM_WINDOWS
#include <windows.h>
#include <shellapi.h>
#endif

namespace Echo
{
	void registerGameModules() {}
	void registerModules() {}
}

int main(int argc, char* argv[])
{
	// Regedit
#if defined(ECHO_PLATFORM_WINDOWS)
	QSettings regIcon("HKEY_CLASSES_ROOT\\.echo\\shell\\Generate Visual Studio Files\\command", QSettings::NativeFormat);
	Echo::String currentEdit = regIcon.value("Default").toString().toStdString().c_str();
	if (!Echo::StringUtil::StartWith(currentEdit, argv[0]))
	{
		HINSTANCE result = ShellExecute(NULL, "runas", argv[0], "regedit", NULL, SW_SHOWNORMAL);
		assert(result != 0);
	}
#endif

	// parse & run
	Echo::CMDLine::Parser(argc, argv);

	return 0;
}
