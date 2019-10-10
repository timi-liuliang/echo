#include <QtGui>
#include "Window.h"
#include <QDateTime>
#include "GameMainWindow.h"
#include <engine/core/main/GameSettings.h>
#include <engine/core/input/input.h>

namespace Game
{
	Window::Window(QWidget* parent/* = NULL*/)
		: QWidget(parent)
	{
		setUpdatesEnabled(false);
		setMouseTracking(true);

		m_app = new App;
	}

	Window::~Window()
	{
	}

	void Window::start(const Echo::String& echoProject)
	{
		m_app->init((size_t)winId(), echoProject);

		Echo::i32 thisW = width();
		Echo::i32 thisH = height();
		Echo::i32 mainW = GameMainWindow::instance()->width();
		Echo::i32 mainH = GameMainWindow::instance()->height();
		Echo::i32 aimW = Echo::GameSettings::instance()->getWindowWidth();
		Echo::i32 aimH = Echo::GameSettings::instance()->getWindowHeight();
		Echo::i32 mainNewWidth = mainW + (aimW - thisW);
		Echo::i32 mainNewHeight = mainH + (aimH - thisH);
		GameMainWindow::instance()->resize(mainNewWidth, mainNewHeight);

		GameMainWindow::instance()->moveToCenter();

		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(render()));
		m_timer->start(10);
	}

	void  Window::render()
	{
		checkWindowSize();

        static Echo::Dword lastTime = QDateTime::currentMSecsSinceEpoch();

		// calc delta Time
        Echo::Dword curTime = QDateTime::currentMSecsSinceEpoch();
        Echo::Dword elapsedTime = curTime - lastTime;

		// Call the main render function
		m_app->tick(elapsedTime * 0.001f);

		lastTime = curTime;
	}

	void Window::checkWindowSize()
	{	
		int width = static_cast<int>(this->width());
		int height = static_cast<int>(this->height());

		if (m_dx9Size.width() != width || m_dx9Size.height() != height)
		{
			m_app->onSize(width, height);
			m_dx9Size.setWidth(width);
			m_dx9Size.setHeight(height);
		}
	}

	void Window::setAspectRatio(const QSize& size)
	{
		m_ratio = size;
	}

	void  Window::resetDevice()
	{
	}

	void Window::wheelEvent(QWheelEvent * e)
	{

	}

	void Window::mouseMoveEvent(QMouseEvent* e)
	{
	}

	void Window::mousePressEvent(QMouseEvent* e)
	{
		Echo::ui32 buttonId = e->button() == Qt::LeftButton ? 0 : ( e->button() == Qt::RightButton ? 1 : 2);
		const QPointF& windowPos = e->windowPos();
		Echo::Input::instance()->notifyMouseButtonDown(buttonId, Echo::Vector2(windowPos.x(), windowPos.y()));
	}

	void Window::mouseDoubleClickEvent(QMouseEvent* e)
	{

	}

	void Window::mouseReleaseEvent(QMouseEvent* e)
	{
		Echo::ui32 buttonId = e->button() == Qt::LeftButton ? 0 : (e->button() == Qt::RightButton ? 1 : 2);
		const QPointF& windowPos = e->windowPos();
		Echo::Input::instance()->notifyMouseButtonUp(buttonId, Echo::Vector2(windowPos.x(), windowPos.y()));
	}

	void Window::keyPressEvent(QKeyEvent* e)
	{

	}

	void Window::keyReleaseEvent(QKeyEvent* e)
	{

	}
}
