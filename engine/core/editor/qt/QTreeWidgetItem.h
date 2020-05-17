#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a new QTreeWidgetItem
	typedef QTreeWidgetItem* (*qTreeWidgetItemNewFun)();

	// get parent
	typedef QTreeWidgetItem* (*qTreeWidgetItemParentFun)(QTreeWidgetItem* item);

	// add child item
	typedef void (*qTreeWidgetItemAddChildFun)(QTreeWidgetItem* parent, QTreeWidgetItem* child);

	// set text
	typedef void (*qTreeWidgetItemSetTextFun)(QTreeWidgetItem* item, int column, const char* text);
	
	// get text
	typedef String (*qTreeWidgetItemTextFun)(QTreeWidgetItem* item, int column);

	// set icon
	typedef void (*qTreeWidgetItemSetIconFun)(QTreeWidgetItem* item, int column, const char* iconPath);

	// get user data
	typedef String (*qTreeWidgetItemUserDataFun)(QTreeWidgetItem* item, int column);

	// set user data
	typedef void (*qTreeWidgetItemSetUserDataFun)(QTreeWidgetItem* item, int column, const char* userData);

	// set editable
	typedef void (*qTreeWidgetItemSetEditableFun)(QTreeWidgetItem* item, bool editable);

	// expand QTreeWidgetItem
	typedef void (*qTreeWidgetItemSetExpandedFun)(QTreeWidgetItem* item, bool aexpand);

	// child count
	typedef i32(*qTreeWidgetItemChildCountFun)(QTreeWidgetItem* item);

	// child at index
	typedef QTreeWidgetItem* (*qTreeWidgetItemChildFun)(QTreeWidgetItem* item, i32 index);
}

#endif
