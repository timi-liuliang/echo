#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	QTreeWidgetItem* qTreeWidgetInvisibleRootItem(QWidget* treeWidget);

	// clear
	void qTreeWidgetClear(QWidget* widget);
}

#endif