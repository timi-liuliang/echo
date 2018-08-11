#include "GameSettings.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/render/Renderer.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/NodeTree.h"
#include <ostream>

namespace Echo
{
	GameSettings::GameSettings()
		: m_designWidth(1366)
		, m_designHeight(768)
		, m_launchScene("", ".scene")
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

	// get bg color
	const Color& GameSettings::getBackgroundColor() const
	{ 
		return Renderer::BGCOLOR;
	}

	// set bg color
	void GameSettings::setBackgroundColor(const Color& color)
	{ 
		Renderer::BGCOLOR = color;
	}

	// bind methods to script
	void GameSettings::bindMethods() 
	{
		CLASS_BIND_METHOD(GameSettings, getDesignWidth, DEF_METHOD("getDesignWidth"));
		CLASS_BIND_METHOD(GameSettings, setDesignWidth, DEF_METHOD("setDesignWidth"));
		CLASS_BIND_METHOD(GameSettings, getDesignHeight, DEF_METHOD("getDesignHeight"));
		CLASS_BIND_METHOD(GameSettings, setDesignHeight, DEF_METHOD("setDesignHeight"));
		CLASS_BIND_METHOD(GameSettings, getLaunchScene, DEF_METHOD("getLaunchScene"));
		CLASS_BIND_METHOD(GameSettings, setLaunchScene, DEF_METHOD("setLaunchScene"));
		CLASS_BIND_METHOD(GameSettings, getBackgroundColor, DEF_METHOD("getBackgroundColor"));
		CLASS_BIND_METHOD(GameSettings, setBackgroundColor, DEF_METHOD("setBackgroundColor"));

		CLASS_REGISTER_PROPERTY(GameSettings, "Background", Variant::Type::Color, "getBackgroundColor", "setBackgroundColor");
		CLASS_REGISTER_PROPERTY(GameSettings, "DesignWidth", Variant::Type::Int, "getDesignWidth", "setDesignWidth");
		CLASS_REGISTER_PROPERTY(GameSettings, "DesignHeight", Variant::Type::Int, "getDesignHeight", "setDesignHeight");
		CLASS_REGISTER_PROPERTY(GameSettings, "LaunchScene", Variant::Type::ResourcePath, "getLaunchScene", "setLaunchScene");
	}

	// set design width
	void GameSettings::setDesignWidth(i32 width) 
	{ 
		m_designWidth = width;

		onSize( Renderer::instance()->getScreenWidth(), Renderer::instance()->getScreenHeight());
	}

	// set design height
	void GameSettings::setDesignHeight(i32 height) 
	{ 
		m_designHeight = height;

		onSize(Renderer::instance()->getScreenWidth(), Renderer::instance()->getScreenHeight());
	}

	// on size
	void GameSettings::onSize(ui32 windowWidth, ui32 windowHeight)
	{
		if (Engine::instance()->getConfig().m_isGame)
		{
			Camera* pMainCamera = NodeTree::instance()->get3dCamera();
			pMainCamera->setWidth(Real(windowWidth));
			pMainCamera->setHeight(Real(windowHeight));
			pMainCamera->update();

			Camera* p2DCamera = NodeTree::instance()->get2dCamera();
			p2DCamera->setWidth(Real(getDesignWidth()));
			p2DCamera->setHeight(Real(getDesignHeight()));
			p2DCamera->update();

		}
		else
		{
			Camera* pMainCamera = NodeTree::instance()->get3dCamera();
			pMainCamera->setWidth(Real(windowWidth));
			pMainCamera->setHeight(Real(windowHeight));
			pMainCamera->update();

			Camera* p2DCamera = NodeTree::instance()->get2dCamera();
			p2DCamera->setWidth(Real(windowWidth));
			p2DCamera->setHeight(Real(windowHeight));
			p2DCamera->update();
		}
	}
}
