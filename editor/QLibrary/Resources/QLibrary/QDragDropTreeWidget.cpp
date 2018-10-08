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
		QTreeWidget* source = qobject_cast<QTreeWidget*>( event->source());
		QTreeWidgetItem* currentItem = source->currentItem();
		if (currentItem)
		{
			QTreeWidget::dropEvent(event);
			emit itemPositionChanged(currentItem);
		}
	}
}