#include <QApplication>
#include <QDesktopWidget>
#include "BuildWindow.h"
#include "engine/core/util/PathUtil.h"
#include "MacHelper.h"

namespace Studio
{
	BuildWindow::BuildWindow(QWidget* parent/*=0*/)
		: QMainWindow( parent)
	{
		setupUi( this);

#ifdef ECHO_PLATFORM_WINDOWS
        // hide window hwnd
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
        // set title bar color
        macChangeTitleBarColor(winId(), 66.f/255.f, 66.f/255.f, 66.f/255.f);
#endif

		// set icon
		menubar->setTopLeftCornerIcon(":/icon/Icon/icon.png");
	}

	BuildWindow::~BuildWindow()
	{
	}
}
