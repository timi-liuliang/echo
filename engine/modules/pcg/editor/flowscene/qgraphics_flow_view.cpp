#include "qgraphics_flow_view.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	QGraphicsFlowView::QGraphicsFlowView(QWidget* parent)
		: QGraphicsView(parent)
	{
		//setDragMode(QGraphicsView::ScrollHandDrag);
		setRenderHint(QPainter::Antialiasing);
		setContextMenuPolicy(Qt::CustomContextMenu);
	}

	QGraphicsFlowView::~QGraphicsFlowView()
	{

	}

	void QGraphicsFlowView::mousePressEvent(QMouseEvent* event)
	{
		QGraphicsView::mousePressEvent(event);
		if (event->button() == Qt::MiddleButton)
		{
			m_clickPos = mapToScene(event->pos());
			m_isMiddleButtonDown = true;
		}
	}

	void QGraphicsFlowView::mouseMoveEvent(QMouseEvent* event)
	{
		QGraphicsView::mouseMoveEvent(event);
		if (m_isMiddleButtonDown)
		{
			QPointF newPos = mapToScene(event->pos());
			QPointF difference = m_clickPos - newPos;
			if (!difference.isNull())
			{
				QRectF  modifiedSceneRect = sceneRect().translated(difference.x(), difference.y());
				setSceneRect(modifiedSceneRect);
			}
		}
	}

	void QGraphicsFlowView::mouseReleaseEvent(QMouseEvent* event)
	{
		QGraphicsView::mouseReleaseEvent(event);
		if (event->button() == Qt::MiddleButton)
		{
			m_isMiddleButtonDown = false;
		}
	}
}

#endif