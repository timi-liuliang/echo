#include <QApplication>
#include <QDesktopWidget>
#include <QStatusBar>
#include "GameMainWindow.h"
#include "WindowMetal.h"
#include "TerminalDialog.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/main/game_settings.h"
#include "MacHelper.h"

namespace Game
{
	static GameMainWindow* g_inst = nullptr;

	GameMainWindow::GameMainWindow(QWidget* parent/*=0*/)
		: QMainWindow( parent)
		, m_renderWindow(nullptr)
	{
		m_studio = Studio::AStudio::instance();

		setupUi( this);

		// window
		QObject::connect(m_actionTerminal, SIGNAL(triggered(bool)), this, SLOT(onOpenWindow()));

#ifdef ECHO_PLATFORM_MAC
        m_renderWindow = new WindowMetal(this);
#else
        m_renderWindow = new Window(this);
#endif

		setCentralWidget(m_renderWindow);;

#ifdef ECHO_PLATFORM_WINDOWS
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
		menubar->setNativeMenuBar(false);
#endif
		// set icon
		menubar->setTopLeftCornerIcon(":/icon/Icon/icon.png");

		assert(!g_inst);
		g_inst = this;
	}

	GameMainWindow::~GameMainWindow()
	{
		EchoSafeDelete(m_studio, AStudio);
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

		// windows size
		configWindowSizes();

		// show status message
		QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(onShowStatusMessage()));
		m_timer.start(500);
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
		m_windowSizes.emplace_back(Echo::GameSettings::instance()->getWindowWidth(), Echo::GameSettings::instance()->getWindowHeight(), "Design");
		m_windowSizes.emplace_back(320, 568, "iPhoneSE @2x");
		m_windowSizes.emplace_back(375, 667, "iPhone6s iPhone7 iPhone8 @2x");
		m_windowSizes.emplace_back(414, 736, "iPhone6s Plus iPhone7 Plus iPhone8 Plus @3x");
		m_windowSizes.emplace_back(375, 812, "iPhoneX iPhoneXs @3x");
		m_windowSizes.emplace_back(414, 896, "iPhoneXR @2x");
		m_windowSizes.emplace_back(414, 896, "iPhoneXs Max @3x");

		m_windowSizes.emplace_back(768, 1024, "iPad Mini, iPad @2x");
		m_windowSizes.emplace_back(834, 1112, "10.5 iPad pro @2x");
		m_windowSizes.emplace_back(834, 1194, "11 iPad pro @2x");
		m_windowSizes.emplace_back(1024, 1366,"12.9 iPad pro @2x");

		// Horizontal
		for (size_t i = 0; i < m_windowSizes.size(); i++)
		{
			QAction* action = new QAction(Echo::StringUtil::Format("%s [%dx%d]", m_windowSizes[i].m_displayText.c_str(), m_windowSizes[i].m_height, m_windowSizes[i].m_width).c_str(), this);
			action->setData(int(i));
			m_menuHorizontal->addAction(action);

			QObject::connect(action, SIGNAL(triggered()), this, SLOT(onSwitchResolutionHorizontal()));
		}

		// Vertical
		for (size_t i=0; i<m_windowSizes.size(); i++)
		{
			QAction* action = new QAction(Echo::StringUtil::Format("%s [%dx%d]", m_windowSizes[i].m_displayText.c_str(), m_windowSizes[i].m_width, m_windowSizes[i].m_height).c_str(), this);
			action->setData(int(i));
			m_menuVertical->addAction(action);

			QObject::connect(action, SIGNAL(triggered()), this, SLOT(onSwitchResolutionVertical()));
		}
	}

	void GameMainWindow::onSwitchResolutionHorizontal()
	{
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
		{
			Echo::i32 index = action->data().toInt();
			setRenderWindowSize(m_windowSizes[index].m_height, m_windowSizes[index].m_width);
		}
	}

	void GameMainWindow::onSwitchResolutionVertical()
	{
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
		{
			Echo::i32 index = action->data().toInt();
			setRenderWindowSize(m_windowSizes[index].m_width, m_windowSizes[index].m_height);
		}
	}

	void GameMainWindow::onShowStatusMessage()
	{
		Echo::i32 fps = Echo::FrameState::instance()->getFps();
		Echo::i32 drawcall = Echo::FrameState::instance()->getDrawCalls();
		statusBar()->showMessage(Echo::StringUtil::Format("Fps:%d Drawcall:%d", fps, drawcall).c_str());
	}

	void GameMainWindow::onOpenWindow()
	{
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
		{
			Echo::String windowName = action->text().toStdString().c_str();
			if (Echo::StringUtil::Equal(windowName, "Terminal"))
			{
				Studio::TerminalDialog* inst = Studio::TerminalDialog::instance();
				inst->setVisible(true);
				if (inst->exec() == QDialog::Accepted)
				{
				}
			}
		}
	}
}
