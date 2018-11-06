#include <QApplication>
#include <QTextCodec>
#include <QFileInfo>
#include <QDir>
#include <QScopedPointer>
#include <QtPlugin>
#include <QSplashScreen>
#include <QSettings>
#include "CMDLine.h"
#include "Studio.h"
#include <shellapi.h>
#include <windows.h>
#include <QTime>
#include <engine/core/base/echo_def.h>
#include <algorithm>

int main( int argc, char* argv[])
{
#ifdef ECHO_PLATFORM_WINDOWS
	// default icon
	QSettings settings;
#endif

	// set directory path
	QString filePath = QFileInfo(argv[0]).path();
	if (filePath.length() <= 1)
	{
		char buffer[1024] = {};
		::GetModuleFileNameA(NULL, buffer, 1024);
		filePath = QFileInfo(buffer).path();
	}
	else
	{
		filePath = QFileInfo(argv[0]).path();
	}

	QDir::setCurrent(filePath);
	QApplication::setLibraryPaths(QStringList() << QApplication::libraryPaths() << QDir::currentPath().append("/plugins/Qt"));

	// parse & run
	Echo::CMDLine::Parser(argc, argv);
}
