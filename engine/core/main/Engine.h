#pragma once

#include "FrameState.h"
#include "engine/core/base/object.h"

namespace Echo
{
	class Engine : public Object
	{	
		ECHO_SINGLETON_CLASS(Engine, Object);

	public:
		// config
		struct Config
		{
			String			m_projectFile;
            String          m_userPath;
			bool			m_isGame;

			Config()
				: m_projectFile("")
				, m_isGame(true)
			{}
		};

	public:
        virtual ~Engine();
        
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
		float* getFrameTimePtr() { return &m_frameTime; }
		ui32 getFrameTimeMS() { return ui32(m_frameTime*1000.f); }

		// is inited
		bool isInited() const { return m_isInited; }

		// get root path
		void setRootPath(const String& rootPath) { m_rootPath = rootPath; }
		virtual const String& getRootPath() { return m_rootPath; }

		// res path
		const String& getResPath() const;

		// user path
		const String& getUserPath() const;
		void setUserPath(const String& strPath);

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
		String				m_rootPath;
		String				m_resPath;
		String				m_userPath;
		bool				m_isInited;
		float				m_frameTime;
	};
}

#ifdef ECHO_EDITOR_MODE
#define IsGame Echo::Engine::instance()->getConfig().m_isGame
#else
#define IsGame true
#endif
