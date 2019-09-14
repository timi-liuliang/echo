#include "engine/core/base/class.h"
#include "engine/core/main/Engine.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/HashGenerator.h"
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
#include "PluginSettings.h"
#include "engine/core/script/lua/register_core_to_lua.cx"
#include "engine/core/script/lua/lua_binder.h"
#include "engine/core/script/lua/lua_script.h"
#include "Plugin.h"
#include "module.h"
#include "engine/core/render/interface/pipeline/RenderPipeline.h"
#include "engine/core/render/interface/pipeline/RenderStage.h"
#include "engine/core/render/gles/GLES.h"
#include "engine/core/render/vulkan/vk.h"
#include "engine/core/render/metal/mt.h"
#include "engine/core/render/interface/ShaderProgram.h"
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
        destroy();
	}

	Engine* Engine::instance()
	{
		static Engine* inst = EchoNew(Engine);
		return inst;
	}

	void Engine::initializeDll()
	{

	}

	bool Engine::initialize(const Config& cfg)
	{
		m_config = cfg;

        ImageCodecMgr::instance();
        IO::instance();
        
		// check root path
		setlocale(LC_ALL, "zh_CN.UTF-8");
        if (PathUtil::IsFileExist( m_config.m_projectFile))
        {
            m_resPath = PathUtil::GetFileDirPath(m_config.m_projectFile);
            IO::instance()->setResPath(m_resPath);
        }
        else
        {
            EchoLogError("Set root path failed [%s], initialise Echo Engine failed.", cfg.m_projectFile.c_str());
            return false;
        }

        IO::instance()->setUserPath( m_config.m_userPath);

		// lua script
		{
			LuaBinder::instance()->init();
			registerCoreToLua();
			registerClassTypes();
		}

		// init render
        if (Renderer::instance())
		{
			if (!NodeTree::instance()->init())
				return false;

			// load settings second time
			loadSettings();
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

	void Engine::registerClassTypes()
	{
		// load settings first
		Class::registerType<PluginSettings>();
		loadSettings();

		// regiser class types in core
		Class::registerType<Object>();
		Class::registerType<Node>();
		Class::registerType<Render>();
		Class::registerType<Res>();
		Class::registerType<Texture>();
		Class::registerType<ShaderProgram>();
		Class::registerType<Material>();
		Class::registerType<LuaScript>();
		Class::registerType<TextureCube>();
		Class::registerType<GameSettings>();
		Class::registerType<Gizmos>();
		Class::registerType<Input>();
        Class::registerType<IO>();
        Class::registerType<DataStream>();
		Class::registerType<Log>();
		Class::registerType<RenderPipeline>();
		Class::registerType<RenderStage>();
		Class::registerType<RenderQueue>();
		Class::registerType<FrameBuffer>();

		// load all plugins
		Plugin::loadAllPlugins();

		// register all module
		Module::registerAllTypes();
	}

	void Engine::loadSettings()
	{
		if (PathUtil::IsFileExist(m_config.m_projectFile))
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
									if (classNode)
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
		else
		{
			EchoLogError("Not found project file [%s], initialise Echo Engine failed.", m_config.m_projectFile.c_str());
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
		RenderPipeline::instance()->onSize(windowWidth, windowHeight);

		return true;
	}

	void Engine::destroy()
	{
		EchoSafeDeleteInstance(NodeTree);
		EchoSafeDeleteInstance(ImageCodecMgr);
		EchoSafeDeleteInstance(IO);
		EchoSafeDeleteInstance(Time);	
		EchoLogInfo("Echo Engine has been shutdown.");
		EchoSafeDeleteInstance(Renderer);
		EchoLogInfo("Echo Renderer has been shutdown.");
		EchoSafeDeleteInstance(Log);
		EchoSafeDeleteInstance(LuaBinder);
		EchoSafeDeleteInstance(GameSettings);
		EchoSafeDeleteInstance(Input);
		EchoSafeDeleteInstance(PluginSettings);
		EchoSafeDeleteInstance(RenderStage);
		EchoSafeDeleteInstance(RenderPipeline);
        
        Module::clear();
        Class::clear();
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
        Renderer::instance()->beginRender();
		RenderStage::instance()->process();
		Renderer::instance()->present();
	}
    
    Engine* initEngine(const String& project, bool isGame)
    {
        Echo::Engine::Config rootcfg;
        rootcfg.m_projectFile = project;
        rootcfg.m_isGame = isGame;

#ifdef ECHO_PLATFORM_WINDOWS
        rootcfg.m_userPath = PathUtil::GetCurrentDir() + "/user/" + StringUtil::Format("u%d/", BKDRHash(project.c_str()));
        PathUtil::FormatPath(rootcfg.m_userPath);
#else

#endif

        Engine::instance()->initialize(rootcfg);

		return Engine::instance();
    }
}
