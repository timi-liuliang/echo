#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// invisible root item
	QTreeWidgetItem* qTreeWidgetInvisibleRootItem(QWidget* treeWidget);

	// current select item
	QTreeWidgetItem* qTreeWidgetCurrentItem(QWidget* treeWidget);

	// header
	QWidget* qTreeWidgetHeader(QWidget* widget);

	// current column
	int qTreeWidgetCurrentColumn(QWidget* treeWidget);

	// get width
	int qTreeWidgetWidth(QWidget* treeWidget);

	// clear
	void qTreeWidgetClear(QWidget* widget);
}

#endif