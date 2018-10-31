#include <QApplication>
#include <QDesktopWidget>
#include "About.h"
#include "engine/core/util/PathUtil.h"

namespace Studio
{
	AboutWindow::AboutWindow(QWidget* parent/*=0*/)
		: QMainWindow( parent)
	{
		setupUi( this);

		// 隐藏标题
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

		// 设置菜单左上控件
		menubar->setTopLeftCornerIcon(":/icon/Icon/icon.png");

	}

	AboutWindow::~AboutWindow()
	{
	}
}