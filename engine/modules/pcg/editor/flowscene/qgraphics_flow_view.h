#pragma once

#include <engine/core/editor/qt/QWidget.h>

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	class QGraphicsFlowView : public QGraphicsView
	{
	public:
		QGraphicsFlowView(QWidget* parent=nullptr);
		virtual ~QGraphicsFlowView();

	protected:
		// mouse event
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;

	protected:
		QPointF     m_clickPos;
	};
}

#endif
