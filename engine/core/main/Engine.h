#pragma once

#include "engine/core/render/render/Renderer.h"
#include "engine/core/Scene/NodeTree.h"
#include "FrameState.h"
#include "ProjectSettings.h"

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

		// 是否已初始化
		bool isInited() const { return m_isInited; }

		// 装载日志系统
		bool initialize(const Config& cfg);

		// screen size changed
		bool onSize(ui32 width, ui32 height);
		void destroy();

		// 获取资源主路径
		const String& getResPath() const;

		// 获取用户资源路径
		const String& getUserPath() const;

		// 设置用户资源路径
		void setUserPath(const String& strPath);

		bool isRendererInited() const;
		const ui32&	getCurrentTime() const;

		// frame state
		FrameState&	frameState() { return m_frameState; }
		const FrameState& frameState() const { return m_frameState; }
		void resetFrameState() { m_frameState.reset(); }

		// 加载项目,引擎初始化时会自动调用，也可单独调用(全路径)
		void loadProject( const char* projectFile);

		// 当游戏挂起时候引擎需要进行的处理
		void onPlatformSuspend();

		// 当游戏从挂起中恢复时引擎需要进行的处理
		void onPlatformResume();

	private:
		Engine();
		~Engine();

		// register all class types
		void registerClassTypes();

	public:
		// config
		const Config& getConfig() const { return m_cfg; }

		// project settings
		ProjectSettings* getProjectSettings() { return m_projectSettings.ptr(); }

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
		ProjectSettingsPtr	m_projectSettings;		// 项目信息
	};
}
