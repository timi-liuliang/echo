#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// invisible root item
	typedef QTreeWidgetItem* (*qTreeWidgetInvisibleRootItemFun)(QWidget* treeWidget);

	// current select item
	typedef QTreeWidgetItem* (*qTreeWidgetCurrentItemFun)(QWidget* treeWidget);

	// header
	typedef QWidget* (*qTreeWidgetHeaderFun)(QWidget* widget);

	// current column
	typedef int (*qTreeWidgetCurrentColumnFun)(QWidget* treeWidget);

	// get width
	typedef int (*qTreeWidgetWidthFun)(QWidget* treeWidget);

	// clear
	typedef void (*qTreeWidgetClearFun)(QWidget* widget);
}

#endif
