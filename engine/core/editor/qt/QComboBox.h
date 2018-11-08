#pragma once

#include "engine/core/base/echo_def.h"

#ifdef ECHO_EDITOR_MODE

class QWidget;

namespace Echo
{
	// QComboBox->addItem(const QIcon &icon, const QString &text)
	void qComboBoxAddItem(QWidget* widget, const char* icon, const char* text);

	// int QComboBox->currentIndex() const;
	int qComboBoxCurrentIndex(QWidget* widget);

	// QString QComboBox->currentText() const;
	String qComboBoxCurrentText(QWidget* widget);

	// QComboBox->setCurrentIndex(int index);
	void qComboBoxSetCurrentIndex(QWidget* widget, int index);

	// QComboBox->setCurrentText(const QString &text);
	void qComboBoxSetCurrentText(QWidget* widget, const char* text);

	// void QComboBox->setItemText(int index, const QString &text);
	void qComboBoxSetItemText(QWidget* widget, int index, const char* text);

	// QComboBox->clear();
	void qComboBoxClear(QWidget* widget);
}

#endif