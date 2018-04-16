#include "MainWindow.h"
#include "Studio.h"
#include "LogPanel.h"
#include "ResPanel.h"
#include "NodeTreePanel.h"
#include <QFileDialog>
#include <QDesktopservices>
#include <QShortcut>
#include "TimelinePanel.h"
#include "DebuggerPanel.h"

namespace Studio
{
	// 构造函数
	MainWindow::MainWindow(QMainWindow* parent/*=0*/)
		: QMainWindow( parent)
		, m_resPanel(nullptr)
		, m_timelinePanel(nullptr)
		, m_debuggerPanel(nullptr)
	{
		setupUi( this);

		// 隐藏标题
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

		// 设置菜单左上控件
		menubar->setTopLeftCornerIcon(":/icon/Icon/icon.png");

		// connect signal slot
		QObject::connect(m_actionSave, SIGNAL(triggered(bool)), this, SLOT(onSaveProject()));
	}

	// 析构函数
	MainWindow::~MainWindow()
	{
	}

	// 打开项目时调用
	void MainWindow::onOpenProject()
	{
		m_resPanel = EchoNew(ResPanel(this));
		m_scenePanel = EchoNew(NodeTreePanel(this));
		m_timelinePanel = EchoNew(TimelinePanel(this));
		m_debuggerPanel = EchoNew(DebuggerPanel(this));

		setCentralWidget(AStudio::Instance()->getRenderWindow());

		this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
		this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

		this->addDockWidget(Qt::LeftDockWidgetArea, m_resPanel);
		this->addDockWidget(Qt::RightDockWidgetArea, m_scenePanel);
		
		this->addDockWidget(Qt::BottomDockWidgetArea, AStudio::Instance()->getLogPanel());
		this->addDockWidget(Qt::BottomDockWidgetArea, m_debuggerPanel);
		this->addDockWidget(Qt::BottomDockWidgetArea, m_timelinePanel);

		this->tabifyDockWidget(AStudio::Instance()->getLogPanel(), m_debuggerPanel);
		this->tabifyDockWidget(m_debuggerPanel, m_timelinePanel);

		m_resPanel->onOpenProject();
	}

	// 保存文件
	void MainWindow::onSaveProject()
	{
		int  a = 10;
	}

	// 打开文件
	void MainWindow::OpenProject(const char* projectName)
	{
		AStudio::Instance()->OpenProject(projectName);

		// 初始化渲染窗口
		AStudio::Instance()->getRenderWindow();
	}

	void MainWindow::closeEvent(QCloseEvent *event)
	{
		AStudio::Instance()->getLogPanel()->close();
	}
}