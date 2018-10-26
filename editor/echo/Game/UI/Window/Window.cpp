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
		, m_app(nullptr)
		, m_timer(nullptr)
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
		m_app->init((HWND)winId(), echoProject);

		// 设置初始宽高
		Echo::i32 thisW = width();
		Echo::i32 thisH = height();
		Echo::i32 mainW = GameMainWindow::instance()->width();
		Echo::i32 mainH = GameMainWindow::instance()->height();
		Echo::i32 aimW = Echo::GameSettings::instance()->getWindowWidth();
		Echo::i32 aimH = Echo::GameSettings::instance()->getWindowHeight();
		Echo::i32 mainNewWidth = mainW + (aimW - thisW);
		Echo::i32 mainNewHeight = mainH + (aimH - thisH);
		GameMainWindow::instance()->resize(mainNewWidth, mainNewHeight);

		// 居中
		GameMainWindow::instance()->moveToCenter();

		// 时间事件
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(Render()));
		m_timer->start(10);
	}

	void  Window::Render()
	{
		// 鼠标是否位于窗口中
		checkWindowSize();

		static DWORD lastTime = QDateTime::currentMSecsSinceEpoch();

		// 计算delta Time
		DWORD curTime = QDateTime::currentMSecsSinceEpoch();
		DWORD elapsedTime = curTime - lastTime;

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

	void Window::SetAspectRatio(const QSize& size)
	{
		m_ratio = size;
	}

	void  Window::ResetDevice()
	{
	}

	void Window::wheelEvent(QWheelEvent * e)
	{
		//auto controller = m_inputController ? m_inputController : m_defaultInputController;
		//controller->wheelEvent(e);
	}

	void Window::mouseMoveEvent(QMouseEvent* e)
	{
		// UI事件
		//if ( m_isLeftButtonDown)
		//{
			// 初始上次位置
		//	static QPointF lastPos = e->localPos();

			// 计算位移
		//	QPointF changedPos = e->localPos() - lastPos;

		//	lastPos = e->localPos();
		//}

		//auto controller = m_inputController ? m_inputController : m_defaultInputController;
		//controller->mouseMoveEvent(e);
	}

	void Window::mousePressEvent(QMouseEvent* e)
	{
		Echo::ui32 buttonId = e->button() == Qt::LeftButton ? 0 : ( e->button() == Qt::RightButton ? 1 : 2);
		Echo::Input::instance()->notifyMouseButtonDown(buttonId);
	}

	void Window::mouseDoubleClickEvent(QMouseEvent* e)
	{
		//const auto& controller = m_inputController ? m_inputController : m_defaultInputController; 
		//controller->mouseDoubleClickEvent(e); 
	}

	void Window::mouseReleaseEvent(QMouseEvent* e)
	{
		Echo::ui32 buttonId = e->button() == Qt::LeftButton ? 0 : (e->button() == Qt::RightButton ? 1 : 2);
		Echo::Input::instance()->notifyMouseButtonUp(buttonId);
	}

	void Window::keyPressEvent(QKeyEvent* e)
	{
		//auto controller = m_inputController ? m_inputController : m_defaultInputController;
		//controller->keyPressEvent(e);
	}

	void Window::keyReleaseEvent(QKeyEvent* e)
	{
		//auto controller = m_inputController ? m_inputController : m_defaultInputController;
		//controller->keyReleaseEvent(e);
	}
}
