#pragma once

#include "Window.h"
#include <QProcess>
#include "ui_GameMainWindow.h"
#include <engine/core/util/StringUtil.h>

namespace Game
{
	class GameMainWindow : public QMainWindow, public Ui_GameMainWindow
	{
	public:
		GameMainWindow(QWidget* parent=0);
		~GameMainWindow();

		// get instance
		static GameMainWindow* instance();

		// ¿ªÊ¼äÖÈ¾
		void start(const Echo::String& echoProject);

	private:
		Window*		m_renderWindow;
	};
}