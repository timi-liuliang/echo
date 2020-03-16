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
		// Window Size
		struct WindowSize
		{
			Echo::i32		m_width;
			Echo::i32		m_height;
			Echo::String	m_displayText;

			WindowSize(Echo::i32 width, Echo::i32 height, const Echo::String& displayText)
				: m_width(width), m_height(height), m_displayText(displayText)
			{}
		};

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
		void onSwitchResolutionHorizontal();
		void onSwitchResolutionVertical();

	private:
		Window*							m_renderWindow;
		Echo::vector<WindowSize>::type	m_windowSizes;
	};
}