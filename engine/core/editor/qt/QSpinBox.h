#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// new QLineEdit
	typedef QWidget* (*qSpinBoxNewFun)();

	// set text
	typedef void (*qSpinBoxSetValueFun)(QWidget* widget, int value);

	// set maximum width
	typedef int (*qSpinBoxValueFun)(QWidget* widget);
}

#endif
