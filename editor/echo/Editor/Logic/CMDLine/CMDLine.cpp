#include "CMDLine.h"
#include <string>
#include <QDir>
#include <QSplashScreen>
#include <QApplication>
#include <QTextCodec>
#include <QSettings>
#include <QDesktopServices>
#include <QTime>
#include "Studio.h"
#include "EchoEngine.h"
#include "MainWindow.h"
#include "Window.h"
#include "ProjectWnd.h"
#include "EditorConfig.h"
#include "GameMainWindow.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/util/TimeProfiler.h>
#include <engine/core/io/io.h>

static const char* g_batchGotAdmin = R"(@echo off
:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
    IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params= %*
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params:"=""%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------    

)";

static const char* g_runArguments = R"(<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <LocalDebuggerCommandArguments>${Args}</LocalDebuggerCommandArguments>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <LocalDebuggerCommandArguments>${Args}</LocalDebuggerCommandArguments>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
</Project>
)";

namespace Echo
{
	Studio::AStudio* g_astudio = NULL;

	bool CMDLine::Parser(int argc, char* argv[])
	{
		if ( argc > 1 )
		{	
			StringArray sargv;
			for( int i=1; i<argc; i++)
				sargv.emplace_back( argv[i]);

			if (sargv[0] == "play")
			{
				GameMode gameMode;
				gameMode.exec(argc, argv);
			}
			else if (sargv[0] == "vs")
			{
				VsGenMode vsGenMode;
				vsGenMode.exec(argc, argv);
			}
			else if (sargv[0] == "regedit")
			{
				RegEditMode regEditMode;
				regEditMode.exec(argc, argv);
			}
			else if (sargv[0] == "open")
			{
				EditOpenMode openMode;
				openMode.exec(argc, argv);
			}

			return true;
		}
		else
		{
			EditorMode editorMode;
			editorMode.exec(argc, argv);
		}

		return false;
	}

	// exec command
	bool EditorMode::exec(int argc, char* argv[])
	{
		QApplication app(argc, argv);

		app.setAttribute(Qt::AA_NativeWindows);

		QTextCodec *codec = QTextCodec::codecForName("GB18030");
		QTextCodec::setCodecForLocale(codec);

		// launch image
		//int idx = QTime(0, 0, 0).secsTo(QTime::currentTime()) % 4;
		//Echo::String iconLocation = Echo::StringUtil::Format(":/icon/Icon/Launch/launch-%d.png", idx);

		//QSplashScreen splash;
		//splash.setPixmap(QPixmap(iconLocation.c_str()));
		//splash.show();
		//splash.showMessage(QString::fromLocal8Bit("Echo (32 bit OpenGLES) Copyright @ 2018-2019 B-Lab"), Qt::AlignLeft | Qt::AlignBottom, Qt::white);

		// new astudio
        g_astudio = Studio::AStudio::instance();

		// load editor config
		TIME_PROFILE
		(
			Studio::EditorConfig::instance()->loadCfgFile();
		)

		// Qss
		{
			Echo::String theme = Studio::EditorConfig::instance()->getValue("CurrentTheme");
			theme = theme.empty() ? ":/Qss/Qss/Ps.qss" : theme;
			if (!theme.empty())
			{
				Studio::MainWindow::setTheme(theme.c_str());
			}
		}

		TIME_PROFILE
		(
			g_astudio->Start();
		)

		TIME_PROFILE
		(
			//ThreadSleepByMilliSecond(1000);
			g_astudio->getProjectWindow()->show();
			//splash.finish(g_astudio->getProjectWindow());
		)

		// run application
		app.exec();

		delete g_astudio;

		return true;
	}

	bool GameMode::exec(int argc, char* argv[])
	{
		Echo::String type = argv[1];
		if (type != "play")
			return false;

		Echo::String projectFile = argv[2];

		QApplication app(argc, argv);
		app.setAttribute(Qt::AA_NativeWindows);

		// set qss
		QFile qssFile(":/Qss/Qss/Ps.qss");
		qssFile.open(QFile::ReadOnly);
		if (qssFile.isOpen())
		{
			QString qss = QLatin1String(qssFile.readAll());
			app.setStyleSheet(qss);
			qssFile.close();
		}

		// start window
		Game::GameMainWindow mainWindow;
		mainWindow.show();
		mainWindow.start(projectFile);

		app.exec();

		return true;
	}

	bool EditOpenMode::exec(int argc, char* argv[])
	{
		QApplication app(argc, argv);
		app.setAttribute(Qt::AA_NativeWindows);

		// codec
		QTextCodec *codec = QTextCodec::codecForName("GB18030");
		QTextCodec::setCodecForLocale(codec);

		// ui style
		QFile qssFile(":/Qss/Qss/Ps.qss");
		qssFile.open(QFile::ReadOnly);
		if (qssFile.isOpen())
		{
			QString qss = QLatin1String(qssFile.readAll());
			app.setStyleSheet(qss);

			qssFile.close();
		}

		TIME_PROFILE
		(
			g_astudio = Studio::AStudio::instance();
		)

		TIME_PROFILE
		(
			g_astudio->getConfigMgr()->loadCfgFile();
		)

		TIME_PROFILE
		(
			g_astudio->Start();
		)

		TIME_PROFILE
		(
			Echo::String projectFile = argv[2];
			Echo::PathUtil::FormatPath(projectFile, false);
			g_astudio->getProjectWindow()->openProject( projectFile);
		)

		app.exec();

		delete g_astudio;

		return true;
	}

	bool VsGenMode::exec(int argc, char* argv[])
	{
		using namespace Echo;

		Echo::String type = argv[1];
		if (type == "vs")
		{
			Echo::String project = argv[2];
			Echo::PathUtil::FormatPath(project);

			Echo::String editor = argv[0];
			Echo::PathUtil::FormatPath(editor);

			Echo::String projectPath = Echo::PathUtil::GetFileDirPath(project);
			Echo::String projectName = Echo::PathUtil::GetPureFilename(project, false);
			Echo::String projectSrcPath = projectPath + "Source/";
			Echo::String buildPath = projectPath + "Build/";
			Echo::String binaryPath = projectPath + "Bin/";
			Echo::String vsVersion = "-G\"Visual Studio 17 2022\" -A x64";
			Echo::String batFile = buildPath + "cmake.bat";
			Echo::String vsSolutionFile = projectPath + projectName + ".sln";

			Echo::String engineEditExePath = Echo::PathUtil::GetFileDirPath(editor);
			Echo::String enginePath = PathUtil::GetParentPath(PathUtil::GetParentPath(PathUtil::GetParentPath(PathUtil::GetParentPath(engineEditExePath))));

			writeCMakeBatFile(projectName.c_str(), projectSrcPath.c_str(), batFile.c_str(), enginePath.c_str(), vsSolutionFile.c_str());

#ifdef ECHO_PLATFORM_WINDOWS
			Echo::String cmd = StringUtil::Format("%s", batFile.c_str());

			QProcess process;
			process.start("cmd.exe", QStringList() << "/c" << batFile.c_str());
			process.waitForFinished();

			Echo::String output(process.readAllStandardOutput());
			printf(output.c_str());

			Echo::String userFilePath = buildPath + "editor/echo/echo.vcxproj.user";
			Echo::String args = "open " + project;
			writeVcxprojUser(userFilePath, args);
#endif

			return true;
		}

		return false;
	}

	void VsGenMode::writeCMakeBatFile(const char* projectName, const char* projectSrcPath, const char* batFile, const char* enginePath, const char* vsSolutionFile)
	{
		using namespace Echo;

		String batSrc;

		StringUtil::WriteLine(batSrc, g_batchGotAdmin);

		StringUtil::WriteLine(batSrc, ":: start in current directory ");
		StringUtil::WriteLine(batSrc, "cd /D \"%~dp0\"\n");

		//-DECHO_GAME=TRUE -DECHO_GAME_NAME=moon -DECHO_GAME_SOURCE=D:/github/myprojects/moon/Source
		StringUtil::WriteLine(batSrc, StringUtil::Format("call cmake.exe -DECHO_GAME_SOURCE=TRUE -DECHO_GAME_NAME=\"%s\" -DECHO_GAME_SOURCE_PATH=\"%s\" %s\n", projectName, projectSrcPath, enginePath).c_str());

		StringUtil::WriteLine(batSrc, ":: make link");
		StringUtil::WriteLine(batSrc, "cd /D \"%~dp0\"");
		StringUtil::WriteLine(batSrc, "cd ../\n");
		StringUtil::WriteLine(batSrc, Echo::StringUtil::Format("if not exist .\\%s.sln mklink .\\%s.sln .\\Build\\echo.sln\n", projectName, projectName));

		StringUtil::WriteLine(batSrc, ":: delay close");
		StringUtil::WriteLine(batSrc, "timeout /t 10");

		IO::instance()->saveStringToFile(batFile, batSrc);
	}

	void VsGenMode::writeVcxprojUser(const Echo::String& userFilePath, const Echo::String& args)
	{
		Echo::String userSrc = Echo::StringUtil::Replace(g_runArguments, "${Args}", args.c_str());
		IO::instance()->saveStringToFile(userFilePath, userSrc);
	}

	bool RegEditMode::exec(int argc, char* argv[])
	{
		Echo::String type = argv[1];
		if (type != "regedit")
			return false;

#ifdef ECHO_PLATFORM_WINDOWS
		QSettings regOpenIcon("HKEY_CLASSES_ROOT\\.echo\\shell\\Open", QSettings::NativeFormat);
		regOpenIcon.setValue("Icon", Echo::StringUtil::Format("%s", argv[0]).c_str());

		QSettings regOpen("HKEY_CLASSES_ROOT\\.echo\\shell\\Open\\command", QSettings::NativeFormat);
		regOpen.setValue("Default", (Echo::String(argv[0]) + " %1").c_str());

		QSettings regIcon("HKEY_CLASSES_ROOT\\.echo\\shell\\Generate Visual Studio Files", QSettings::NativeFormat);
		regIcon.setValue("Icon", Echo::StringUtil::Format("%s", argv[0]).c_str());

		QSettings regGv("HKEY_CLASSES_ROOT\\.echo\\shell\\Generate Visual Studio Files\\command", QSettings::NativeFormat);
		regGv.setValue("Default", (Echo::String(argv[0]) + " vs %1").c_str());
#endif

		return true;
	}
}
