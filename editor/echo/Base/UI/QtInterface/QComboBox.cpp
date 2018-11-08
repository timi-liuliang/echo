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
