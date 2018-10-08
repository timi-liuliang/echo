#include "QDragDropTreeWidget.h"
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	QDragDropTreeWidget::QDragDropTreeWidget(QWidget* parent)
		: QTreeWidget(parent)
	{

	}

	// override drop event
	void QDragDropTreeWidget::dropEvent(QDropEvent *event)
	{
		QModelIndex index = indexAt(event->pos());
		if (index.isValid())
		{
			QTreeWidgetItem* item = itemFromIndex(index);
			if (item)
			{
				emit itemChildrenChanged(item);

				QTreeWidget::dropEvent(event);
			}
		}
	}
}