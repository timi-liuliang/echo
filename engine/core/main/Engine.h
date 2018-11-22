#pragma once

#include "FrameState.h"

namespace Echo
{
	class Engine
	{	
	public:
		// ≈‰÷√
		struct Config
		{
			String			m_projectFile;
			size_t			m_windowHandle;
			bool			m_isGame;

			Config()
				: m_projectFile("")
				, m_isGame(true)
			{}
		};

	public:
		// instance
		static Engine* instance();

		// initialize
		bool initialize(const Config& cfg);

        // tick second
		void tick(float elapsedTime);

		// get frame time
		float getFrameTime() { return m_frameTime; }

		// is inited
		bool isInited() const { return m_isInited; }

		// res path
		const String& getResPath() const;

		// urse path
		const String& getUserPath() const;
		void setUserPath(const String& strPath);

		// get current time
		const ui32&	getCurrentTime() const;

		// frame state
		FrameState&	getFrameState() { return m_frameState; }
		const FrameState& getFrameState() const { return m_frameState; }

		// on platform suspend
		void onPlatformSuspend();

		// on platform resume
		void onPlatformResume();

		// screen size changed
		bool onSize(ui32 windowWidth, ui32 windowHeight);
		void destroy();

		// config
		const Config& getConfig() const { return m_config; }

		// settings
		void loadSettings();
		void saveSettings();

	private:
		Engine();
		~Engine();

		// register all class types
		void registerClassTypes();

		// load project
		void loadProject(const char* projectFile);

		// load launch scene
		void loadLaunchScene();

		// initialize render
		bool initRenderer(size_t windowHandle);

	private:
		Config				m_config;
		String				m_resPath;
		String				m_userPath;
		bool				m_isInited;
		float				m_frameTime;
		ui32				m_currentTime;
		FrameState			m_frameState;
	};
}
