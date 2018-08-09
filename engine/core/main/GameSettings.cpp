#include "GameSettings.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/render/Renderer.h"
#include "engine/core/main/Engine.h"
#include <ostream>

namespace Echo
{
	GameSettings::GameSettings()
		: m_windowWidth(1366)
		, m_windowHeight(768)
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
		CLASS_BIND_METHOD(GameSettings, getWindowWidth, DEF_METHOD("getWindowWidth"));
		CLASS_BIND_METHOD(GameSettings, setWindowWidth, DEF_METHOD("setWindowWidth"));
		CLASS_BIND_METHOD(GameSettings, getWindowHeight, DEF_METHOD("getWindowHeight"));
		CLASS_BIND_METHOD(GameSettings, setWindowHeight, DEF_METHOD("setWindowHeight"));
		CLASS_BIND_METHOD(GameSettings, getLaunchScene, DEF_METHOD("getLaunchScene"));
		CLASS_BIND_METHOD(GameSettings, setLaunchScene, DEF_METHOD("setLaunchScene"));
		CLASS_BIND_METHOD(GameSettings, getBackgroundColor, DEF_METHOD("getBackgroundColor"));
		CLASS_BIND_METHOD(GameSettings, setBackgroundColor, DEF_METHOD("setBackgroundColor"));

		CLASS_REGISTER_PROPERTY(GameSettings, "Width", Variant::Type::Int, "getWindowWidth", "setWindowWidth");
		CLASS_REGISTER_PROPERTY(GameSettings, "Height", Variant::Type::Int, "getWindowHeight", "setWindowHeight");
		CLASS_REGISTER_PROPERTY(GameSettings, "LaunchScene", Variant::Type::ResourcePath, "getLaunchScene", "setLaunchScene");
		CLASS_REGISTER_PROPERTY(GameSettings, "Background", Variant::Type::Color, "getBackgroundColor", "setBackgroundColor");
	}
}
