#include <QLineEdit>
#include <engine/core/util/StringUtil.h>

namespace Echo
{
	QWidget* qLineEditNew()
	{
		return new QLineEdit();
	}

	void qLineEditSetText(QWidget* widget, const String& text)
	{
		QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
		if (lineEdit)
		{
			lineEdit->setText(text.c_str());
		}
	}

	String qLineEditText(QWidget* widget)
	{
		QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
		return lineEdit ? lineEdit->text().toStdString().c_str() : StringUtil::BLANK;
	}
}