#include "GameMainWindow.h"

namespace Game
{
	// 构造函数
	GameMainWindow::GameMainWindow(QWidget* parent/*=0*/)
		: QMainWindow( parent)
		, m_renderWindow(nullptr)
	{
		setupUi( this);

		m_renderWindow = new Window(this);
		setCentralWidget(m_renderWindow);;

		// 隐藏标题
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

		// 设置菜单左上控件
		menubar->setTopLeftCornerIcon(":/icon/Icon/icon.png");

		resize(800, 490);
	}

	// 析构函数
	GameMainWindow::~GameMainWindow()
	{
	}

	// 开始渲染
	void GameMainWindow::start(const Echo::String& echoProject)
	{
		setWindowTitle(echoProject.c_str());
		m_renderWindow->start(echoProject);
	}
}