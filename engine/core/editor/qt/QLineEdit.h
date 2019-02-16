#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// new QLineEdit
	typedef QWidget* (*qLineEditNewFun)();

	// set text
	typedef void (*qLineEditSetTextFun)(QWidget* widget, const String& text);

	// set maximum width
	typedef void (*qLineEditSetMaximumWidthFun)(QWidget* widget, int width);

	// set cursor position
	typedef void (*qLineEditSetCursorPositionFun)(QWidget* widget, int position);

	// get text of QLineEdit
	typedef String (*qLineEditTextFun)(QWidget* widget);

	// width
	typedef int (*qLineEditWidthFun)(QWidget* widget);

	// height
	typedef int (*qLineEditHeightFun)(QWidget* widget);
}

#endif
