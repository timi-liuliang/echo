#pragma once

#include "Window.h"
#include <QProcess>
#include "ui_GameMainWindow.h"
#include <engine/core/util/StringUtil.h>

namespace Game
{
	class GameMainWindow : public QMainWindow, public Ui_GameMainWindow
	{
		Q_OBJECT

	public:
		GameMainWindow(QWidget* parent=0);
		~GameMainWindow();

		// get instance
		static GameMainWindow* instance();

		// begin render
		void start(const Echo::String& echoProject);

		// move to center
		void moveToCenter();

		// set render window resolution
		void setRenderWindowSize(Echo::i32 width, Echo::i32 height);

	private:
		// config window sizes
		void configWindowSizes();

	private slots:
		// on switch resolution
		void onSwitchResolution();

	private:
		Window*		m_renderWindow;
	};
}