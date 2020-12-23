#include "QMoveableLabel.h"

#ifdef ECHO_EDITOR_MODE

#include <QToolButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <engine/core/util/StringUtil.h>
#include <engine/core/util/PathUtil.h>
#include "engine/core/editor/editor.h"
#include "engine/core/base/class_method_bind.h"

namespace QT_UI
{
	QMoveableLabel::QMoveableLabel(QWidget* parent)
		: QLabel(parent)
	{
		m_parent = parent;
	}

	void QMoveableLabel::onMinus()
	{
		m_parent->setWindowState(Qt::WindowMinimized);
	}

	void QMoveableLabel::onMaxmized()
	{
		if (m_parent->windowState()!=Qt::WindowMaximized)
			m_parent->setWindowState(Qt::WindowMaximized);
		else
			m_parent->setWindowState(Qt::WindowNoState);
	}

	void QMoveableLabel::onFullScreen()
	{
		if (m_parent->windowState() != Qt::WindowFullScreen)
			m_parent->setWindowState(Qt::WindowFullScreen);
		else
			m_parent->setWindowState(Qt::WindowNoState);
	}

	void QMoveableLabel::mouseMoveEvent(QMouseEvent *e)
	{
		if (e->buttons()==Qt::LeftButton)
		{
			QPointF changedPos = e->screenPos() - m_lastPos;
			QPoint  destPos = m_parent->pos() + QPoint(changedPos.x(), changedPos.y());
			m_parent->move(destPos);

			m_lastPos = e->screenPos();
		}
	}

	void QMoveableLabel::mouseDoubleClickEvent(QMouseEvent *event)
	{
		onMaxmized();
	}

	void QMoveableLabel::mousePressEvent(QMouseEvent *event)
	{
		QLabel::mousePressEvent(event);

		if (event->buttons() == Qt::LeftButton)
		{
			m_lastPos = event->screenPos();
		}
	}

	void QMoveableLabel::mouseReleaseEvent(QMouseEvent *event)
	{
		QLabel::mouseReleaseEvent(event);
	}

	void QMoveableLabel::paintEvent(QPaintEvent* event)
	{
		QLabel::paintEvent(event);
	}
}

#endif