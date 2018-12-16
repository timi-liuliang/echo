#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// new QLineEdit
	QWidget* qLineEditNew();

	// set text
	void qLineEditSetText(QWidget* widget, const String& text);

	// get text of QLineEdit
	String qLineEditText(QWidget* widget);
}

#endif