#include <QTreeWidgetItem>
#include <engine/core/util/StringUtil.h>
#include "Studio.h"

namespace Echo
{
	QTreeWidgetItem* qTreeWidgetItemNew()
	{
		return new QTreeWidgetItem();
	}

	// get parent
	QTreeWidgetItem* qTreeWidgetItemParent(QTreeWidgetItem* item)
	{
		return item ? item->parent() : nullptr;
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
			item->setText( column, text);
		}
	}

	String qTreeWidgetItemText(QTreeWidgetItem* item, int column)
	{
		if (item)
		{
			return item->text( column).toStdString().c_str();
		}

		return Echo::StringUtil::BLANK;
	}

	String qTreeWidgetItemUserData(QTreeWidgetItem* item, int column)
	{
		if (item)
		{
			return item->data(column, Qt::UserRole).toString().toStdString().c_str();
		}

		return StringUtil::BLANK;
	}

	void qTreeWidgetItemSetUserData(QTreeWidgetItem* item, int column, const char* userData)
	{
		if (item)
		{
			item->setData(column, Qt::UserRole, userData);
		}
	}

	void qTreeWidgetItemSetIcon(QTreeWidgetItem* item, int column, const char* iconPath)
	{
		if (item)
		{
			Echo::String fullPath = Echo::StringUtil::StartWith(iconPath, ":/") ? iconPath : Studio::AStudio::instance()->getRootPath() + iconPath;
			item->setIcon(column, QIcon(fullPath.c_str()));
		}
	}

	// expand QTreeWidgetItem
	void qTreeWidgetItemSetExpanded(QTreeWidgetItem* item, bool aexpand)
	{
		if (item)
		{
			item->setExpanded( aexpand);
		}
	}
}