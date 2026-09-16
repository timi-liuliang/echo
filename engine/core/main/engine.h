#pragma once

#include "frame_state.h"
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
			String			m_rootPath;
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

		// get frame count
		ui32 getFrameCount() const { return m_frameCount;}

		// is initialized
		bool isInited() const { return m_isInited; }

		// get root path
		void setRootPath(const String& rootPath) { m_rootPath = rootPath; }
		virtual const String& getRootPath() { return m_rootPath; }

		// Res|User path
		const String& getResPath();
		const String& getUserPath();

		// on platform event
		void onPlatformSuspend();
		void onPlatformResume();

		// screen size changed
		bool onSize(ui32 windowWidth, ui32 windowHeight);

		// settings
		void loadSettings();
		void saveSettings();

		// Register all class types into the reflection system without requiring a project.
		//
		// Pure and side-effect free: it neither reads the project file, nor sets up IO res
		// paths, nor creates a renderer. That makes reflection usable before (and without)
		// Engine::initialize(), which is what `echo.exe cmd` needs: a command line that can
		// run tools such as createproject without opening a window or touching the GPU.
		static void registerTypeInfos();

	private:
		Engine();
        
        // destroy
        void destroy();

		// register all class types
		void registerClassTypes();

		// Pure registration steps, shared by registerClassTypes() and registerTypeInfos().
		// Kept as separate functions so the exact interleaved call order inside
		// registerClassTypes() (core -> plugins -> modules) stays identical to before.
		static void registerCoreClassTypes();
		static void registerModuleTypes();

		// load launch scene
		void loadLaunchScene();

	private:
		Config				m_config;
		String				m_rootPath;
		bool				m_isInited;
		float				m_frameTime;
		ui32				m_frameCount = 0;
	};
}

#ifdef ECHO_EDITOR_MODE
#define IsGame Echo::Engine::instance()->getConfig().m_isGame
#else
#define IsGame true
#endif
