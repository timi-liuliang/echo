#pragma once

#include "../QWidget.h"

#ifdef ECHO_EDITOR_MODE

#include <QLabel>

namespace QT_UI
{
	class QMoveableLabel : public QLabel
	{
	public:
		QMoveableLabel(QWidget* parent);
		~QMoveableLabel(){}

	protected:
		virtual void mouseMoveEvent(QMouseEvent *e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
		virtual void mousePressEvent(QMouseEvent *event) override;
		virtual void mouseReleaseEvent(QMouseEvent *event) override;

	protected:
		void onMinus();
		void onMaxmized();
		void onFullScreen();

	protected:
		// redefine paintEvent
		virtual void paintEvent(QPaintEvent* event) override;

	private:
		QPointF			m_lastPos;
		QWidget*		m_parent;
	};
}

#endif
