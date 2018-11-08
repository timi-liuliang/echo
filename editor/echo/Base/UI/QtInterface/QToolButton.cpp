#include <QToolButton>
#include <engine/core/util/StringUtil.h>
#include "Studio.h"

namespace Echo
{
	void qToolButtonSetFixedWidth(QWidget* button, int width)
	{
		if (button)
		{
			QToolButton* toolButton = qobject_cast<QToolButton*>(button);
			if (toolButton)
			{
				toolButton->setFixedWidth(width);
			}
		}
	}

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

	// set icon size
	void qToolButtonSetIconSize(QWidget* widget, int width, int height)
	{
		if (widget)
		{
			QToolButton* toolButton = qobject_cast<QToolButton*>(widget);
			if (toolButton)
			{
				toolButton->setIconSize(QSize(width, height));
			}
		}
	}
}
