#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a new QMenu
	QWidget* qMenuNew(QWidget* parent=nullptr);

	// void addAction(QAction *action)
	void qMenuAddAction(QWidget* menu, QAction* action);

	// QAction *exec();
	void qMenuExec(QWidget* menu);
}

#endif