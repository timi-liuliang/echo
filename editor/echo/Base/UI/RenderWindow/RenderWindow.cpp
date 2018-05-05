#include <QtGui>
#include "RenderWindow.h"
#include "EchoEngine.h"
#include "DefaultInputController.h"
#include <QDateTime>


namespace Studio
{
	// 构造函数
	RenderWindow::RenderWindow(QWidget* parent/* = NULL*/)
		: QWidget(parent)
		, m_mouseMenu(NULL)
		, m_timer(NULL)
		, m_inputController(NULL)
		, m_defaultInputController(NULL)
		, m_isLeftButtonDown(false)
	{
		setAttribute(Qt::WA_NativeWindow);
		setUpdatesEnabled(false);
		setMouseTracking(true);
	}

	// 析构函数
	RenderWindow::~RenderWindow()
	{
		delete m_timer; m_timer = NULL;

		EchoEngine::instance()->Release();
		delete m_defaultInputController;
	}

	// 开始渲染
	void RenderWindow::BeginRender()
	{
		EchoEngine::instance()->Initialize((HWND)this->winId());

		if (!m_defaultInputController)
			m_defaultInputController = new DefaultInputController; 

		// 时间事件
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(Render()));
		m_timer->start(10);
	}

	// 渲染更新
	void  RenderWindow::Render()
	{
		// 鼠标是否位于窗口中
		ResizeWindow();

		static DWORD lastTime = QDateTime::currentMSecsSinceEpoch();

		// 计算delta Time
		DWORD curTime = QDateTime::currentMSecsSinceEpoch();
		DWORD elapsedTime = curTime - lastTime;

		auto controller = m_inputController ? m_inputController : m_defaultInputController;
		auto pos = controller->mousePosition();
		auto button = controller->pressedMouseButton();
		auto elapsed = elapsedTime * 0.001f;
		InputContext ctx(pos, button, elapsed);

		controller->tick(ctx);

		// Call the main render function
		EchoEngine::instance()->Render(elapsedTime, this->isVisible());

		lastTime = curTime;
	}

	// 窗口大小改变
	void RenderWindow::ResizeWindow()
	{
		
		int width = static_cast<int>(this->width());
		int height = static_cast<int>(this->height());

		if (m_dx9Size.width() != width || m_dx9Size.height() != height)
		{
			EchoEngine::instance()->Resize(width, height);
			m_dx9Size.setWidth(width);
			m_dx9Size.setHeight(height);
		}
	}

	void RenderWindow::SetAspectRatio(const QSize& size)
	{
		m_ratio = size;
	}

	// 重置设备
	void  RenderWindow::ResetDevice()
	{
	}

	// 鼠标滚轮事件
	void RenderWindow::wheelEvent(QWheelEvent * e)
	{
		auto controller = m_inputController ? m_inputController : m_defaultInputController;
		controller->wheelEvent(e);
	}

	// 鼠标移动事件
	void RenderWindow::mouseMoveEvent(QMouseEvent* e)
	{
		// UI事件
		if ( m_isLeftButtonDown)
		{
			// 初始上次位置
			static QPointF lastPos = e->localPos();

			// 计算位移
			QPointF changedPos = e->localPos() - lastPos;

			lastPos = e->localPos();
		}

		auto controller = m_inputController ? m_inputController : m_defaultInputController;
		controller->mouseMoveEvent(e);
	}

	// 鼠标按下事件
	void RenderWindow::mousePressEvent(QMouseEvent* e)
	{
		if (!hasFocus())
		{
			setFocus();
		}

		// UI事件
		if ( e->button()==Qt::LeftButton)
		{
			m_isLeftButtonDown = true;
		}

		// 注册控制器事件
		auto controller = m_inputController ? m_inputController : m_defaultInputController;
		controller->mousePressEvent(e);
	}

	void RenderWindow::mouseDoubleClickEvent(QMouseEvent* e)
	{
		const auto& controller = m_inputController ? m_inputController : m_defaultInputController; 
		controller->mouseDoubleClickEvent(e); 
	}

	// 鼠标释放事件
	void RenderWindow::mouseReleaseEvent(QMouseEvent* e)
	{
		// UI事件
		if (e->button() == Qt::LeftButton)
		{
			m_isLeftButtonDown = false;
		}

		auto controller = m_inputController ? m_inputController : m_defaultInputController;
		controller->mouseReleaseEvent(e);
	}

	// 鼠标按下事件
	void RenderWindow::keyPressEvent(QKeyEvent* e)
	{
		auto controller = m_inputController ? m_inputController : m_defaultInputController;
		controller->keyPressEvent(e);
	}

	// 鼠标抬起事件
	void RenderWindow::keyReleaseEvent(QKeyEvent* e)
	{
		auto controller = m_inputController ? m_inputController : m_defaultInputController;
		controller->keyReleaseEvent(e);
	}
}
