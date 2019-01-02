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

	void qLineEditSetMaximumWidth(QWidget* widget, int width)
	{
		QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
		if (lineEdit)
		{
			lineEdit->setMaximumWidth(width);
		}
	}

	void qLineEditSetCursorPosition(QWidget* widget, int position)
	{
		QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
		if (lineEdit)
		{
			lineEdit->setCursorPosition(position);
		}
	}

	String qLineEditText(QWidget* widget)
	{
		QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
		return lineEdit ? lineEdit->text().toStdString().c_str() : StringUtil::BLANK;
	}


	int qLineEditWidth(QWidget* widget)
	{
		QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
		if (lineEdit)
		{
			return lineEdit->width();
		}

		return 0;
	}


	int qLineEditHeight(QWidget* widget)
	{
		QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
		if (lineEdit)
		{
			return lineEdit->height();
		}

		return 0;
	}
}