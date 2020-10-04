#include "engine/core/base/class.h"
#include "engine/core/editor/importer.h"
#include "engine/core/main/Engine.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/HashGenerator.h"
#include "engine/core/util/Exception.h"
#include "engine/core/io/IO.h"
#include "engine/core/log/Log.h"
#include "engine/core/localization/Localization.h"
#include "engine/core/render/base/image/image_codec_mgr.h"
#include "engine/core/render/base/atla/texture_atla.h"
#include "engine/core/render/base/atla/texture_atlas.h"
#include "engine/core/render/base/editor/atlas/texture_atla_editor.h"
#include "engine/core/render/base/editor/atlas/texture_atlas_editor.h"
#include "engine/core/render/base/view_port.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/pipeline/render_pipeline.h"
#include "engine/core/render/base/editor/pipeline/render_pipeline_editor.h"
#include "engine/core/render/base/pipeline/render_stage.h"
#include "engine/core/render/base/shader_program.h"
#include "engine/core/render/base/editor/shader/shader_editor.h"
#include "engine/core/render/base/texture_cube.h"
#include "engine/core/scene/render_node.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/util/Timer.h"
#include "GameSettings.h"
#include "PluginSettings.h"
#include "engine/core/script/lua/register_core_to_lua.cx"
#include "engine/core/script/lua/lua_binder.h"
#include "engine/core/script/lua/lua_script.h"
#include "Plugin.h"
#include "module.h"
#include "engine/core/render/gles/gles.h"
#include "engine/core/render/vulkan/vk.h"
#include "engine/core/render/metal/mt.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/input/input.h"
#include "engine/core/input/mouse_event.h"

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
		CLASS_BIND_METHOD(Engine, getFrameTime, "getFrameTime");
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

		// register class types in core
		Class::registerType<Object>();
        Class::registerType<FrameState>();
		Class::registerType<Engine>();
		Class::registerType<Node>();
		Class::registerType<Module>();
		Class::registerType<Render>();
		Class::registerType<Res>();
		Class::registerType<Texture>();
		Class::registerType<ShaderProgram>();
		Class::registerType<Material>();
		Class::registerType<Mesh>();
		Class::registerType<LuaScript>();
		Class::registerType<TextureCube>();
		Class::registerType<GameSettings>();
		Class::registerType<Gizmos>();
		Class::registerType<Input>();
		Class::registerType<MouseEvent>();
        Class::registerType<IO>();
        Class::registerType<DataStream>();
		Class::registerType<Log>();
		Class::registerType<TextureAtla>();
		Class::registerType<TextureAtlas>();
		Class::registerType<RenderPipeline>();
		Class::registerType<Translator>();
		Class::registerType<Localization>();

	#ifdef ECHO_EDITOR_MODE
		Class::registerType<Importer>();
	#endif
        
		REGISTER_OBJECT_EDITOR(ShaderProgram, ShaderEditor)
		REGISTER_OBJECT_EDITOR(TextureAtla, TextureAtlaEditor)
		REGISTER_OBJECT_EDITOR(TextureAtlas, TextureAtlasEditor)
		REGISTER_OBJECT_EDITOR(RenderPipeline, RenderPipelineEditor)

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
				pugi::xml_node classNode = root.append_child(className.c_str());
				if (classNode)
				{
					Object* obj = Echo::Class::create(className);
					if (obj)
					{
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
		EchoSafeDeleteInstance(FrameState);
		EchoSafeDeleteInstance(Localization);
        
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

	void Engine::tick(float elapsedTime)
	{
		Time::instance()->tick();

        FrameState::instance()->reset();
        FrameState::instance()->tick(elapsedTime);
        
		// calculate time
		elapsedTime = Math::Clamp( elapsedTime, 0.f, 1.f);
		m_frameTime = elapsedTime;

		// res
		Res::updateAll(m_frameTime);

		// update logic
		Module::updateAll(m_frameTime);
		NodeTree::instance()->update(m_frameTime);

		// input update
		Input::instance()->update();

		// render
		RenderPipeline::current()->render();
	}
}
