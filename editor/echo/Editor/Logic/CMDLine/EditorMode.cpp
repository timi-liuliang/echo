#include "EditorMode.h"
#include <QApplication>
#include <QTextCodec>
#include "Studio.h"
#include "MainWindow.h"
#include "ProjectWnd.h"
#include <engine/core/util/TimeProfiler.h>

namespace Echo
{
	// exec command
	bool EditorMode::exec(int argc, char* argv[])
	{
		QApplication app(argc, argv);

		app.setAttribute(Qt::AA_NativeWindows);

		QTextCodec* codec = QTextCodec::codecForName("GB18030");
		QTextCodec::setCodecForLocale(codec);

		// launch image
		//int idx = QTime(0, 0, 0).secsTo(QTime::currentTime()) % 4;
		//Echo::String iconLocation = Echo::StringUtil::Format(":/icon/Icon/Launch/launch-%d.png", idx);

		//QSplashScreen splash;
		//splash.setPixmap(QPixmap(iconLocation.c_str()));
		//splash.show();
		//splash.showMessage(QString::fromLocal8Bit("Echo (32 bit OpenGLES) Copyright @ 2018-2019 B-Lab"), Qt::AlignLeft | Qt::AlignBottom, Qt::white);

		// new astudio
		Studio::AStudio* astudio = Studio::AStudio::instance();

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
			astudio->start();
		)

		TIME_PROFILE
		(
			//ThreadSleepByMilliSecond(1000);
			astudio->getProjectWindow()->show();
			//splash.finish(astudio->getProjectWindow());
		)

		// run application
		app.exec();

		delete astudio;

		return true;
	}
}