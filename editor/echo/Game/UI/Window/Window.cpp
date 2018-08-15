#include <QtGui>
#include "Window.h"
#include <QDateTime>
#include "GameMainWindow.h"
#include <engine/core/main/GameSettings.h>
#include <engine/core/input/input.h>

namespace Game
{
	// 构造函数
	Window::Window(QWidget* parent/* = NULL*/)
		: QWidget(parent)
		, m_app(nullptr)
		, m_timer(nullptr)
	{
		setUpdatesEnabled(false);
		setMouseTracking(true);

		m_app = new App;
	}

	// 析构函数
	Window::~Window()
	{
	}

	// 开始渲染
	void Window::start(const Echo::String& echoProject)
	{
		m_app->init((HWND)winId(), echoProject);

		// 设置初始宽高
		Echo::i32 thisW = width();
		Echo::i32 thisH = height();
		Echo::i32 mainW = GameMainWindow::instance()->width();
		Echo::i32 mainH = GameMainWindow::instance()->height();
		Echo::i32 aimW = Echo::GameSettings::instance()->getDesignWidth();
		Echo::i32 aimH = Echo::GameSettings::instance()->getDesignHeight();
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

	// 渲染更新
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

	// 窗口大小改变
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

	// 重置设备
	void  Window::ResetDevice()
	{
	}

	// 鼠标滚轮事件
	void Window::wheelEvent(QWheelEvent * e)
	{
		//auto controller = m_inputController ? m_inputController : m_defaultInputController;
		//controller->wheelEvent(e);
	}

	// 鼠标移动事件
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

	// 鼠标按下事件
	void Window::mousePressEvent(QMouseEvent* e)
	{
		Echo::Input::instance()->notifyMouseButtonDown(0);
	}

	void Window::mouseDoubleClickEvent(QMouseEvent* e)
	{
		//const auto& controller = m_inputController ? m_inputController : m_defaultInputController; 
		//controller->mouseDoubleClickEvent(e); 
	}

	// 鼠标释放事件
	void Window::mouseReleaseEvent(QMouseEvent* e)
	{
		Echo::Input::instance()->notifyMouseButtonUp(0);
	}

	// 鼠标按下事件
	void Window::keyPressEvent(QKeyEvent* e)
	{
		//auto controller = m_inputController ? m_inputController : m_defaultInputController;
		//controller->keyPressEvent(e);
	}

	// 鼠标抬起事件
	void Window::keyReleaseEvent(QKeyEvent* e)
	{
		//auto controller = m_inputController ? m_inputController : m_defaultInputController;
		//controller->keyReleaseEvent(e);
	}
}
