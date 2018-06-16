#pragma once

#include "engine/core/render/render/Renderer.h"
#include "engine/core/Scene/NodeTree.h"
#include "FrameState.h"
#include "ProjectSettings.h"

namespace Echo
{
	class IO;
	class Engine
	{	
	public:
		// 配置
		struct Config
		{
			String				m_projectFile;
			int					m_AudiomaxVoribsCodecs;
			bool				m_AudioLoadDecompresse;
			bool				m_isEnableProfiler;			// 是否开启性能分析服务
			unsigned int		m_windowHandle;
			bool				m_isGame;

			Config()
				: m_projectFile("")
				, m_AudiomaxVoribsCodecs(32)
				, m_AudioLoadDecompresse(false)
				, m_isEnableProfiler(false)
				, m_isGame(true)
			{}
		};

	public:
		// instance
		static Engine* instance();

		void tick(i32 elapsedTime);

		float getFrameTime() { return m_frameTime; }

		// 是否已初始化
		bool isInited() const { return m_isInited; }

		// 装载日志系统
		bool initLogSystem();
		bool initialize(const Config& cfg);
		bool initRenderer(Renderer* pRenderer, const Renderer::RenderCfg& config);
		bool onRendererInited();

		// screen size changed
		bool onSize(ui32 width, ui32 height);
		void destroy();

		// 获取配置
		const Config& getConfig() const { return m_cfg; }

		// 获取资源主路径
		const String& getResPath() const;

		// 获取用户资源路径
		const String& getUserPath() const;

		// 设置用户资源路径
		void setUserPath(const String& strPath);

		bool isRendererInited() const;
		const ui32&		getCurrentTime() const;

		FrameState&		frameState() { return m_frameState; }

		const FrameState& frameState() const { return m_frameState; }

		void resetFrameState() { m_frameState.reset(); }

		// 加载项目,引擎初始化时会自动调用，也可单独调用(全路径)
		void loadProject( const char* projectFile);
        
        void loadAllBankFile();

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
		ProjectSettings* getProjectFile() { return m_projectSettings.ptr(); }

	protected:
		bool render();
		void loadLaunchScene();

	private:
		bool				m_isInited;				// 是否已初始化
		Config				m_cfg;					// 客户端传入
		String				m_resPath;				// 资源路径
		String				m_userPath;				// 用户资源路径
		bool				m_bRendererInited;
		float				m_frameTime;
		ui32				m_currentTime;
		Renderer*			m_renderer;				// 渲染器
		FrameState			m_frameState;
		ProjectSettingsPtr	m_projectSettings;		// 项目信息
	};
}
