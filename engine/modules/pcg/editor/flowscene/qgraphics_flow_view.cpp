#include "qgraphics_flow_view.h"
#include "qgraphics_flow_scene.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	QGraphicsFlowView::QGraphicsFlowView(QWidget* parent)
		: QGraphicsView(parent)
	{
		setRenderHint(QPainter::Antialiasing);
		setContextMenuPolicy(Qt::CustomContextMenu);
	}

	QGraphicsFlowView::~QGraphicsFlowView()
	{

	}

	void QGraphicsFlowView::mousePressEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::LeftButton)
		{
			QGraphicsFlowScene* flowScene = dynamic_cast<QGraphicsFlowScene*>(scene());
			if (flowScene)
			{
				QGraphicsItem* item = flowScene->itemAt(mapToScene(event->pos()), QTransform());
				if (!item || item->data(Qt::UserRole).toString() == "bg")
				{
					setDragMode(QGraphicsView::ScrollHandDrag);
				}
			}
		}

		QGraphicsView::mousePressEvent(event);
	}

	void QGraphicsFlowView::mouseMoveEvent(QMouseEvent* event)
	{
		QGraphicsView::mouseMoveEvent(event);
	}

	void QGraphicsFlowView::mouseReleaseEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::LeftButton)
		{
			setDragMode(QGraphicsView::NoDrag);
		}

		QGraphicsView::mouseReleaseEvent(event);
	}
}

#endif