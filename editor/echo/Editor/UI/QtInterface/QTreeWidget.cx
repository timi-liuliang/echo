#include <QTreeWidget>
#include <QHeaderView>

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

	// header
	QWidget* qTreeWidgetHeader(QWidget* widget)
	{
		if (widget)
		{
			QTreeWidget* treewidget = qobject_cast<QTreeWidget*>(widget);
			if (treewidget)
			{
				return treewidget->header();
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

	// get width
	int qTreeWidgetWidth(QWidget* widget)
	{
		if (widget)
		{
			QTreeWidget* treewidget = qobject_cast<QTreeWidget*>(widget);
			if (treewidget)
			{
				return treewidget->width();
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