#include "EditorOpenMode.h"
#include <QApplication>
#include <QTextCodec>
#include <QDir>
#include <QTimer>
#include "Studio.h"
#include "ProjectWnd.h"
#include <engine/core/util/TimeProfiler.h>
#include <engine/core/util/PathUtil.h>

namespace Echo
{
	bool EditOpenMode::exec(int argc, char* argv[])
	{
		QApplication app(argc, argv);
		app.setAttribute(Qt::AA_NativeWindows);

		// codec
		QTextCodec* codec = QTextCodec::codecForName("GB18030");
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

		Studio::AStudio * astudio = Studio::AStudio::instance();
		astudio->getConfigMgr()->loadCfgFile();
		astudio->start();

		TIME_PROFILE
		(
			// How to execute initialization method after QApplication::exec has been called?
			// https://forum.qt.io/topic/92873/how-to-execute-initialization-method-after-qapplication-exec-has-been-called
			QTimer::singleShot(1000, [&]()
				{
					Echo::String projectFile = argv[2];
					Echo::PathUtil::FormatPath(projectFile, false);
					astudio->getProjectWindow()->openProject(projectFile);
				});
		)

		app.exec();

		delete astudio;

		return true;
	}
}