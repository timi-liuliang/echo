#include "GameMode.h"
#include <QApplication>
#include <QDir>
#include "GameMainWindow.h"

namespace Echo
{
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
}