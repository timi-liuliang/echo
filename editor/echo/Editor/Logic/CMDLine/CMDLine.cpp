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
			else if ( argc==2)
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
			Echo::String projectFile = argv[1];
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
			Echo::String buildPath = projectPath + "Build/";
			Echo::String binaryPath = projectPath + "Bin/";
			Echo::String vsVersion = "-G\"Visual Studio 16 2019\" -A x64";
			Echo::String batFile = buildPath + "cmake.bat";

			Echo::String engineEditExePath = Echo::PathUtil::GetFileDirPath(editor);
			Echo::String enginePath = PathUtil::GetParentPath(PathUtil::GetParentPath(PathUtil::GetParentPath(PathUtil::GetParentPath(engineEditExePath))));

			writeCMakeBatFile(batFile.c_str(), enginePath.c_str());

#ifdef ECHO_PLATFORM_WINDOWS
			Echo::String cmd = StringUtil::Format("%s", batFile.c_str());

			QProcess process;
			process.start("cmd.exe", QStringList() << "/c" << batFile.c_str());
			process.waitForFinished();

			Echo::String output(process.readAllStandardOutput());
			printf(output.c_str());
#endif

			return true;
		}

		return false;
	}

	void VsGenMode::writeCMakeBatFile(const char* batFile, const char* enginePath)
	{
		using namespace Echo;

		String batSrc;

		StringUtil::WriteLine(batSrc, "echo off\n");

		StringUtil::WriteLine(batSrc, ":: start in current directory ");
		StringUtil::WriteLine(batSrc, "cd /D \"%~dp0\"\n");

		StringUtil::WriteLine(batSrc, StringUtil::Format("call cmake.exe %s", enginePath).c_str());

		IO::instance()->saveStringToFile(batFile, batSrc);
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
