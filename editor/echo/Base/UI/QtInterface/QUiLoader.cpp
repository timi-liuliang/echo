#include <QUiLoader>
#include <QFile>
#include <engine/core/util/StringUtil.h>
#include "Studio.h"

namespace Echo
{
	QWidget* qLoadUi( const char* uiFilePath)
	{
		Echo::String path = Studio::AStudio::instance()->getRootPath() + uiFilePath;

		QFile file(path.c_str());
		file.open(QFile::ReadOnly);

		QUiLoader loader;
		QWidget* widget = loader.load(&file, nullptr);

		file.close();

		return widget;
	}
}