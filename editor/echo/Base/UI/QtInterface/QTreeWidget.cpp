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

	// current select item
	QTreeWidgetItem* qTreeWidgetCurrentItem(QWidget* widget)
	{
		if (widget)
		{
			QTreeWidget* treewidget = qobject_cast<QTreeWidget*>(widget);
			if (treewidget)
			{
				return treewidget->currentItem();
			}
		}

		return nullptr;
	}

	// current column
	int qTreeWidgetCurrentColumn(QWidget* widget)
	{
		if (widget)
		{
			QTreeWidget* treewidget = qobject_cast<QTreeWidget*>(widget);
			if (treewidget)
			{
				return treewidget->currentColumn();
			}
		}

		return 0;
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