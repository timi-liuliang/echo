#include "plugin_settings.h"
#include "engine/core/log/Log.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/base/renderer.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node_tree.h"
#include <ostream>

namespace Echo
{
	PluginSettings::PluginSettings()
	{
	}

	PluginSettings::~PluginSettings()
	{

	}

	PluginSettings* PluginSettings::instance()
	{
		static PluginSettings* inst = EchoNew(PluginSettings);
		return inst;
	}

	// bind methods to script
	void PluginSettings::bindMethods()
	{
		CLASS_BIND_METHOD(PluginSettings, getSearchPath);
		CLASS_BIND_METHOD(PluginSettings, setSearchPath);

		CLASS_REGISTER_PROPERTY(PluginSettings, "SearchPath", Variant::Type::String, getSearchPath, setSearchPath);
	}
}
