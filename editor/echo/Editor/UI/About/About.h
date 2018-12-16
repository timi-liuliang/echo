#pragma once

#include <QProcess>
#include "ui_About.h"
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class AboutWindow : public QMainWindow, public Ui_AboutWindow
	{
	public:
		AboutWindow(QWidget* parent=0);
		~AboutWindow();

	private:
	};
}