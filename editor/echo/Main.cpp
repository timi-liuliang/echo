#include <QApplication>
#include <QTextCodec>
#include <QFileInfo>
#include <QDir>
#include <QScopedPointer>
#include <QtPlugin>
#include <QSplashScreen>
#include "CMDLine.h"
#include "Studio.h"
#include <shellapi.h>
#include <windows.h>
#include <QTime>
#include <engine/core/base/EchoDef.h>
#include <algorithm>

int main( int argc, char* argv[])
{
#ifdef ECHO_PLATFORM_WINDOWS
	//int aElements[2] = { COLOR_WINDOWFRAME, COLOR_ACTIVECAPTION };

	//DWORD aNewColors[2];
	//aNewColors[0] = RGB(83, 83, 83);  // light gray 
	//aNewColors[1] = RGB(83, 83, 83);  // dark purple

	//SetSysColors(2, aElements, aNewColors);
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

	// 解析命令行，运行主界面。
	Echo::CMDLine::Parser(argc, argv);
}
