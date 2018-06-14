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
		, m_mainScene("", ".scene")
	{
		registerProperty(ECHO_CLASS_NAME(ProjectSettings), "Window.Width", Variant::Type::Int);
		registerProperty(ECHO_CLASS_NAME(ProjectSettings), "Window.Height", Variant::Type::Int);
		registerProperty(ECHO_CLASS_NAME(ProjectSettings), "Game.MainScene", Variant::Type::ResourcePath);
	}

	// 析构函数
	ProjectSettings::~ProjectSettings()
	{

	}

	// bind methods to script
	void ProjectSettings::bindMethods() 
	{
	}

	// get property value
	bool ProjectSettings::getPropertyValue(const String& propertyName, Variant& oVar)
	{
		if (propertyName == "Window.Width") 
		{
			oVar = m_windowWidth; 
			return true; 
		}
		else if (propertyName == "Window.Height") 
		{
			oVar = m_windowHeight; 
			return true;
		}
		else if (propertyName == "Game.MainScene") 
		{
			oVar = m_mainScene; 
			return true;
		}
		
		return false;
	}
}