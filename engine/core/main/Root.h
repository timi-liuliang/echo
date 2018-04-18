#pragma once

#include "engine/core/render/render/Renderer.h"
#include "engine/core/Resource/EchoThread.h"
#include "engine/core/main/EngineSettings.h"
#include "engine/core/main/EngineConsole.h"
#include "engine/core/Util/Singleton.h"
#include "engine/core/Scene/NodeTree.h"
#include "FrameState.h"

namespace Echo
{
	class ProjectFile;
	class ArchiveManager;
	class ArchiveFactory;
	class IO;
	class ImageCodecMgr;
	class RenderTargetManager;
	class ProfilerServer;
	class Root
	{	
	public:
		// 外部模块接口
		struct ExternalMgr
		{
			typedef std::function<void(int)> TickFun;
			typedef std::function<void()> ReleaseFun;
			typedef std::function<void()> RenderFun;

			String		m_name;			// 组件名称
			TickFun		m_tick;			// 更新函数 
			RenderFun	m_render;		// 渲染函数
			ReleaseFun	m_release;		// 释放函数
			bool		m_isReleased;	// 是否已经被释放
		};

		// 配置
		struct Config
		{
			typedef vector<ArchiveFactory*>::type ArchiveFactoryTypes;
			typedef vector<ExternalMgr>::type ExternalMgrs;

			String				projectFile;
			String				engineCfgFile;
			void*				pAssetMgr;					// for Android platfrom
			ArchiveFactoryTypes	externalArchiveFactorys;	// external archive factory
			int					m_AudiomaxVoribsCodecs;
			bool				m_AudioLoadDecompresse;
			bool				m_isEnableProfiler;			// 是否开启性能分析服务
			ExternalMgrs		m_externalMgrs;				// 外部模块

			unsigned int		m_windowHandle;
			bool				m_isGame;

			Config()
				: projectFile("")
				, engineCfgFile("engine.xml")
				, pAssetMgr(NULL)
				, m_AudiomaxVoribsCodecs(32)
				, m_AudioLoadDecompresse(false)
				, m_isEnableProfiler(false)
				, m_isGame(true)
			{}
		};

		__DeclareSingleton(Root);

	public:
		Root();
		~Root();

		void tick(i32 elapsedTime);

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
		void releasePlugins();

		// 获取配置
		const Config& getConfig() const { return m_cfg; }

		// 获取资源主路径
		const String& getResPath() const;

		// 获取用户资源路径
		const String& getUserPath() const;

		// 设置用户资源路径
		void setUserPath(const String& strPath);

		void* getAssetManager() const;
		bool isRendererInited() const;
		const ui32&		getCurrentTime() const;
		inline ui32		getFrameTime() const { return m_frameTime; }
		inline ui32		getFrameRealTime() const { return m_frameRealTime; }
		ui32			getFrameCount() const;
		ui32			getFPS() const;
		ui32			getMaxFrameTime() const;

		void			enableStreamThread(bool enable);
		StreamThread*	getThreadThread() const;

		inline void		setEnableFrameProfile( bool _enable ){ m_enableFrameProfile = _enable; }

		inline bool		getEnableFrameProfile() const { return m_enableFrameProfile; }

		inline FrameState&		frameState() { return m_frameState; }

		inline const FrameState& frameState() const { return m_frameState; }

		void outputFrameState() { m_frameState.output(); }
		void resetFrameState() { m_frameState.reset(); }
		void SetPhoneinformation(int max,int free,String cpu);

		// 获取当前帧数量
		ui32 getCurFrameCount() const { return m_curFameCount; }

		void setEnableFilterAdditional( bool _val) { m_enableFilterAdditional = _val; }

		bool getEnableFilterAdditional() const { return m_enableFilterAdditional; }

		// 设置帧缓冲缩放比
		void setFrameBufferScale( float scale) { m_framebufferScale =  scale; }
		
		// 获取帧缓存缩放比
		float getFramebufferScale() const { return m_framebufferScale; }
		
		void changeFilterAdditionalMap(const String& mapName);

		// 设置是否渲染当前场景
		void setRenderSceneEnable( bool isRenderScene) { m_isRenderScene = isRenderScene; }

		// 是否渲染场景
		bool isRenderScene() { return m_isRenderScene; }

		// 加载项目,引擎初始化时会自动调用，也可单独调用(全路径)
		void loadProject( const char* projectFile);
        
        void loadAllBankFile();

		// 当游戏挂起时候引擎需要进行的处理
		void onPlatformSuspend();

		// 当游戏从挂起中恢复时引擎需要进行的处理
		void onPlatformResume();

		// 设置资源延迟释放时间
		void setReleaseDelayTime(ui32 t);

	private:
		// register all class types
		void registerClassTypes();

	public:
		ProjectFile* getProjectFile() { return m_projectFile; }
		MemoryManager* getMemoryManager() { EchoAssert(m_memoryManager);  return m_memoryManager; }
		LogManager* getLogManager() { EchoAssert(m_logManager);  return m_logManager; }
		SkeletonManager* getSkeletonManager() { EchoAssert(m_skeletonManager);  return m_skeletonManager; }
		AnimManager* getAnimManager() { EchoAssert(m_animManager);  return m_animManager; }
		AnimSystemManager* getAnimSysManager() { EchoAssert(m_animSysManager);  return m_animSysManager; }
		ModelManager* getModelManager() { EchoAssert(m_modelManager);  return m_modelManager; }
		EffectSystemManager* getEffectSystemManager() { EchoAssert(m_EffectSystemManager);  return m_EffectSystemManager; }
		IO* getResourceGroupManager() { EchoAssert(m_io); return m_io; }
		TextureResManager* getTextureResManager() { EchoAssert(m_textureResManager);  return m_textureResManager; }
		OpenMPTaskMgr* getOpenMPTaskMgr() { EchoAssert(m_openMPTaskMgr);  return m_openMPTaskMgr; }
		EngineSettingsMgr& getSettingsMgr() { return m_settingsMgr; }
		EngineConsole& getConsole() { return m_console; }
		ImageCodecMgr* getImageCodecManager() { EchoAssert(m_imageCodecManager);  return m_imageCodecManager; }
		FSAudioManager* getAudioManager() { EchoAssert(m_audioManager);  return m_audioManager; }

	protected:
		void updateAllManagerDelayResource();
		bool render();
		void loadFirstScene();

	private:
		bool				m_isInited;				// 是否已初始化
		Config				m_cfg;					// 客户端传入
		EngineSettingsMgr	m_settingsMgr;			// 配置管理器
		EngineConsole		m_console;				// 命令行控制台
		String				m_resPath;				// 资源路径
		String				m_userPath;				// 用户资源路径
		void*				m_pAssetMgr;			// for android
		bool				m_bRendererInited;

		bool				m_bSupportGPUSkin;

		ui32				m_lastTime;
		ui32				m_frameTime;
		ui32				m_frameRealTime;
		ui32				m_currentTime;
		ui32				m_timeCount;
		ui32				m_frameCount;
		ui32				m_curFameCount;
		ui32				m_fps;
		ui32				m_maxFrameTime;
		bool				m_isRenderScene;				// 是否渲染场景

		MemoryManager*		m_memoryManager;				// 内存管理器
		LogManager*			m_logManager;
		IO*					m_io;
		TextureResManager*	m_textureResManager;			// 纹理资源管理器
		SkeletonManager*	m_skeletonManager;				// 骨骼资源管理器
		AnimManager*		m_animManager;					// 动画管理器
		AnimSystemManager*	m_animSysManager;				// 动画系统管理器
		ImageCodecMgr*		m_imageCodecManager;			// 纹理编码管理器
		ModelManager*		m_modelManager;					// 模型管理器
		Time*				m_Timer;
		EffectSystemManager*m_EffectSystemManager;			// 特效系统管理器
		Renderer*			m_renderer;						// 渲染器
		NodeTree*		m_sceneManager;					// 场景管理器
		FSAudioManager*		m_audioManager;					// 音频管理器
		StreamThread*		m_StreamThreading;				// 流加载线程
		bool				m_enableFrameProfile;
		FrameState			m_frameState;
		int					Maxmemory;
		int					Freememory;
		String				cputex;
		bool				m_enableBloom;
		bool				m_enableFilterAdditional;
		float				m_framebufferScale;             // 帧缓冲区缩放
		OpenMPTaskMgr*		m_openMPTaskMgr;				// OpenMP任务处理器
		RenderTargetManager*m_renderTargetManager;			// 渲染目标管理器
		ProjectFile*		m_projectFile;					// 项目信息

#ifdef ECHO_PROFILER
		ProfilerServer*		m_profilerSev;					// 性能分析服务器
#endif
	};
}

#define EchoRoot					Echo::Root::instance()
#define EchoMemoryManager			EchoRoot->getMemoryManager()
#define EchoLogManager				EchoRoot->getLogManager()
#define EchoSkeletonManager			EchoRoot->getSkeletonManager()
#define EchoAnimManager				EchoRoot->getAnimManager()
#define EchoAnimSystemManager		EchoRoot->getAnimSysManager()
#define EchoEffectSystemManager		EchoRoot->getEffectSystemManager()
#define EchoTextureResManager		EchoRoot->getTextureResManager()
#define EchoOpenMPTaskMgr			EchoRoot->getOpenMPTaskMgr()
#define EchoEngineSettings			EchoRoot->getSettingsMgr()
#define EchoEngineConsole			EchoRoot->getConsole()
#define EchoModelManager			EchoRoot->getModelManager()
#define EchoImageCodecManager		EchoRoot->getImageCodecManager()
#define EchoAudioManager			EchoRoot->getAudioManager()
