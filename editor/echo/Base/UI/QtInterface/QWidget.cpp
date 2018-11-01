#include <QtWidgets/QWidget>

namespace Echo
{
	QWidget* qFindChild(QWidget* root, const char* name)
	{
		QWidget* widget = root->findChild<QWidget*>(name);
		return widget;
	}

	void qDeleteWidget(QWidget*& widget)
	{
		if (widget)
		{
			delete widget;
			widget = nullptr;
		}
	}
}
