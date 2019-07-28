#pragma once

#include "FrameState.h"

namespace Echo
{
	class Engine
	{	
	public:
		// config
		struct Config
		{
			String			m_projectFile;
			bool			m_isGame;

			Config()
				: m_projectFile("")
				, m_isGame(true)
			{}
		};

	public:
        ~Engine();
        
		// instance
		static Engine* instance();

		// initialize with config
		bool initialize(const Config& cfg);
        const Config& getConfig() const { return m_config; }

		// initialize dll
		void initializeDll();

        // tick second
		void tick(float elapsedTime);

		// get frame time
		float getFrameTime() { return m_frameTime; }
		ui32 getFrameTimeMS() { return ui32(m_frameTime*1000.f); }

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

		// on platform event
		void onPlatformSuspend();
		void onPlatformResume();

		// screen size changed
		bool onSize(ui32 windowWidth, ui32 windowHeight);

		// settings
		void loadSettings();
		void saveSettings();

	private:
		Engine();
        
        // destroy
        void destroy();

		// register all class types
		void registerClassTypes();

		// load launch scene
		void loadLaunchScene();

	private:
		Config				m_config;
		String				m_resPath;
		String				m_userPath;
		bool				m_isInited;
		float				m_frameTime;
		ui32				m_currentTime;
		FrameState			m_frameState;
	};
    
    // init engine
    Engine* initEngine(const String& project, bool isGame);
}

#ifdef ECHO_EDITOR_MODE
#define IsGame Echo::Engine::instance()->getConfig().m_isGame
#else
#define IsGame true
#endif
