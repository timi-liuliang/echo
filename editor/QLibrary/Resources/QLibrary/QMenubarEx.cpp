#include "QMenubarEx.h"
#include <QToolButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <engine/core/util/StringUtil.h>
#include <engine/core/util/PathUtil.h>

namespace QT_UI
{
	QMenuBarEx::QMenuBarEx(QWidget* parent)
		: QMenuBar(parent)
	{
		m_parent = dynamic_cast<QMainWindow*>(parent);

		// set menu bar corner icon
		QWidget* menuTopRightButton = new QWidget(this);
		QToolButton* minusButton = new QToolButton(menuTopRightButton);
		minusButton->setIcon(QIcon(":/icon/Icon/minus.png"));
		minusButton->setStyleSheet("background-color: #00535353;");

		QToolButton* minimizeButton = new QToolButton(menuTopRightButton);
		minimizeButton->setIcon(QIcon(":/icon/Icon/minimize.png"));
		minimizeButton->setStyleSheet("background-color: #00535353;");

		QToolButton* fullScreenButton = new QToolButton(menuTopRightButton);
		fullScreenButton->setIcon(QIcon(":/icon/Icon/fullscreen.png"));
		fullScreenButton->setStyleSheet("background-color: #00535353;");

		QToolButton* closeButton = new QToolButton(menuTopRightButton);
		closeButton->setIcon(QIcon(":/icon/Icon/close.png"));
		closeButton->setStyleSheet("background-color: #00535353;");

		QHBoxLayout* horizontalLayout = new QHBoxLayout(menuTopRightButton);
		horizontalLayout->setSpacing(0);
		horizontalLayout->setObjectName(QStringLiteral("horizontalLayoutTR"));
		horizontalLayout->setContentsMargins(0, 0, 0, 0);
		horizontalLayout->addWidget(minusButton);
		horizontalLayout->addWidget(minimizeButton);
		horizontalLayout->addWidget(fullScreenButton);
		horizontalLayout->addWidget(closeButton);

		setCornerWidget(menuTopRightButton, Qt::TopRightCorner);
		
		// connect signal slots
		QObject::connect(minusButton, SIGNAL(clicked()), this, SLOT(onMinus()));
		QObject::connect(minimizeButton, SIGNAL(clicked()), this, SLOT(onMaxmized()));
		QObject::connect(fullScreenButton, SIGNAL(clicked()), this, SLOT(onFullScreen()));
		QObject::connect(closeButton, SIGNAL(clicked()), parent, SLOT(close()));
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
			menuTopLeftButton->setIcon(QIcon(pixmap));
		}

		menuTopLeftButton->setIconSize(QSize(22, 22));
		menuTopLeftButton->setStyleSheet("background-color: #00535353;");
		setCornerWidget(menuTopLeftButton, Qt::TopLeftCorner);
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
