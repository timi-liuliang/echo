#pragma once

#include "engine/core/base/echo_def.h"

#ifdef ECHO_EDITOR_MODE

class QWidget;

namespace Echo
{
	// QComboBox->addItem(const QIcon &icon, const QString &text)
	typedef void (*qComboBoxAddItemFun)(QWidget* widget, const char* icon, const char* text);

	// int QComboBox->currentIndex() const;
	typedef int (*qComboBoxCurrentIndexFun)(QWidget* widget);

	// QString QComboBox->currentText() const;
	typedef String (*qComboBoxCurrentTextFun)(QWidget* widget);

	// QComboBox->setCurrentIndex(int index);
	typedef void (*qComboBoxSetCurrentIndexFun)(QWidget* widget, int index);

	// QComboBox->setCurrentText(const QString &text);
	typedef void (*qComboBoxSetCurrentTextFun)(QWidget* widget, const char* text);

	// void QComboBox->setItemText(int index, const QString &text);
	typedef void (*qComboBoxSetItemTextFun)(QWidget* widget, int index, const char* text);

	// QComboBox->clear();
	typedef void (*qComboBoxClearFun)(QWidget* widget);
}

#endif
