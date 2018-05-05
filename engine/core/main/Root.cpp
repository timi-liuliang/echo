#include "engine/core/base/class.h"
#include "engine/core/Memory/MemManager.h"
#include "Engine/core/main/Root.h"
#include "engine/core/Util/PathUtil.h"
#include "engine/core/Util/Exception.h"
#include "engine/core/io/IO.h"
#include "Engine/core/Resource/EchoThread.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/render/render/ImageCodecMgr.h"
#include "Engine/modules/Model/Model.h"
#include "Engine/modules/Anim/AnimManager.h"
#include "Engine/modules/Anim/SkeletonManager.h"
#include "Engine/core/Scene/NodeTree.h"
#include "engine/core/Util/Timer.h"
#include "engine/core/render/render/Viewport.h"
#include "Engine/core/Render/MaterialInst.h"
#include "ProjectSettings.h"
#include "Engine/modules/Audio/FMODStudio/FSAudioManager.h"
#include "Engine/modules/anim/AnimSystem.h"
#include "engine/core/render/render/RenderThread.h"
#include "engine/core/render/render/Video.h"
#include "EngineTimeController.h"
#include "engine/core/script/lua/LuaEx.h"
#include "engine/core/script/lua/register_core_to_lua.cxx"
#include "engine/core/script/lua/LuaBinder.h"
#include "engine/core/render/RenderTargetManager.h"
#include "module.h"
#include "engine/core/render/renderstage/RenderStage.h"
#include "engine/core/render/gles/GLES2.h"

#ifdef ECHO_PLATFORM_ANDROID
#include <sys/syscall.h>
static unsigned int _GetCurrThreadId()
{
	return (unsigned int)syscall(224); 
}
#else
static unsigned int _GetCurrThreadId()
{
	return 0;
}
#endif

namespace Echo
{
	// 构造函数
	Root::Root()
		: m_isInited(false)
		, m_pAssetMgr(NULL)
		, m_bRendererInited(NULL)
		, m_currentTime(0)
		, m_enableBloom(false)
		, m_enableFilterAdditional(false)
		, m_framebufferScale(1.0f)
		, m_renderer(NULL)
		, m_projectFile(NULL)
#ifdef ECHO_PROFILER
		, m_profilerSev( nullptr)
#endif
	{
#ifdef ECHO_PLATFORM_WINDOWS
		// 解决Windows VS2013 Thread join死锁BUG
		_Cnd_do_broadcast_at_thread_exit();
#endif
		MemoryManager::instance();
		Time::instance();
		OpenMPTaskMgr::instance();
	}

	Root::~Root()
	{
	}

	// instance
	Root* Root::instance()
	{
		static Root* inst = new Root();
		return inst;
	}

	// 装载日志系统
	bool Root::initLogSystem()
	{
		LogManager::instance();

		return true;
	}

	// 引擎初始化
	bool Root::initialize(const Config& cfg)
	{
		m_cfg = cfg;
		m_pAssetMgr = cfg.pAssetMgr;

		// check root path
		setlocale(LC_ALL, "zh_CN.UTF-8");

		try
		{
			if (!PathUtil::IsFileExist( cfg.projectFile))
			{
				EchoLogError("Set root path failed [%s], initialise Echo Engine failed.", cfg.projectFile.c_str());
				return false;
			}

			ImageCodecMgr::instance();
			ModelManager::instance();
			SkeletonManager::instance();
			IO::instance();
		}
		catch (const Exception &e)
		{
			EchoLogError(e.getMessage().c_str());
			return false;
		}

#if !defined(NO_THEORA_PLAYER)
		// 创建视频播放单例
		EchoNew( VideoPlay);
#endif
		// 加载项目文件
		loadProject(cfg.projectFile.c_str());

		// lua script
		{
			luaex::LuaEx* luaEx = luaex::LuaEx::instance();
			LuaBinder::instance()->init(luaEx->get_state());
			register_core_to_lua();
			registerClassTypes();
		}
		
		// 音频管理器
		FSAudioManager::instance()->init(cfg.m_AudiomaxVoribsCodecs,cfg.m_AudioLoadDecompresse);
		loadAllBankFile();

		setEnableFrameProfile(true);

		// init render
		Renderer* renderer = nullptr;
		LoadGLESRenderer(renderer);

		Echo::Renderer::RenderCfg renderCfg;
		renderCfg.enableThreadedRendering = false;
		renderCfg.windowHandle = cfg.m_windowHandle;
		renderCfg.enableThreadedRendering = false;
		EchoRoot->initRenderer(renderer, renderCfg);

		Renderer::BGCOLOR = Echo::Color(0.298f, 0.298f, 0.322f);

		if (m_cfg.m_isGame)
		{
			loadLaunchScene();
		}

		m_isInited = true;

		return true;
	}

	void Root::loadLaunchScene()
	{
		Echo::String launchScene = "Res://main.scene";
		//const ProjectSettings::Setting* setting = m_projectFile->getSetting("Application/Run/LaunchScene");
		//if (setting)
		{
			Echo::Node* node = Echo::Node::load(launchScene);
			node->setParent(NodeTree::instance()->getInvisibleRootNode());
		}
	}

	// register all class types
	void Root::registerClassTypes()
	{
		Class::registerType<Node>();

		// register all module class
		Module::registerAllTypes();
	}

	// 加载项目,引擎初始化时会自动调用，也可单独调用(全路径)
	void Root::loadProject(const char* projectFile)
	{
		if (PathUtil::IsFileExist(projectFile))
		{
			m_resPath = PathUtil::GetFileDirPath(projectFile);

			EchoSafeDelete(m_projectFile, ProjectSettings);
			m_projectFile = EchoNew(ProjectSettings);
			m_projectFile->load(projectFile);
			m_projectFile->setupResource();

			IO::instance()->setResPath(m_resPath);
		}
		else
		{
			EchoLogError("Not found project file [%s], initialise Echo Engine failed.", projectFile);
		}
	}
	
	void Root::loadAllBankFile()
	{
		FSAudioManager::instance()->loadAllBankFile();
	}

	// 初始化渲染器
	bool Root::initRenderer(Renderer* pRenderer, const Renderer::RenderCfg& config)
	{
		EchoLogDebug("Canvas Size : %d x %d", config.screenWidth, config.screenHeight);

		m_renderer = pRenderer;

		// 创建纹理资源工厂
		TextureResManager::instance();

		EchoAssert(pRenderer);
		if (!pRenderer->initialize(config))
		{
			EchoAssert(false);
			EchoLogError( "Root::initRenderer failed...");
			return false;
		}

		if (!onRendererInited())
			return false;

		EchoLogInfo("Init Renderer success.");

		m_settingsMgr.Apply(m_cfg.engineCfgFile);

		// 初始化渲染目标管理器
		if (!RenderTargetManager::instance()->initialize())
		{
			EchoLogError("RenderTargetManager::initialize Falied !");

			return false;
		}

		EchoLogInfo("Initialize RenderStageManager Success !");

		return true;
	}

	// 当游戏挂起时候引擎需要进行的处理
	void Root::onPlatformSuspend()
	{
		FSAudioManager::instance()->suspendFmodSystem();
		g_render_thread->flushRenderTasks();
	}

	// 当游戏从挂起中恢复时引擎需要进行的处理
	void Root::onPlatformResume()
	{
		FSAudioManager::instance()->resumeFmodSystem();
	}

	// 渲染初始化
	bool Root::onRendererInited()
	{
		if (m_bRendererInited)
			return true;

		if (!NodeTree::instance()->init())
			return false;

		// setup viewport
		Viewport* pViewport = Renderer::instance()->getFrameBuffer()->getViewport();

		Camera* p2DCamera = NodeTree::instance()->get2DCamera();
		pViewport->setViewProjMatrix(p2DCamera->getViewProjMatrix());
		ui32 width = pViewport->getWidth();
		ui32 height = pViewport->getHeight();

		m_bRendererInited = true;

		return true;
	}

	bool Root::onSize(ui32 width, ui32 height)
	{
		if (m_bRendererInited)
		{
			Renderer::instance()->onSize(width, height);

			Camera* pMainCamera = NodeTree::instance()->get3dCamera();
			pMainCamera->setWidth(Real(width));
			pMainCamera->setHeight(Real(height));
			pMainCamera->update();

			Camera* p2DCamera = NodeTree::instance()->get2DCamera();
			p2DCamera->setWidth(Real(width));
			p2DCamera->setHeight(Real(height));
			p2DCamera->update();

			Renderer::instance()->getFrameBuffer()->getViewport()->setViewProjMatrix(p2DCamera->getViewProjMatrix());
		}

		// 渲染目标重置大小
		if (RenderTargetManager::instance())
		{
			RenderTargetManager::instance()->onScreensizeChanged(width, height);
		}

		return true;
	}

	// 游戏销毁
	void Root::destroy()
	{
		EchoSafeDeleteInstance(StreamThread);

#if !defined(NO_THEORA_PLAYER)
		VideoPlay* videoPlay = VideoPlay::Instance();
		EchoSafeDelete(videoPlay, VideoPlay);
#endif

		// 场景管理器
		EchoSafeDeleteInstance(NodeTree);

		// 音频管理器
		FSAudioManager::instance()->release();

		EchoSafeDeleteInstance(FSAudioManager);
		EchoSafeDeleteInstance(ModelManager);
		EchoSafeDeleteInstance(AnimSystemManager); //animSysManager要在animManager之前释放，因为会用到animManager来释放自己的animBlend
		EchoSafeDeleteInstance(AnimManager);
		EchoSafeDeleteInstance(SkeletonManager);
		
		EchoSafeDeleteInstance(ImageCodecMgr);
		EchoSafeDeleteInstance(OpenMPTaskMgr);

		EchoSafeDeleteInstance(IO);
		EchoSafeDeleteInstance(Time);

		// 外部模块释放
		//for (const ExternalMgr& mgr : m_cfg.m_externalMgrs)
		//	mgr.m_release();
		releasePlugins();
		
		EchoLogInfo("Echo Engine has been shutdown.");
		
		EchoSafeDeleteInstance(RenderTargetManager);
		EchoSafeDeleteInstance(TextureResManager);
		
		// 渲染器
		if (m_renderer)
		{
			m_renderer->destroy();
			EchoSafeDelete(m_renderer, Renderer);
			EchoLogInfo("Echo Renderer has been shutdown.");
		}

		EchoSafeDeleteInstance(LogManager);
		EchoSafeDelete(m_projectFile, ProjectSettings);
#ifdef ECHO_PROFILER
#endif
		// 销毁时间控制器
		EngineTimeController::destroy();
		LuaBinder::destroy();
		luaex::LuaEx::instance()->destroy();
		MemoryManager::destroyInstance();
	}

	const String& Root::getResPath() const
	{
		return m_resPath;
	}

	const String& Root::getUserPath() const
	{
		return m_userPath;
	}

	void Root::setUserPath(const String& strPath)
	{
		m_userPath = strPath;

		IO::instance()->setUserPath(m_userPath);
	}

	void Root::SetPhoneinformation(int max, int free, String tex)
	{
		Maxmemory = max;
		Freememory = free;
		cputex = tex;
	}

	void* Root::getAssetManager() const
	{
		return m_pAssetMgr;
	}

	bool Root::isRendererInited() const
	{
		return m_bRendererInited;
	}

	const ui32& Root::getCurrentTime() const
	{
		return m_currentTime;
	}

	// 每帧更新
	void Root::tick(i32 elapsedTime)
	{
		elapsedTime = Math::Clamp( elapsedTime, 0, 1000);
		m_frameTime = elapsedTime * 0.001f;

#ifdef ECHO_PROFILER
#endif
		m_currentTime = Time::instance()->getMilliseconds();


#if !defined(NO_THEORA_PLAYER)
		// 视频更新
		VideoPlay::Instance()->tick(elapsedTime);
#endif
		// 资源加载线程
		if (StreamThread::instance()->IsRunning())
		{
#ifdef ECHO_PLATFORM_HTML5
			// Html5 doesn't support multithread well
			m_StreamThreading->processLoop();
#endif
			StreamThread::instance()->notifyTaskFinished();
		}

		// 声音更新
		auto t0 = EngineTimeController::instance()->getSpeed(EngineTimeController::ET_SOUND);
		auto t1 = EngineTimeController::instance()->getSpeed();
		FSAudioManager::instance()->tick(static_cast<ui32>(elapsedTime * t0 / t1));

		//
		auto t = EngineSettingsMgr::instance()->isSlowDownExclusiveUI() ? m_frameTime : elapsedTime;

		NodeTree::instance()->update(elapsedTime*0.001f);

		updateAllManagerDelayResource();

		// 执行动画更新
		OpenMPTaskMgr::instance()->execTasks(OpenMPTaskMgr::TT_AnimationUpdate);
		OpenMPTaskMgr::instance()->waitForAnimationUpdateComplete();

		// 外部模块更新, 目前只有 CatUI
		for (const ExternalMgr& mgr : m_cfg.m_externalMgrs)
		{
			mgr.m_tick(elapsedTime);
		}

		// 渲染
		render();

		// present to screen
		Renderer::instance()->present();
	}

	void Root::changeFilterAdditionalMap(const String& mapName)
	{
		if (m_enableFilterAdditional)
			RenderTargetManager::instance()->changeFilterBlendmapName(mapName);
	}

	void Root::updateAllManagerDelayResource()
	{
		static const ui32 MOD = 6;
		static ui32 nCount = 0;
		++nCount;
		int ct = nCount % MOD;
		switch (ct)
		{
			case 0:	TextureResManager::instance()->updateDelayedRelease(m_frameTime * MOD); break;
			//case 1:	EchoMeshManager->updateDelayedRelease(m_frameTime * MOD); break;
			case 2: SkeletonManager::instance()->updateDelayedRelease(m_frameTime * MOD); break;
			case 3: AnimManager::instance()->updateDelayedRelease(m_frameTime * MOD); break;
			case 4:
			{
			}
			break;
			case 5: AnimSystemManager::instance()->updateDelayedRelease(m_frameTime * MOD); break;
			default:
				break;
		}
	}

	// 渲染场景
	bool Root::render()
	{
		g_render_thread->syncFrame();

		// 外部模块更新, 目前只有 CatUI
		for (const ExternalMgr& mgr : m_cfg.m_externalMgrs)
		{
			mgr.m_render();
		}

		RenderStage::instance()->process();

		return true;
	}

	void Root::setReleaseDelayTime(ui32 t)
	{
		TextureResManager::instance()->setReleaseDelayTime(t);
		//EchoMeshManager->setReleaseDelayTime(t);
		SkeletonManager::instance()->setReleaseDelayTime(t);
		AnimManager::instance()->setReleaseDelayTime(t);
	}

	void Root::releasePlugins()
	{
		// 外部模块释放
		for (ExternalMgr& mgr : m_cfg.m_externalMgrs)
		{
			if (!mgr.m_isReleased)
			{
				mgr.m_release();
				mgr.m_isReleased = true;
			}
		}
	}
}
