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

		// hide window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	}

	AboutWindow::~AboutWindow()
	{
	}
}