#include <QTreeWidgetItem>

namespace Echo
{
	QTreeWidgetItem* qTreeWidgetItemNew()
	{
		return new QTreeWidgetItem();
	}

	void qTreeWidgetItemAddChild(QTreeWidgetItem* parent, QTreeWidgetItem* child)
	{
		if (parent && child)
		{
			parent->addChild(child);
		}
	}

	void qTreeWidgetItemSetText(QTreeWidgetItem* item, int column, const char* text)
	{
		if (item)
		{
			item->setText(0, text);
		}
	}
}