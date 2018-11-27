#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// invisible root item
	QTreeWidgetItem* qTreeWidgetInvisibleRootItem(QWidget* treeWidget);

	// current select item
	QTreeWidgetItem* qTreeWidgetCurrentItem(QWidget* treeWidget);

	// current column
	int qTreeWidgetCurrentColumn(QWidget* treeWidget);

	// clear
	void qTreeWidgetClear(QWidget* widget);
}

#endif