#include <QTreeWidgetItem>
#include <engine/core/util/StringUtil.h>
#include "Studio.h"

namespace Echo
{
	QTreeWidgetItem* qTreeWidgetItemNew()
	{
		return new QTreeWidgetItem();
	}

	void qTreeWidgetItemAddChild(QTreeWidgetItem* parent, QTreeWidgetItem* child)
	{
		if (parent && child)
		{
			parent->addChild(child);
		}
	}

	void qTreeWidgetItemSetText(QTreeWidgetItem* item, int column, const char* text)
	{
		if (item)
		{
			item->setText(0, text);
		}
	}

	void qTreeWidgetItemSetIcon(QTreeWidgetItem* item, int column, const char* iconPath)
	{
		if (item)
		{
			Echo::String fullPath = Studio::AStudio::instance()->getRootPath() + iconPath;
			item->setIcon(column, QIcon(fullPath.c_str()));
		}
	}
}