#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QScopedPointer>
#include <QtPlugin>
#include <QSettings>
#include "CMDLine.h"
#include "Studio.h"
#include "RegEditMode.h"
#include <engine/core/base/echo_def.h>

int main( int argc, char* argv[])
{
	// Set directory path
    Echo::String filePath = QFileInfo(argv[0]).path().toStdString().c_str();
	QDir::setCurrent(filePath.c_str());
	QApplication::setLibraryPaths(QStringList() << QApplication::libraryPaths() << QDir::currentPath().append("/plugins/Qt"));

	// Regedit
	Echo::RegEditMode::check(argv[0]);

	// Parse & run
	Echo::CMDLine::Parser(argc, argv);
}
