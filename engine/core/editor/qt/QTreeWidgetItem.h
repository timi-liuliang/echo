#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a new QTreeWidgetItem
	QTreeWidgetItem* qTreeWidgetItemNew();

	// get parent
	QTreeWidgetItem* qTreeWidgetItemParent(QTreeWidgetItem* item);

	// add child item
	void qTreeWidgetItemAddChild(QTreeWidgetItem* parent, QTreeWidgetItem* child);

	// set text
	void qTreeWidgetItemSetText(QTreeWidgetItem* item, int column, const char* text);
	
	// get text
	String qTreeWidgetItemText(QTreeWidgetItem* item, int column);

	// set icon
	void qTreeWidgetItemSetIcon(QTreeWidgetItem* item, int column, const char* iconPath);

	// get user data
	String qTreeWidgetItemUserData(QTreeWidgetItem* item, int column);

	// set user data
	void qTreeWidgetItemSetUserData(QTreeWidgetItem* item, int column, const char* userData);

	// expand QTreeWidgetItem
	void qTreeWidgetItemSetExpanded(QTreeWidgetItem* item, bool aexpand);
}

#endif