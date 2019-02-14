#include <QComboBox>
#include <engine/core/util/StringUtil.h>
#include "Studio.h"

namespace Echo
{
	void qComboBoxAddItem(QWidget* widget, const char* icon, const char* text)
	{
		if (widget)
		{
			QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
			if (comboBox)
			{
				if (icon)
					comboBox->addItem(QIcon(icon), text);
				else
					comboBox->addItem(text);
			}
		}
	}

	int qComboBoxCurrentIndex(QWidget* widget)
	{
		String result;
		if (widget)
		{
			QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
			if (comboBox)
			{
				return comboBox->currentIndex();
			}
		}

		return -1;
	}

	String qComboBoxCurrentText(QWidget* widget)
	{
		String result;
		if (widget)
		{
			QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
			if (comboBox)
			{
				result = comboBox->currentText().toStdString().c_str();
			}
		}

		return result;
	}

	void qComboBoxSetCurrentIndex(QWidget* widget, int index)
	{
		if (widget)
		{
			QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
			if (comboBox)
			{
				comboBox->setCurrentIndex( index);
			}
		}
	}

	void qComboBoxSetCurrentText(QWidget* widget, const char* text)
	{
		if (widget)
		{
			QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
			if (comboBox)
			{
				comboBox->setCurrentText(text);
			}
		}
	}

	void qComboBoxSetItemText(QWidget* widget, int index, const char* text)
	{
		if (widget)
		{
			QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
			if (comboBox)
			{
				comboBox->setItemText(index, text);
			}
		}
	}

	void qComboBoxClear(QWidget* widget)
	{
		if (widget)
		{
			QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
			if (comboBox)
			{
				comboBox->clear();
			}
		}
	}
}
