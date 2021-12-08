#include "game_settings.h"
#include "engine/core/log/Log.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/base/renderer.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node_tree.h"
#include <ostream>

namespace Echo
{
	GameSettings::GameSettings()
		: m_designWidth(1366)
		, m_designHeight(768)
		, m_windowWidth(-1)
		, m_windowHeight(-1)
		, m_aspect("stretch", {"none", "stretch", "keep width", "keep height", "keep in", "keep out"})
        , m_launchScene("", ".scene")
		, m_renderPipelinePath("", ".pipeline")
	{
	}

	GameSettings::~GameSettings()
	{

	}

	GameSettings* GameSettings::instance()
	{
		static GameSettings* inst = EchoNew(GameSettings);
		return inst;
	}

	void GameSettings::bindMethods() 
	{
        CLASS_BIND_METHOD(GameSettings, isFullScreen);
        CLASS_BIND_METHOD(GameSettings, setFullScreen);
		CLASS_BIND_METHOD(GameSettings, getDesignWidth);
		CLASS_BIND_METHOD(GameSettings, setDesignWidth);
		CLASS_BIND_METHOD(GameSettings, getDesignHeight);
		CLASS_BIND_METHOD(GameSettings, setDesignHeight);
		CLASS_BIND_METHOD(GameSettings, getWindowWidth);
		CLASS_BIND_METHOD(GameSettings, setWindowWidth);
		CLASS_BIND_METHOD(GameSettings, getWindowHeight);
		CLASS_BIND_METHOD(GameSettings, setWindowHeight);
		CLASS_BIND_METHOD(GameSettings, getAspect);
		CLASS_BIND_METHOD(GameSettings, setAspect);
		CLASS_BIND_METHOD(GameSettings, getRenderPipeline);
		CLASS_BIND_METHOD(GameSettings, setRenderPipeline);
		CLASS_BIND_METHOD(GameSettings, getLaunchScene);
		CLASS_BIND_METHOD(GameSettings, setLaunchScene);

        CLASS_REGISTER_PROPERTY(GameSettings, "FullScreen", Variant::Type::Int, isFullScreen, setFullScreen);
		CLASS_REGISTER_PROPERTY(GameSettings, "DesignWidth", Variant::Type::Int, getDesignWidth, setDesignWidth);
		CLASS_REGISTER_PROPERTY(GameSettings, "DesignHeight", Variant::Type::Int, getDesignHeight, setDesignHeight);
		CLASS_REGISTER_PROPERTY(GameSettings, "WindowWidth", Variant::Type::Int, getWindowWidth, setWindowWidth);
		CLASS_REGISTER_PROPERTY(GameSettings, "WindowHeight", Variant::Type::Int, getWindowHeight, setWindowHeight);
		CLASS_REGISTER_PROPERTY(GameSettings, "Aspect", Variant::Type::StringOption, getAspect, setAspect);
		CLASS_REGISTER_PROPERTY(GameSettings, "RenderPipeline", Variant::Type::ResourcePath, getRenderPipeline, setRenderPipeline);
		CLASS_REGISTER_PROPERTY(GameSettings, "LaunchScene", Variant::Type::ResourcePath, getLaunchScene, setLaunchScene);
	}

    void GameSettings::setFullScreen(bool fullScreen)
    {
        m_fullScreen = fullScreen;
    }

	void GameSettings::setDesignWidth(i32 width) 
	{ 
		m_designWidth = width;

		onSize( Renderer::instance()->getWindowWidth(), Renderer::instance()->getWindowHeight());
	}

	void GameSettings::setDesignHeight(i32 height) 
	{ 
		m_designHeight = height;

		onSize(Renderer::instance()->getWindowWidth(), Renderer::instance()->getWindowHeight());
	}

	void GameSettings::setAspect(const StringOption& option)
	{
		m_aspect.setValue(option.getValue());

		onSize(Renderer::instance()->getWindowWidth(), Renderer::instance()->getWindowHeight());
	}

	void GameSettings::keepAspect(ui32 windowWidth, ui32 windowHeight, KeepAspectType type, Camera* camera)
	{
		if (camera)
		{
			if (type == KeepAspectType::None)
			{
				camera->setWidth(windowWidth);
				camera->setHeight(windowHeight);
			}
			else if (type == KeepAspectType::Stretch)
			{
				camera->setWidth(getDesignWidth());
				camera->setHeight(Real(getDesignHeight()));
			}
			if (type == KeepAspectType::Width)
			{
				Real aspect = (Real)windowHeight / windowWidth;
				camera->setWidth(Real(getDesignWidth()));
				camera->setHeight(Real(getDesignWidth() * aspect));
			}
			else if (type == KeepAspectType::Height)
			{
				Real aspect = (Real)windowWidth / windowHeight;
				camera->setWidth(Real(getDesignHeight() * aspect));
				camera->setHeight(Real(getDesignHeight()));
			}

			camera->update();
		}
	}

	void GameSettings::keepAspect(ui32 windowWidth, ui32 windowHeight, KeepAspectType type)
	{
		// 3d camera
		Camera* pMainCamera = NodeTree::instance()->get3dCamera();
		pMainCamera->setWidth(windowWidth);
		pMainCamera->setHeight(windowHeight);
		pMainCamera->update();

		// 2d camera
		keepAspect(windowWidth, windowHeight, type, NodeTree::instance()->get2dCamera());

		// gui camera
		keepAspect(windowWidth, windowHeight, type, NodeTree::instance()->getUiCamera());
	}

	void GameSettings::onSize(ui32 windowWidth, ui32 windowHeight)
	{
		if (Engine::instance()->getConfig().m_isGame)
		{
			if (m_aspect.getIdx() <= 3)
			{
				keepAspect(windowWidth, windowHeight, KeepAspectType( m_aspect.getIdx()));
			}
			else
			{
				float wRatio = (float)windowWidth / getDesignWidth();
				float hRatio = (float)windowHeight / getDesignHeight();
				if (m_aspect.getIdx() == 4)
					keepAspect(windowWidth, windowHeight, wRatio > hRatio ? KeepAspectType::Height : KeepAspectType::Width);
				else if (m_aspect.getIdx() == 5)
					keepAspect(windowWidth, windowHeight, wRatio > hRatio ? KeepAspectType::Width : KeepAspectType::Height);
			}
		}
		else
		{
			keepAspect(windowWidth, windowHeight, KeepAspectType::None);
		}
	}

	void GameSettings::setRenderPipeline(const ResourcePath& path)
	{ 
		m_renderPipelinePath.setPath(path.getPath());
		RenderPipeline::setCurrent(getRenderPipeline());
	}

	const ResourcePath& GameSettings::getRenderPipeline() const 
	{ 
		return m_renderPipelinePath.isEmpty() ? RenderPipeline::DefaultPipeline : m_renderPipelinePath; 
	}
}
