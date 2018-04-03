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
#include "Engine/modules/Effect/EffectSystemManager.h"
#include "engine/core/Util/Timer.h"
#include "engine/core/render/render/Viewport.h"
#include "Engine/core/Render/MaterialInst.h"
#include "ProjectFile.h"
#include "Engine/modules/Audio/FMODStudio/FSAudioManager.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "engine/core/render/render/RenderThread.h"
#include "engine/core/render/render/Video.h"
#include "EngineTimeController.h"
#include "engine/core/script/lua/LuaBinder.h"
#include "engine/core/render/RenderTargetManager.h"
#include "module.h"
#include "engine/core/render/renderstage/RenderStage.h"

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

using namespace rapidxml;

namespace Echo
{
	// 输出引擎状态
	void FrameState::output()
	{
		// to do it 磊哥
	}

	__ImplementSingleton(Root);

	// 构造函数
	Root::Root()
		: m_isInited(false)
		, m_pAssetMgr(NULL)
		, m_bRendererInited(NULL)
		, m_bSupportGPUSkin(true)
		, m_lastTime(0)
		, m_frameTime(0)
		, m_frameRealTime(0)
		, m_currentTime(0)
		, m_timeCount(0)
		, m_frameCount(0)
		, m_curFameCount(0)
		, m_fps(0)
		, m_maxFrameTime(0)
		, m_io(NULL)
		, m_Timer(NULL)
		, m_EffectSystemManager(NULL)
		, m_StreamThreading(NULL)
		, m_enableBloom(false)
		, m_enableFilterAdditional(false)
		, m_framebufferScale(1.0f)
		, m_isRenderScene(true)
		, m_logManager(NULL)
		, m_skeletonManager(NULL)
		, m_animManager(NULL)
		, m_animSysManager(NULL)
		, m_renderer(NULL)
		, m_sceneManager(NULL)
		, m_modelManager(NULL)
		, m_imageCodecManager(NULL)
		, m_audioManager(NULL)
		, m_textureResManager(NULL)
		, m_renderTargetManager(NULL)
		, m_projectFile(NULL)
#ifdef ECHO_PROFILER
		, m_profilerSev( nullptr)
#endif
	{
#ifdef ECHO_PLATFORM_WINDOWS
		// 解决Windows VS2013 Thread join死锁BUG
		_Cnd_do_broadcast_at_thread_exit();
#endif

		__ConstructSingleton;

		m_memoryManager = MemoryManager::instance();
		m_Timer = Time::instance();
		m_openMPTaskMgr = EchoNew(OpenMPTaskMgr);
	}

	Root::~Root()
	{
		__DestructSingleton;
	}

	// 装载日志系统
	bool Root::initLogSystem()
	{
		m_logManager = LogManager::instance();

		return true;
	}

	// 引擎初始化
	bool Root::initialize(const RootCfg& cfg)
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

			m_imageCodecManager = EchoNew(ImageCodecMgr);
			m_modelManager		= EchoNew( ModelManager);
			m_animManager		= EchoNew( AnimManager);
			m_animSysManager	= EchoNew( AnimSystemManager);
			m_skeletonManager	= EchoNew( SkeletonManager);
			m_sceneManager		= EchoNew( NodeTree);
			m_io = EchoNew(IO);
			m_EffectSystemManager = EchoNew(EffectSystemManager);
			m_StreamThreading = EchoNew(StreamThread);
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

		LuaBinder::instance();

		// register all basic class types
		registerClassTypes();

		{
			//test
			LuaBinder::instance()->loadFile("Res://lua/a.lua");
			LuaBinder::instance()->call("calla");
		}


		// 音频管理器
		m_audioManager = EchoNew(FSAudioManager);
		m_audioManager->init(cfg.m_AudiomaxVoribsCodecs,cfg.m_AudioLoadDecompresse);
		loadAllBankFile();

		setEnableFrameProfile(true);

		m_isInited = true;

		return true;
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

			EchoSafeDelete(m_projectFile, ProjectFile);
			m_projectFile = EchoNew(ProjectFile);
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
		if (m_audioManager)
		{
			m_audioManager->loadAllBankFile();
		}
	}

	// 初始化渲染器
	bool Root::initRenderer(Renderer* pRenderer, const Renderer::RenderCfg& config)
	{
		EchoLogDebug("Canvas Size : %d x %d", config.screenWidth, config.screenHeight);

		m_renderer = pRenderer;

		// 创建纹理资源工厂
		m_textureResManager = EchoNew(TextureResManager);

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
		m_renderTargetManager = EchoNew(RenderTargetManager);
		if (!m_renderTargetManager->initialize())
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
		m_audioManager->suspendFmodSystem();
		g_render_thread->flushRenderTasks();
	}

	// 当游戏从挂起中恢复时引擎需要进行的处理
	void Root::onPlatformResume()
	{
		m_audioManager->resumeFmodSystem();
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

		// 特效系统创建RenderableSet
		m_EffectSystemManager->createRenderableSet();

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
		if (m_StreamThreading)
		{
			m_StreamThreading->shutdown();
			EchoSafeDelete(m_StreamThreading, StreamThread);
		}
#if !defined(NO_THEORA_PLAYER)
		VideoPlay* videoPlay = VideoPlay::Instance();
		EchoSafeDelete(videoPlay, VideoPlay);
#endif
		if (m_EffectSystemManager)
		{
			m_EffectSystemManager->destroyRenderableSet();
			m_EffectSystemManager->destroyAllEffectSystemTemplates();
			m_EffectSystemManager->destroyAllEffectSystems();
			EchoSafeDelete(m_EffectSystemManager, EffectSystemManager);
		}

		// 场景管理器
		if (m_sceneManager)
			m_sceneManager->destroy();

		// 音频管理器
		if (m_audioManager)
			m_audioManager->release();

		EchoSafeDelete(m_audioManager, FSAudioManager);
		EchoSafeDelete(m_sceneManager, NodeTree);
		EchoSafeDelete(m_modelManager, ModelManager);
		EchoSafeDelete(m_animSysManager, AnimSystemManager); //animSysManager要在animManager之前释放，因为会用到animManager来释放自己的animBlend
		EchoSafeDelete(m_animManager, AnimManager);
		EchoSafeDelete(m_skeletonManager, SkeletonManager);
		
		EchoSafeDelete(m_imageCodecManager, ImageCodecMgr);
		EchoSafeDelete(m_openMPTaskMgr, OpenMPTaskMgr);

		EchoSafeDelete(m_io, IO); //ResourceGroupManager的析构需要用到ArchiveManager
		Time::destroyInstance();

		// 外部模块释放
		//for (const ExternalMgr& mgr : m_cfg.m_externalMgrs)
		//	mgr.m_release();
		releasePlugins();
		
		EchoLogInfo("Echo Engine has been shutdown.");
		
		EchoSafeDelete(m_renderTargetManager, RenderTargetManager);
		EchoSafeDelete(m_textureResManager, TextureResManager);
		
		// 渲染器
		if (m_renderer)
		{
			m_renderer->destroy();
			EchoSafeDelete(m_renderer, Renderer);
			EchoLogInfo("Echo Renderer has been shutdown.");
		}

		LogManager* logMgr = LogManager::instance();
		EchoSafeDelete(logMgr, LogManager);
		EchoSafeDelete(m_projectFile, ProjectFile);
#ifdef ECHO_PROFILER
#endif
		// 销毁时间控制器
		EngineTimeController::destroy();
		LuaBinder::destroy();
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

	ui32 Root::getFrameCount() const
	{
		return m_frameCount;
	}

	ui32 Root::getFPS() const
	{
		return m_fps;
	}

	ui32 Root::getMaxFrameTime() const
	{
		return m_maxFrameTime;
	}

	void Root::enableStreamThread(bool enable)
	{
		if (m_StreamThreading)
		{
			if (enable)
				m_StreamThreading->start();
			else
				m_StreamThreading->shutdown();
		}
	}

	StreamThread* Root::getThreadThread() const
	{
		return m_StreamThreading;
	}

	// 每帧更新
	void Root::tick(i32 elapsedTime)
	{
		elapsedTime = Math::Clamp( elapsedTime, 0, 1000);

#ifdef ECHO_PROFILER
#endif
		++m_curFameCount;

		++m_frameCount;
		m_currentTime = Time::instance()->getMilliseconds();

		m_frameTime = elapsedTime;//m_currentTime - m_lastTime;
		m_frameRealTime = m_currentTime - m_lastTime;
		if (m_frameRealTime > 1000)
		{
			m_frameRealTime = 0;
		}
		m_lastTime = m_currentTime;
		m_timeCount += m_frameRealTime;

		if (m_maxFrameTime < m_frameRealTime)
		{
			m_maxFrameTime = m_frameRealTime;
		}

		if (m_timeCount >= 1000)
		{
			m_fps = m_frameCount;
			m_frameCount = 0;
			m_timeCount -= 1000;
			m_maxFrameTime = 0;
		}

#if !defined(NO_THEORA_PLAYER)
		// 视频更新
		VideoPlay::Instance()->tick(elapsedTime);
#endif
		// 资源加载线程
		if (m_StreamThreading && m_StreamThreading->IsRunning())
		{
#ifdef ECHO_PLATFORM_HTML5
			// Html5 doesn't support multithread well
			m_StreamThreading->processLoop();
#endif
			m_StreamThreading->notifyTaskFinished();
		}

		// 声音更新
		auto t0 = EngineTimeController::instance()->getSpeed(EngineTimeController::ET_SOUND);
		auto t1 = EngineTimeController::instance()->getSpeed();
		m_audioManager->tick(static_cast<ui32>(elapsedTime * t0 / t1));

		//
		auto t = EngineSettingsMgr::instance()->isSlowDownExclusiveUI() ? m_frameRealTime : elapsedTime;

		NodeTree::instance()->update(elapsedTime*0.001f);

		updateAllManagerDelayResource();

		// 执行动画更新
		EchoOpenMPTaskMgr->execTasks(OpenMPTaskMgr::TT_AnimationUpdate);
		EchoOpenMPTaskMgr->waitForAnimationUpdateComplete();

		// 特效系统后续更新
		EffectSystemManager::instance()->tick(elapsedTime);

		// 外部模块更新, 目前只有 CatUI
		for (const ExternalMgr& mgr : m_cfg.m_externalMgrs)
		{
			mgr.m_tick(elapsedTime);
		}

		m_rootNode.update(true);

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
			case 0:	EchoTextureResManager->updateDelayedRelease(m_frameTime * MOD); break;
			//case 1:	EchoMeshManager->updateDelayedRelease(m_frameTime * MOD); break;
			case 2: EchoSkeletonManager->updateDelayedRelease(m_frameTime * MOD); break;
			case 3: EchoAnimManager->updateDelayedRelease(m_frameTime * MOD); break;
			case 4:
			{
	/*			if (EchoRoot->getActorManagerUnsafe())
					EchoEngineActorManager->DestroyDelayActors(false);*/
			}
			break;
			case 5: EchoAnimSystemManager->updateDelayedRelease(m_frameTime * MOD); break;
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
		EchoTextureResManager->setReleaseDelayTime(t);
		//EchoMeshManager->setReleaseDelayTime(t);
		EchoSkeletonManager->setReleaseDelayTime(t);
		EchoAnimManager->setReleaseDelayTime(t);
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
