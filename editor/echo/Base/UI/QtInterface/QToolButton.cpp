#include <QToolButton>
#include <engine/core/util/StringUtil.h>
#include "Studio.h"

namespace Echo
{
	void qToolButtonSetIcon(QWidget* button, const char* iconPath)
	{
		if (button)
		{
			QToolButton* toolButton = qobject_cast<QToolButton*>(button);
			if (toolButton)
			{
				Echo::String fullPath = Studio::AStudio::instance()->getRootPath() + iconPath;
				toolButton->setIcon(QIcon(fullPath.c_str()));
			}
		}
	}
}
