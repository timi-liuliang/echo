#include "engine/core/base/class.h"
#include "engine/core/editor/importer.h"
#include "engine/core/editor/importer/file_importer.h"
#include "engine/core/main/Engine.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/hash_generator.h"
#include "engine/core/util/Exception.h"
#include "engine/core/io/IO.h"
#include "engine/core/log/Log.h"
#include "engine/core/localization/Localization.h"
#include "engine/core/render/base/image/image_codec_mgr.h"
#include "engine/core/render/base/misc/view_port.h"
#include "engine/core/render/base/shader/material.h"
#include "engine/core/render/base/pipeline/render_stage.h"
#include "engine/core/render/base/shader/shader_program.h"
#include "engine/core/render/base/texture/texture_cube.h"
#include "engine/core/scene/render_node.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/util/Timer.h"
#include "game_settings.h"
#include "plugin_settings.h"
#include "engine/core/script/lua/register_core_to_lua.cx"
#include "engine/core/script/lua/lua_binder.h"
#include "engine/core/script/lua/lua_script.h"
#include "engine/core/script/scratch/scratch.h"
#include "engine/core/script/scratch/editor/scratch_editor.h"
#include "Plugin.h"
#include "module.h"
#include "engine/core/render/gles/gles.h"
#include "engine/core/render/vulkan/vk.h"
#include "engine/core/render/metal/mt.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/input/input.h"
#include "engine/core/terminal/terminal.h"

namespace Echo
{
	Engine::Engine()
		: m_isInited(false)
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

	void Engine::bindMethods()
	{
		CLASS_BIND_METHOD(Engine, getFrameTime);
	}

	void Engine::initializeDll()
	{

	}

	bool Engine::initialize(const Config& cfg)
	{
		m_config = cfg;

        ImageCodecMgr::instance();
        IO::instance();

		// Engine root path
		setRootPath(m_config.m_rootPath);
        
		// check root path
		setlocale(LC_ALL, "zh_CN.UTF-8");
        if (PathUtil::IsFileExist( m_config.m_projectFile))
        {
            String resPath = PathUtil::GetFileDirPath(m_config.m_projectFile);
            IO::instance()->setResPath(resPath);
        }
        else
        {
            EchoLogError("Set root path failed [%s], initialise Echo Engine failed.", cfg.m_projectFile.c_str());
            return false;
        }

        IO::instance()->setUserPath( m_config.m_userPath);
		IO::instance()->setEngineResPath(m_config.m_engineResPath);

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

		// register class types in core
		Class::registerType<Object>();
        Class::registerType<FrameState>();
		Class::registerType<Engine>();
		Class::registerType<Renderer>();
		Class::registerType<Node>();
		Class::registerType<Render>();
		Class::registerType<Module>();
		Class::registerType<Res>();
		Class::registerType<LuaScript>();
		Class::registerType<Scratch>();
		Class::registerType<GameSettings>();
		Class::registerType<Gizmos>();
		Class::registerType<Input>();
        Class::registerType<IO>();
        Class::registerType<DataStream>();
		Class::registerType<Log>();

		Class::registerType<Translator>();
		Class::registerType<Localization>();

		Class::registerType<Command>();
		Class::registerType<Terminal>();

	#ifdef ECHO_EDITOR_MODE
		Class::registerType<Importer>();
		Class::registerType<FileImporter>();
	#endif
       
		CLASS_REGISTER_EDITOR(Scratch, ScratchEditor)

		// register render classes
		Renderer::registerClassTypes();

		// load all plugin
		Plugin::loadAllPlugins();

		// register all module
		Module::registerAllTypes();
	}

	void Engine::loadSettings()
	{
		if (PathUtil::IsFileExist(m_config.m_projectFile))
		{
			String projectFile;
			if (IO::instance()->convertFullPathToResPath(m_config.m_projectFile, projectFile))
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
				Object* obj = Echo::Class::create(className);
				if (obj)
				{
					pugi::xml_node classNode = root.append_child(className.c_str());
					if (classNode)
						Object::savePropertyRecursive(&classNode, obj, obj->getClassName());
				}
			}
		}

		doc.save_file(m_config.m_projectFile.c_str(), "\t", 1U, pugi::encoding_utf8);
	}

	const String& Engine::getResPath()
	{
		return IO::instance()->getResPath();
	}

	const String& Engine::getUserPath()
	{
		return IO::instance()->getUserPath();
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

		return true;
	}

	void Engine::destroy()
	{
		Res::clear();

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
		EchoSafeDeleteInstance(FrameState);
		EchoSafeDeleteInstance(Localization);
        
        Module::clear();
        Class::clear();
	}

	void Engine::tick(float elapsedTime)
	{
		Time::instance()->tick();

        FrameState::instance()->reset();
        FrameState::instance()->tick(elapsedTime);
        
		// calculate time
		elapsedTime = Math::Clamp( elapsedTime, 0.f, 1.f);
		m_frameTime = elapsedTime;

		// input update
		Input::instance()->update();

		// res
		Res::updateAll(m_frameTime);

		// update logic
		Module::updateAll(m_frameTime);
		NodeTree::instance()->update(m_frameTime);

		// render
		Renderer::instance()->render();
	}
}
