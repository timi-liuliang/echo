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

int main( int argc, char* argv[])
{
	// Set directory path
    Echo::String filePath = QFileInfo(argv[0]).path().toStdString().c_str();
	QDir::setCurrent(filePath.c_str());
	QApplication::setLibraryPaths(QStringList() << QApplication::libraryPaths() << QDir::currentPath().append("/plugins/Qt"));

	// Parse & run
	Echo::CMDLine::Parser(argc, argv);
}
