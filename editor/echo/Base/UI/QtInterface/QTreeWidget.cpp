#include <QTreeWidget>

namespace Echo
{
	QTreeWidgetItem* qTreeWidgetInvisibleRootItem(QWidget* widget)
	{
		if (widget)
		{
			QTreeWidget* treewidget = qobject_cast<QTreeWidget*>(widget);
			if (treewidget)
			{
				return treewidget->invisibleRootItem();
			}
		}

		return nullptr;
	}

	// clear
	void qTreeWidgetClear(QWidget* widget)
	{
		if (widget)
		{
			QTreeWidget* treewidget = qobject_cast<QTreeWidget*>(widget);
			if (treewidget)
			{
				treewidget->clear();
			}
		}
	}
}