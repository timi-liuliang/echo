#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a new QTreeWidgetItem
	QTreeWidgetItem* qTreeWidgetItemNew();

	// add child item
	void qTreeWidgetItemAddChild(QTreeWidgetItem* parent, QTreeWidgetItem* child);

	// set text
	void qTreeWidgetItemSetText(QTreeWidgetItem* item, int column, const char* text);

	// set icon
	void qTreeWidgetItemSetIcon(QTreeWidgetItem* item, int column, const char* iconPath);
}

#endif