#pragma once

#include "engine/core/base/echo_def.h"

#ifdef ECHO_EDITOR_MODE

class QWidget;

namespace Echo
{
	// QComboBox->addItem(const QIcon &icon, const QString &text)
	void qComboBoxAddItem(QWidget* widget, const char* icon, const char* text);

	// QComboBox->clear();
	void qComboBoxClear(QWidget* widget);
}

#endif