#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QScopedPointer>
#include <QtPlugin>
#include <QSettings>
#include "CMDLine.h"
#include "Studio.h"
#ifdef ECHO_PLATFORM_WINDOWS
    #include <shellapi.h>
    #include <windows.h>
#endif
#include <engine/core/base/echo_def.h>

int main( int argc, char* argv[])
{
	// default icon
	QSettings settings;

	// set directory path
    Echo::String filePath = QFileInfo(argv[0]).path().toStdString().c_str();
	QDir::setCurrent(filePath.c_str());
	QApplication::setLibraryPaths(QStringList() << QApplication::libraryPaths() << QDir::currentPath().append("/plugins/Qt"));

	// parse & run
	Echo::CMDLine::Parser(argc, argv);
}
