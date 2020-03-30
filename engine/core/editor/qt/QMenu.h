#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a new QMenu
	typedef QWidget* (*qMenuNewFun)(QWidget* parent);

	// void addAction(QAction *action)
	typedef void (*qMenuAddActionFun)(QWidget* menu, QAction* action);

	// add separator
	typedef void(*qMenuAddSeparatorFun)(QWidget* menu);

	// QAction *exec();
	typedef void (*qMenuExecFun)(QWidget* menu);
}

#endif
