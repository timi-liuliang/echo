#pragma once

#include <engine/core/editor/qt/QWidget.h>

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class QGraphicsSceneEx : public QGraphicsScene
	{
	protected:
		virtual void keyReleaseEvent(QKeyEvent* event) override
		{
			if (event->key() == Qt::Key_Delete)
			{
				int a = 10;

				//QList<QGraphicsItem*> selectedItems = selectedItems();
				//foreach(QGraphicsItem * item, selectedItems)
				//{
				//	removeItem(item);
				//	delete item;
				//}
			}
		}
	};
#endif
}
