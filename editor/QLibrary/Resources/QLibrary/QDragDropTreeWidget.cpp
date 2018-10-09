#include "QDragDropTreeWidget.h"
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	QDragDropTreeWidget::QDragDropTreeWidget(QWidget* parent)
		: QTreeWidget(parent)
	{
	}

	void QDragDropTreeWidget::dragMoveEvent(QDragMoveEvent* event)
	{
		QTreeWidget::dragMoveEvent(event);
	}

	// override drop event
	void QDragDropTreeWidget::dropEvent(QDropEvent *event)
	{
		QModelIndex droppedIndex = indexAt(event->pos());
		if (droppedIndex.isValid())
		{
			QTreeWidget* source = qobject_cast<QTreeWidget*>(event->source());
			QTreeWidgetItem* currentItem = source->currentItem();
			if (currentItem && currentItem->parent())
			{
				QTreeWidget::dropEvent(event);
				emit itemPositionChanged(currentItem);
			}
		}
	}
}