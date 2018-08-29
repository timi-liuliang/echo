#include "GameSettings.h"
#include "engine/core/log/Log.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/render/Renderer.h"
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
		, m_launchScene("", ".scene")
		, m_aspect("stretch", {"none", "stretch", "keep width", "keep height", "keep in", "keep out"})
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
		CLASS_BIND_METHOD(GameSettings, getWindowWidth, DEF_METHOD("getWindowWidth"));
		CLASS_BIND_METHOD(GameSettings, setWindowWidth, DEF_METHOD("setWindowWidth"));
		CLASS_BIND_METHOD(GameSettings, getWindowHeight, DEF_METHOD("getWindowHeight"));
		CLASS_BIND_METHOD(GameSettings, setWindowHeight, DEF_METHOD("setWindowHeight"));
		CLASS_BIND_METHOD(GameSettings, getAspect, DEF_METHOD("getAspect"));
		CLASS_BIND_METHOD(GameSettings, setAspect, DEF_METHOD("setAspect"));
		CLASS_BIND_METHOD(GameSettings, getLaunchScene, DEF_METHOD("getLaunchScene"));
		CLASS_BIND_METHOD(GameSettings, setLaunchScene, DEF_METHOD("setLaunchScene"));
		CLASS_BIND_METHOD(GameSettings, getBackgroundColor, DEF_METHOD("getBackgroundColor"));
		CLASS_BIND_METHOD(GameSettings, setBackgroundColor, DEF_METHOD("setBackgroundColor"));

		CLASS_REGISTER_PROPERTY(GameSettings, "Background", Variant::Type::Color, "getBackgroundColor", "setBackgroundColor");
		CLASS_REGISTER_PROPERTY(GameSettings, "DesignWidth", Variant::Type::Int, "getDesignWidth", "setDesignWidth");
		CLASS_REGISTER_PROPERTY(GameSettings, "DesignHeight", Variant::Type::Int, "getDesignHeight", "setDesignHeight");
		CLASS_REGISTER_PROPERTY(GameSettings, "WindowWidth", Variant::Type::Int, "getWindowWidth", "setWindowWidth");
		CLASS_REGISTER_PROPERTY(GameSettings, "WindowHeight", Variant::Type::Int, "getWindowHeight", "setWindowHeight");
		CLASS_REGISTER_PROPERTY(GameSettings, "Aspect", Variant::Type::StringOption, "getAspect", "setAspect");
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

	void GameSettings::setAspect(const StringOption& option)
	{
		m_aspect.setValue(option.getValue());

		onSize(Renderer::instance()->getScreenWidth(), Renderer::instance()->getScreenHeight());
	}

	// keep aspect
	void GameSettings::keepAspect(ui32 windowWidth, ui32 windowHeight, KeepAspectType type)
	{
		// 3d camera
		Camera* pMainCamera = NodeTree::instance()->get3dCamera();
		pMainCamera->setWidth(Real(windowWidth));
		pMainCamera->setHeight(Real(windowHeight));
		pMainCamera->update();

		// 2d camera
		Camera* p2DCamera = NodeTree::instance()->get2dCamera();
		if (type == KeepAspectType::None)
		{
			p2DCamera->setWidth(Real(windowWidth));
			p2DCamera->setHeight(Real(windowHeight));
		}
		else if (type == KeepAspectType::Stretch)
		{
			p2DCamera->setWidth(Real(getDesignWidth()));
			p2DCamera->setHeight(Real(getDesignHeight()));
		}
		if (type==KeepAspectType::Width)
		{
			Real aspect = (Real)windowHeight / windowWidth;
			p2DCamera->setWidth(Real(getDesignWidth()));
			p2DCamera->setHeight(Real(getDesignWidth() * aspect));
		}
		else if (type == KeepAspectType::Height)
		{
			Real aspect = (Real)windowWidth / windowHeight;
			p2DCamera->setWidth(Real(getDesignHeight() * aspect));
			p2DCamera->setHeight(Real(getDesignHeight()));
		}

		p2DCamera->update();
	}

	// on size
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
}
