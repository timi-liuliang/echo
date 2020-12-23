#include "QMenuBarEx.h"

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
	QMenuBarEx::QMenuBarEx(QWidget* parent)
		: QMenuBar(parent)
	{
		m_parent = parent;

		// set menu bar corner icon
		QWidget* menuTopRightButton = new QWidget(this);
		m_minusButton = new QToolButton(menuTopRightButton);
		m_minusButton->setIcon(QIcon(":/icon/Icon/minus.png"));
		m_minusButton->setStyleSheet("background-color: #00535353;");
		m_minusButton->setToolTip("Minimize");

		m_minimizeButton = new QToolButton(menuTopRightButton);
		m_minimizeButton->setIcon(QIcon(":/icon/Icon/minimize.png"));
		m_minimizeButton->setStyleSheet("background-color: #00535353;");
		m_minimizeButton->setToolTip("Maximize");

		m_fullScreenButton = new QToolButton(menuTopRightButton);
		m_fullScreenButton->setIcon(QIcon(":/icon/Icon/fullscreen.png"));
		m_fullScreenButton->setStyleSheet("background-color: #00535353;");
		m_fullScreenButton->setToolTip("FullScreen");

		m_closeButton = new QToolButton(menuTopRightButton);
		m_closeButton->setIcon(QIcon(":/icon/Icon/close.png"));
		m_closeButton->setStyleSheet("background-color: #00535353;");
		m_closeButton->setToolTip("Close");

		QHBoxLayout* horizontalLayout = new QHBoxLayout(menuTopRightButton);
		horizontalLayout->setSpacing(0);
		horizontalLayout->setObjectName(QStringLiteral("horizontalLayoutTR"));
		horizontalLayout->setContentsMargins(0, 0, 0, 0);
		horizontalLayout->addWidget(m_minusButton);
		horizontalLayout->addWidget(m_minimizeButton);
		horizontalLayout->addWidget(m_fullScreenButton);
		horizontalLayout->addWidget(m_closeButton);

		setCornerWidget(menuTopRightButton, Qt::TopRightCorner);
		
		// connect signal slots
		EditorApi.qConnectWidget(m_minusButton, SIGNAL(clicked()), this, Echo::createMethodBind(&QMenuBarEx::onMinus));
		EditorApi.qConnectWidget(m_minimizeButton, SIGNAL(clicked()), this, Echo::createMethodBind(&QMenuBarEx::onMaxmized));
		EditorApi.qConnectWidget(m_fullScreenButton, SIGNAL(clicked()), this, Echo::createMethodBind(&QMenuBarEx::onFullScreen));
		EditorApi.qConnectWidget(m_closeButton, SIGNAL(clicked()), parent, Echo::createMethodBind(&QMenuBarEx::close));
	}

	void QMenuBarEx::setTopLeftCornerIcon(const char* icon)
	{
		QToolButton* menuTopLeftButton = new QToolButton(this);
		if (Echo::StringUtil::StartWith( icon,":/"))
		{
			menuTopLeftButton->setIcon(QIcon(icon));
		}
		else
		{
			QPixmap pixmap(icon);
			pixmap.scaled(QSize(22, 22));
			menuTopLeftButton->setIcon(QIcon(pixmap));
		}

		menuTopLeftButton->setIconSize(QSize(22, 22));
		menuTopLeftButton->setStyleSheet("background-color: #00535353;");
		setCornerWidget(menuTopLeftButton, Qt::TopLeftCorner);
	}

	void QMenuBarEx::setCornderButtonVisible(QMenuBarEx::ButtonType type, bool visible)
	{
		if		(type == Minus)		m_minusButton->setVisible(visible);
		else if (type == Minimize)	m_minimizeButton->setVisible(visible);
		else if (type == FullScreen)m_fullScreenButton->setVisible(visible);
		else if (type == Close)		m_closeButton->setVisible(visible);
	}

	void QMenuBarEx::onMinus()
	{
		m_parent->setWindowState(Qt::WindowMinimized);
	}

	void QMenuBarEx::onMaxmized()
	{
		if (m_parent->windowState()!=Qt::WindowMaximized)
			m_parent->setWindowState(Qt::WindowMaximized);
		else
			m_parent->setWindowState(Qt::WindowNoState);
	}

	void QMenuBarEx::onFullScreen()
	{
		if (m_parent->windowState() != Qt::WindowFullScreen)
			m_parent->setWindowState(Qt::WindowFullScreen);
		else
			m_parent->setWindowState(Qt::WindowNoState);
	}

	void QMenuBarEx::mouseMoveEvent(QMouseEvent *e)
	{
		if (e->buttons()==Qt::LeftButton)
		{
			QPointF changedPos = e->screenPos() - m_lastPos;
			QPoint  destPos = m_parent->pos() + QPoint(changedPos.x(), changedPos.y());
			m_parent->move(destPos);

			m_lastPos = e->screenPos();
		}
	}

	void QMenuBarEx::mouseDoubleClickEvent(QMouseEvent *event)
	{
		QAction* action = QMenuBar::actionAt(event->pos());
		if (!action)
		{
			onMaxmized();
		}
	}

	void QMenuBarEx::mousePressEvent(QMouseEvent *event)
	{
		QMenuBar::mousePressEvent(event);

		if (event->buttons() == Qt::LeftButton)
		{
			m_lastPos = event->screenPos();
		}
	}

	void QMenuBarEx::mouseReleaseEvent(QMouseEvent *event)
	{
		QMenuBar::mouseReleaseEvent(event);
	}

	void QMenuBarEx::paintEvent(QPaintEvent* event)
	{
		QMenuBar::paintEvent(event);
	}
}

#endif