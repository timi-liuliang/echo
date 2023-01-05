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
	// set directory path
    Echo::String filePath = QFileInfo(argv[0]).path().toStdString().c_str();
	QDir::setCurrent(filePath.c_str());
	QApplication::setLibraryPaths(QStringList() << QApplication::libraryPaths() << QDir::currentPath().append("/plugins/Qt"));

#ifdef ECHO_PLATFORM_WINDOWS
	// Generate Visual Studio project files
	QSettings regGv("HKEY_CLASSES_ROOT\\.echo\\shell\\Generate Visual Studio Files\\command", QSettings::NativeFormat);
	regGv.setValue("Default", (Echo::String(argv[0]) + " gen_vs %1").c_str());

	QSettings regIcon("HKEY_CLASSES_ROOT\\.echo\\shell\\Generate Visual Studio Files", QSettings::NativeFormat);
	regIcon.setValue("Icon", Echo::StringUtil::Format("%s", argv[0]).c_str());
#endif

	// parse & run
	Echo::CMDLine::Parser(argc, argv);
}
