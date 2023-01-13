#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QScopedPointer>
#include <QtPlugin>
#include <QSettings>
#include "CMDLine.h"
#include "Studio.h"
#ifdef ECHO_PLATFORM_WINDOWS
#include <windows.h>
#include <shellapi.h>
#endif
#include <engine/core/base/echo_def.h>
#include <app/game/game_config.h>

int main( int argc, char* argv[])
{
	// Set directory path
    Echo::String filePath = QFileInfo(argv[0]).path().toStdString().c_str();
	QDir::setCurrent(filePath.c_str());
	QApplication::setLibraryPaths(QStringList() << QApplication::libraryPaths() << QDir::currentPath().append("/plugins/Qt"));

	// Regedit
#if defined(ECHO_PLATFORM_WINDOWS) && !defined(ECHO_GAME_SOURCE) 
	QSettings regIcon("HKEY_CLASSES_ROOT\\.echo\\shell\\Generate Visual Studio Files", QSettings::NativeFormat);
	Echo::String currentEdit = regIcon.value("Icon").toString().toStdString().c_str();
	if (currentEdit != argv[0])
	{
		HINSTANCE result = ShellExecute(NULL, "runas", argv[0], "regedit", NULL, SW_SHOWNORMAL);
		assert (result != 0);
	}
#endif

	// parse & run
	Echo::CMDLine::Parser(argc, argv);
}
