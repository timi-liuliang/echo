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
		}
	}

	void QGraphicsFlowView::mouseMoveEvent(QMouseEvent* event)
	{
		QGraphicsView::mouseMoveEvent(event);
		if (event->button() == Qt::MiddleButton)
		{
			QPointF difference = m_clickPos - mapToScene(event->pos());
			setSceneRect(sceneRect().translated(difference.x(), difference.y()));
		}
	}

	void QGraphicsFlowView::mouseReleaseEvent(QMouseEvent* event)
	{
		QGraphicsView::mouseReleaseEvent(event);
		if (event->button() == Qt::MiddleButton)
		{
		}
	}
}

#endif