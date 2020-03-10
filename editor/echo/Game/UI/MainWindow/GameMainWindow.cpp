#include <QApplication>
#include <QDesktopWidget>
#include "GameMainWindow.h"
#include "engine/core/util/PathUtil.h"
#include "MacHelper.h"

namespace Game
{
	static GameMainWindow* g_inst = nullptr;

	GameMainWindow::GameMainWindow(QWidget* parent/*=0*/)
		: QMainWindow( parent)
		, m_renderWindow(nullptr)
	{
		setupUi( this);

		m_renderWindow = new Window(this);
		setCentralWidget(m_renderWindow);;

#ifdef ECHO_PLATFORM_WINDOWS
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
		menubar->setNativeMenuBar(false);
#endif

		// set icon
		menubar->setTopLeftCornerIcon(":/icon/Icon/icon.png");

		// windows size
		configWindowSizes();

		assert(!g_inst);
		g_inst = this;

	}

	GameMainWindow::~GameMainWindow()
	{
	}

	GameMainWindow* GameMainWindow::instance()
	{
		return g_inst;
	}

	void GameMainWindow::start(const Echo::String& echoProject)
	{
		Echo::String iconPath = Echo::PathUtil::GetFileDirPath(echoProject);

		//setWindowTitle(echoProject.c_str());
		//menubar->setTopLeftCornerIcon((iconPath + "icon.png").c_str());

		m_renderWindow->start(echoProject);
	}

	void GameMainWindow::moveToCenter()
	{
		move((qApp->desktop()->availableGeometry().width() - width()) / 2 + qApp->desktop()->availableGeometry().x(),
			(qApp->desktop()->availableGeometry().height() - height()) / 2 + qApp->desktop()->availableGeometry().y());
	}

	void GameMainWindow::setRenderWindowSize(Echo::i32 width, Echo::i32 height)
	{
		Echo::i32 thisW = m_renderWindow->width();
		Echo::i32 thisH = m_renderWindow->height();
		Echo::i32 mainW = this->width();
		Echo::i32 mainH = this->height();
		Echo::i32 aimW = width;
		Echo::i32 aimH = height;
		Echo::i32 mainNewWidth = mainW + (aimW - thisW);
		Echo::i32 mainNewHeight = mainH + (aimH - thisH);

		resize(mainNewWidth, mainNewHeight);
		moveToCenter();
	}

	void GameMainWindow::configWindowSizes()
	{
		QAction* Action = new QAction("Deisgn", this);
		m_menuSize->addAction(Action);

		QObject::connect(Action, SIGNAL(triggered()), this, SLOT(onSwitchResolution()));
	}

	void GameMainWindow::onSwitchResolution()
	{
		setRenderWindowSize(1024, 768);
	}
}
