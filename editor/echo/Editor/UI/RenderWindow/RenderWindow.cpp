#include <QtGui>
#include "RenderWindow.h"
#include "EchoEngine.h"
#include "MainWindow.h"
#include "InputController2d.h"
#include "InputController3d.h"
#include "OperationManager.h"
#include <QDateTime>

namespace Studio
{
	RenderWindow::RenderWindow(QWidget* parent/* = NULL*/)
		: QWidget(parent)
	{
		setupUi(this);

		setAttribute(Qt::WA_NativeWindow);
		setUpdatesEnabled(false);
		setMouseTracking(true);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	RenderWindow::~RenderWindow()
	{
		delete m_timer; m_timer = NULL;

		delete m_inputController2d;
		delete m_inputController3d;
		EchoSafeDelete(m_transformWidget, TransformWidget);
	}

	QSize RenderWindow::sizeHint() const
	{
		float width = MainWindow::instance()->width() * 0.3f;
		float height = MainWindow::instance()->height() * 0.65f;
		return QSize( width, height);
	}

	void RenderWindow::BeginRender()
	{
		EchoEngine::instance()->Initialize((size_t)this->winId());

		if (!m_inputController2d)
			m_inputController2d = new InputController2d;

		if (!m_inputController3d)
			m_inputController3d = new InputController3d;

		m_inputController = m_inputController2d;

		if (!m_transformToolBar)
		{
			m_transformToolBar = new QToolBar(nullptr);
			m_transformToolBar->addAction(m_actionTransition);
			m_transformToolBar->addAction(m_actionRotation);
			m_transformToolBar->addAction(m_actionScale);

			MainWindow::instance()->addToolBar(m_transformToolBar);
		}

		if (!m_transformWidget)
			m_transformWidget = EchoNew(TransformWidget);

		m_timer = new QTimer(this);
		QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(Render()));
		m_timer->start(10);
	}

	void  RenderWindow::Render()
	{
		ResizeWindow();

        static Echo::Dword lastTime = QDateTime::currentMSecsSinceEpoch();

		// calc delta Time
        Echo::Dword curTime = QDateTime::currentMSecsSinceEpoch();
        Echo::Dword elapsedTime = curTime - lastTime;

		QPointF pos = m_inputController->mousePosition();
		Qt::MouseButton button = m_inputController->pressedMouseButton();
		float elapsed = elapsedTime * 0.001f;
		InputContext ctx(pos, button, elapsed);

		m_inputController->tick(ctx);

		// operations
		OperationManager::instance()->tick();

		// Call the main render function
		EchoEngine::instance()->Render(elapsed, this->isVisible());

		lastTime = curTime;
	}

	void RenderWindow::ResizeWindow()
	{		
		int width = static_cast<int>(this->width());
		int height = static_cast<int>(this->height());

		if (m_size.width() != width || m_size.height() != height)
		{
			EchoEngine::instance()->Resize(width, height);
			m_size.setWidth(width);
			m_size.setHeight(height);
		}
	}

	void RenderWindow::SetAspectRatio(const QSize& size)
	{
		m_ratio = size;
	}

	void  RenderWindow::ResetDevice()
	{
	}

	void RenderWindow::wheelEvent(QWheelEvent * e)
	{
		m_inputController->wheelEvent(e);
	}

	void RenderWindow::mouseMoveEvent(QMouseEvent* e)
	{
		m_transformWidget->onMouseMove(Echo::Vector2(e->localPos().x(), e->localPos().y()));
		m_inputController->mouseMoveEvent(e);
	}

	void RenderWindow::mousePressEvent(QMouseEvent* e)
	{
		if (!hasFocus())
		{
			setFocus();
		}

		if ( e->button()==Qt::LeftButton)
		{
			m_isLeftButtonDown = true;
		}

		m_transformWidget->onMouseDown(Echo::Vector2(e->localPos().x(), e->localPos().y()));
		m_inputController->mousePressEvent(e);
	}

	void RenderWindow::mouseDoubleClickEvent(QMouseEvent* e)
	{
		m_inputController->mouseDoubleClickEvent(e);
	}

	void RenderWindow::mouseReleaseEvent(QMouseEvent* e)
	{
		if (e->button() == Qt::LeftButton)
		{
			m_isLeftButtonDown = false;
		}

		m_transformWidget->onMouseUp();
		m_inputController->mouseReleaseEvent(e);
	}

	void RenderWindow::keyPressEvent(QKeyEvent* e)
	{
		m_inputController->keyPressEvent(e);
	}

	void RenderWindow::keyReleaseEvent(QKeyEvent* e)
	{
		m_inputController->keyReleaseEvent(e);
	}
}
