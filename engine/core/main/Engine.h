#pragma once

#include "FrameState.h"

namespace Echo
{
	class Engine
	{	
	public:
		// 配置
		struct Config
		{
			String			m_projectFile;
			bool			m_isEnableProfiler;			// 是否开启性能分析服务
			unsigned int	m_windowHandle;
			bool			m_isGame;

			Config()
				: m_projectFile("")
				, m_isEnableProfiler(false)
				, m_isGame(true)
			{}
		};

	public:
		// instance
		static Engine* instance();

		void tick(i32 elapsedTime);

		// get frame time
		float getFrameTime() { return m_frameTime; }

		// 蔵s inited
		bool isInited() const { return m_isInited; }

		// initialize
		bool initialize(const Config& cfg);

		// screen size changed
		bool onSize(ui32 width, ui32 height);
		void destroy();

		// get res path
		const String& getResPath() const;

		// get urse path
		const String& getUserPath() const;

		// set user path
		void setUserPath(const String& strPath);

		bool isRendererInited() const;
		const ui32&	getCurrentTime() const;

		// frame state
		FrameState&	frameState() { return m_frameState; }
		const FrameState& frameState() const { return m_frameState; }
		void resetFrameState() { m_frameState.reset(); }

		// load project
		void loadProject( const char* projectFile);

		// on platform suspend
		void onPlatformSuspend();

		// on platform resume
		void onPlatformResume();

	private:
		Engine();
		~Engine();

		// register all class types
		void registerClassTypes();

	public:
		// config
		const Config& getConfig() const { return m_cfg; }

		// settings
		void loadSettings();
		void saveSettings();

	protected:
		void loadLaunchScene();

		// initialize
		bool initRenderer(unsigned int windowHandle);
		bool onRendererInited();

	private:
		bool				m_isInited;				// 是否已初始化
		Config				m_cfg;					// 客户端传入
		String				m_resPath;				// 资源路径
		String				m_userPath;				// 用户资源路径
		bool				m_bRendererInited;
		float				m_frameTime;
		ui32				m_currentTime;
		FrameState			m_frameState;
	};
}
