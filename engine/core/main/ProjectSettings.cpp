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
	{
		registerProperty(ECHO_CLASS_NAME(ProjectSettings), "Window.Width", Variant::Type::Int);
		registerProperty(ECHO_CLASS_NAME(ProjectSettings), "Window.Height", Variant::Type::Int);

		//registerProperty(ECHO_CLASS_NAME(ProjectSettings), "Game.MainScene", Variant::Type::ResourcePath);
	}

	// 析构函数
	ProjectSettings::~ProjectSettings()
	{

	}
}