#include "ProjectSettings.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/Util/PathUtil.h"
#include "engine/core/io/IO.h"
#include "engine/core/main/Engine.h"
#include <ostream>

namespace Echo
{
	// 构造函数
	ProjectSettings::ProjectSettings()
		: m_windowWidth(1366)
		, m_windowHeight(768)
		, m_launchScene("", ".scene")
	{
		
	}

	// 析构函数
	ProjectSettings::~ProjectSettings()
	{

	}

	// bind methods to script
	void ProjectSettings::bindMethods() 
	{
		CLASS_BIND_METHOD(ProjectSettings, getWindowWidth, DEF_METHOD("getWindowWidth"));
		CLASS_BIND_METHOD(ProjectSettings, setWindowWidth, DEF_METHOD("setWindowWidth"));
		CLASS_BIND_METHOD(ProjectSettings, getWindowHeight, DEF_METHOD("getWindowHeight"));
		CLASS_BIND_METHOD(ProjectSettings, setWindowHeight, DEF_METHOD("setWindowHeight"));
		CLASS_BIND_METHOD(ProjectSettings, getLaunchScene, DEF_METHOD("getLaunchScene"));
		CLASS_BIND_METHOD(ProjectSettings, setLaunchScene, DEF_METHOD("setLaunchScene"));

		CLASS_REGISTER_PROPERTY(ProjectSettings, "Window.Width", Variant::Type::Int, "getWindowWidth", "setWindowWidth");
		CLASS_REGISTER_PROPERTY(ProjectSettings, "Window.Height", Variant::Type::Int, "getWindowHeight", "setWindowHeight");
		CLASS_REGISTER_PROPERTY(ProjectSettings, "Game.LaunchScene", Variant::Type::Int, "getLaunchScene", "setLaunchScene");
	}

	// get property value
	bool ProjectSettings::getPropertyValue(const String& propertyName, Variant& oVar)
	{
		return false;
	}
}