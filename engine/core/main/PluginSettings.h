#pragma once

#include "engine/core/base/variant.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/math/color.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include "engine/core/resource/Res.h"

namespace Echo
{
	class PluginSettings : public Object
	{
		ECHO_SINGLETON_CLASS(PluginSettings, Object);

	public:
		enum class KeepAspectType
		{
			None,
			Stretch,
			Width,
			Height,
		};

	public:
		PluginSettings();
		~PluginSettings();

		// instance
		static PluginSettings* instance();

		// set plugin path
		void setSearchPath(const ResourcePath& path) { m_pluginSearchPath.setPath(path.getPath()); }
		const ResourcePath& getSearchPath() const { return m_pluginSearchPath; }

	private:
		ResourcePath	m_pluginSearchPath;
	};
}
