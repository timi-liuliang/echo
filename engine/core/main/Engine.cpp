#include "engine/core/base/class.h"
#include "engine/core/memory/MemManager.h"
#include "engine/core/main/Engine.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/Exception.h"
#include "engine/core/io/IO.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/render/render/ImageCodecMgr.h"
#include "engine/core/scene/render_node.h"
#include "engine/core/scene/NodeTree.h"
#include "engine/core/util/Timer.h"
#include "engine/core/render/render/Viewport.h"
#include "engine/core/render/Material.h"
#include "GameSettings.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/script/lua/register_core_to_lua.cxx"
#include "engine/core/script/lua/LuaBinder.h"
#include "engine/core/render/RenderTargetManager.h"
#include "module.h"
#include "engine/core/render/renderstage/RenderStage.h"
#include "engine/core/render/gles/GLES2.h"
#include "engine/core/script/LuaScript.h"
#include "engine/core/render/render/ShaderProgramRes.h"
#include "engine/core/render/TextureCube.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
	Engine::Engine()
		: m_isInited(false)
		, m_bRendererInited(NULL)
		, m_currentTime(0)
	{
		MemoryManager::instance();
		Time::instance();
		LogManager::instance();
	}

	Engine::~Engine()
	{
	}

	// instance
	Engine* Engine::instance()
	{
		static Engine* inst = new Engine();
		return inst;
	}

	// 引擎初始化
	bool Engine::initialize(const Config& cfg)
	{
		m_cfg = cfg;

		// check root path
		setlocale(LC_ALL, "zh_CN.UTF-8");

		try
		{
			if (!PathUtil::IsFileExist( cfg.m_projectFile))
			{
				EchoLogError("Set root path failed [%s], initialise Echo Engine failed.", cfg.m_projectFile.c_str());
				return false;
			}

			ImageCodecMgr::instance();
			IO::instance();
		}
		catch (const Exception &e)
		{
			EchoLogError(e.getMessage().c_str());
			return false;
		}

		// lua script
		{
			luaex::LuaEx* luaEx = luaex::LuaEx::instance();
			LuaBinder::instance()->init(luaEx->get_state());
			register_core_to_lua();
			registerClassTypes();
		}

		// 加载项目文件
		loadProject(cfg.m_projectFile.c_str());

		// init render
		initRenderer( cfg.m_windowHandle);

		Renderer::BGCOLOR = Echo::Color(0.298f, 0.298f, 0.322f);

		if (m_cfg.m_isGame)
		{
			loadLaunchScene();
		}

		m_isInited = true;

		return true;
	}

	void Engine::loadLaunchScene()
	{
		const ResourcePath& launchScene = GameSettings::instance()->getLaunchScene();
		if (!launchScene.isEmpty())
		{
			Echo::Node* node = Echo::Node::load(launchScene.getPath(), false);
			node->setParent(NodeTree::instance()->getInvisibleRootNode());
		}
		else
		{
			EchoLogError("Please set Game.LaunchScene before start the game");
		}
	}

	// register all class types
	void Engine::registerClassTypes()
	{
		Class::registerType<Node>();
		Class::registerType<Render>();
		Class::registerType<Res>();
		Class::registerType<ShaderProgramRes>();
		Class::registerType<Material>();
		Class::registerType<LuaScript>();
		Class::registerType<TextureCube>();
		Class::registerType<GameSettings>();
		Class::registerType<Gizmos>();

		// register all module class
		Module::registerAllTypes();
	}

	// 加载项目,引擎初始化时会自动调用，也可单独调用(全路径)
	void Engine::loadProject(const char* projectFile)
	{
		if (PathUtil::IsFileExist(projectFile))
		{
			m_resPath = PathUtil::GetFileDirPath(projectFile);
			IO::instance()->setResPath(m_resPath);
		
			loadSettings();	
		}
		else
		{
			EchoLogError("Not found project file [%s], initialise Echo Engine failed.", projectFile);
		}
	}

	// settings
	void Engine::loadSettings()
	{
		String projectFile;
		if (IO::instance()->covertFullPathToResPath(m_cfg.m_projectFile, projectFile))
		{
			MemoryReader reader(projectFile);
			if (reader.getSize())
			{
				pugi::xml_document doc;
				if (doc.load_buffer(reader.getData<char*>(), reader.getSize()))
				{
					pugi::xml_node root = doc.child("settings");
					if (root)
					{
						Echo::StringArray classes;
						Echo::Class::getAllClasses(classes);
						for (Echo::String& className : classes)
						{
							if (Echo::Class::isSingleton(className))
							{
								pugi::xml_node classNode = root.child(className.c_str());
								if(classNode)
								{
									Echo::Object::instanceObject(&classNode);
								}
							}
						}
					}
				}
			}
		}
	}

	void Engine::saveSettings()
	{
		pugi::xml_document doc;

		// declaration
		pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
		dec.append_attribute("version") = "1.0";
		dec.append_attribute("encoding") = "utf-8";

		// root node
		pugi::xml_node root = doc.append_child("settings");

		// save all singleton class settings
		Echo::StringArray classes;
		Echo::Class::getAllClasses(classes);
		for (Echo::String& className : classes)
		{
			if (Echo::Class::isSingleton(className))
			{
				pugi::xml_node classNode = root.append_child(className.c_str());
				if (classNode)
				{
					Object* obj = Echo::Class::create(className);
					if (obj)
					{
						classNode.append_attribute("name").set_value(className.c_str());
						classNode.append_attribute("class").set_value(className.c_str());
						Object::savePropertyRecursive(&classNode, obj, obj->getClassName());
					}
				}
			}
		}

		doc.save_file(m_cfg.m_projectFile.c_str(), "\t", 1U, pugi::encoding_utf8);
	}

	// 初始化渲染器
	bool Engine::initRenderer(unsigned int windowHandle)
	{
		Renderer* renderer = nullptr;
		LoadGLESRenderer(renderer);

		Echo::Renderer::RenderCfg renderCfg;
		renderCfg.enableThreadedRendering = false;
		renderCfg.windowHandle = windowHandle;
		renderCfg.enableThreadedRendering = false;

		EchoLogDebug("Canvas Size : %d x %d", renderCfg.screenWidth, renderCfg.screenHeight);
		if (renderer)
		{
			if (!renderer->initialize(renderCfg))
			{
				EchoLogError("Root::initRenderer failed...");
				return false;
			}

			if (!onRendererInited())
				return false;

			// 初始化渲染目标管理器
			if (!RenderTargetManager::instance()->initialize())
				return false;

			EchoLogInfo("Init Renderer success.");
			return true;
		}
		
		return false;
	}

	// 当游戏挂起时候引擎需要进行的处理
	void Engine::onPlatformSuspend()
	{
	}

	// 当游戏从挂起中恢复时引擎需要进行的处理
	void Engine::onPlatformResume()
	{
	}

	// 渲染初始化
	bool Engine::onRendererInited()
	{
		if (m_bRendererInited)
			return true;

		if (!NodeTree::instance()->init())
			return false;

		// setup viewport
		Viewport* pViewport = Renderer::instance()->getFrameBuffer()->getViewport();

		Camera* p2DCamera = NodeTree::instance()->get2dCamera();
		pViewport->setViewProjMatrix(p2DCamera->getViewProjMatrix());

		m_bRendererInited = true;

		return true;
	}

	bool Engine::onSize(ui32 width, ui32 height)
	{
		if (m_bRendererInited)
		{
			Renderer::instance()->onSize(width, height);

			Camera* pMainCamera = NodeTree::instance()->get3dCamera();
			pMainCamera->setWidth(Real(width));
			pMainCamera->setHeight(Real(height));
			pMainCamera->update();

			Camera* p2DCamera = NodeTree::instance()->get2dCamera();
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

	void Engine::destroy()
	{
		EchoSafeDeleteInstance(NodeTree);
		EchoSafeDeleteInstance(ImageCodecMgr);
		EchoSafeDeleteInstance(IO);
		EchoSafeDeleteInstance(Time);	
		EchoSafeDeleteInstance(RenderTargetManager);
		EchoLogInfo("Echo Engine has been shutdown.");
		Renderer::instance()->destroy();
		EchoSafeDeleteInstance(Renderer);
		EchoLogInfo("Echo Renderer has been shutdown.");
		EchoSafeDeleteInstance(LogManager);
		LuaBinder::destroy();
		luaex::LuaEx::instance()->destroy();
		MemoryManager::destroyInstance();
	}

	const String& Engine::getResPath() const
	{
		return m_resPath;
	}

	const String& Engine::getUserPath() const
	{
		return m_userPath;
	}

	void Engine::setUserPath(const String& strPath)
	{
		m_userPath = strPath;

		IO::instance()->setUserPath(m_userPath);
	}

	bool Engine::isRendererInited() const
	{
		return m_bRendererInited;
	}

	const ui32& Engine::getCurrentTime() const
	{
		return m_currentTime;
	}

	void Engine::tick(float elapsedTime)
	{
		// calculate time
		elapsedTime = Math::Clamp( elapsedTime, 0.f, 1.f);
		m_frameTime = elapsedTime;
		m_currentTime = static_cast<ui32>(Time::instance()->getMilliseconds());

		// update logic
		Module::updateAll(m_frameTime);
		NodeTree::instance()->update(m_frameTime);

		// render
		RenderStage::instance()->process();

		// present to screen
		Renderer::instance()->present();
	}
}
