#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// new QLineEdit
	QWidget* qLineEditNew();

	// set text
	void qLineEditSetText(QWidget* widget, const String& text);

	// set maximum width
	void qLineEditSetMaximumWidth(QWidget* widget, int width);

	// set cursor position
	void qLineEditSetCursorPosition(QWidget* widget, int position);

	// get text of QLineEdit
	String qLineEditText(QWidget* widget);

	// width
	int qLineEditWidth(QWidget* widget);

	// height
	int qLineEditHeight(QWidget* widget);
}

#endif