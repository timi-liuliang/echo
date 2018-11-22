#include "engine/core/base/class.h"
#include "engine/core/main/Engine.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/Exception.h"
#include "engine/core/io/IO.h"
#include "engine/core/log/Log.h"
#include "engine/core/render/interface/image/ImageCodecMgr.h"
#include "engine/core/scene/render_node.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/util/Timer.h"
#include "engine/core/render/interface/Viewport.h"
#include "engine/core/render/interface/Material.h"
#include "GameSettings.h"
#include "engine/core/script/lua/register_core_to_lua.cxx"
#include "engine/core/script/lua/lua_binder.h"
#include "engine/core/script/lua/lua_script.h"
#include "engine/core/render/interface/RenderTargetManager.h"
#include "module.h"
#include "engine/core/render/interface/renderstage/RenderStage.h"
#include "engine/core/render/gles/GLES.h"
#include "engine/core/render/interface/ShaderProgramRes.h"
#include "engine/core/render/interface/TextureCube.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/input/input.h"

namespace Echo
{
	Engine::Engine()
		: m_isInited(false)
		, m_currentTime(0)
	{
		Time::instance();
		Log::instance();
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

	// ÒýÇæ³õÊ¼»¯
	bool Engine::initialize(const Config& cfg)
	{
		m_config = cfg;

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
			LuaBinder::instance()->init();
			registerCoreToLua();
			registerClassTypes();
		}

		// init render
		if (initRenderer(cfg.m_windowHandle))
		{
			if (!NodeTree::instance()->init())
				return false;

			if (!RenderTargetManager::instance()->initialize())
				return false;

			// load project
			loadProject(cfg.m_projectFile.c_str());
			if (m_config.m_isGame)
			{
				loadLaunchScene();
			}
		}

		m_isInited = true;

		return true;
	}

	void Engine::loadLaunchScene()
	{
		const ResourcePath& launchScene = GameSettings::instance()->getLaunchScene();
		if (!launchScene.isEmpty())
		{
			Echo::Node* node = Echo::Node::loadLink(launchScene.getPath(), false);
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
		Class::registerType<Texture>();
		Class::registerType<ShaderProgramRes>();
		Class::registerType<Material>();
		Class::registerType<LuaScript>();
		Class::registerType<TextureCube>();
		Class::registerType<GameSettings>();
		Class::registerType<Gizmos>();
		Class::registerType<Input>();
		Class::registerType<Log>();

		// register all module class
		Module::registerAllTypes();
	}

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
		if (IO::instance()->covertFullPathToResPath(m_config.m_projectFile, projectFile))
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

		doc.save_file(m_config.m_projectFile.c_str(), "\t", 1U, pugi::encoding_utf8);
	}

	// init renderer
	bool Engine::initRenderer(size_t windowHandle)
	{
		Renderer* renderer = nullptr;
		LoadGLESRenderer(renderer);

		Echo::Renderer::Config renderCfg;
		renderCfg.enableThreadedRendering = false;
		renderCfg.windowHandle = windowHandle;
		renderCfg.enableThreadedRendering = false;

		EchoLogDebug("Canvas Size : %d x %d", renderCfg.screenWidth, renderCfg.screenHeight);
		if (renderer && renderer->initialize(renderCfg))
		{
			EchoLogInfo("Init Renderer success.");
			return true;
		}
		
		EchoLogError("Root::initRenderer failed...");
		return false;
	}

	void Engine::onPlatformSuspend()
	{
	}

	void Engine::onPlatformResume()
	{
	}

	bool Engine::onSize(ui32 windowWidth, ui32 windowHeight)
	{
		if ( m_isInited)
		{
			Renderer::instance()->onSize(windowWidth, windowHeight);
			GameSettings::instance()->onSize(windowWidth, windowHeight);
		}

		// render target
		if (RenderTargetManager::instance())
		{
			RenderTargetManager::instance()->onScreensizeChanged(windowWidth, windowHeight);
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
		EchoSafeDeleteInstance(Renderer);
		EchoLogInfo("Echo Renderer has been shutdown.");
		EchoSafeDeleteInstance(Log);
		EchoSafeDeleteInstance(LuaBinder);
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

		// input update
		Input::instance()->update();

		// render
		RenderStage::instance()->process();

		// present to screen
		Renderer::instance()->present();
	}
}
