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
		PluginSettings();
		~PluginSettings();

		// instance
		static PluginSettings* instance();

		// set plugin path
		void setSearchPath(const String& path) { m_pluginSearchPath = path; }
		const String& getSearchPath() const { return m_pluginSearchPath; }

	private:
        String	m_pluginSearchPath = "${EchoDir}/plugins/;${ProjectDir}/plugins/;";
	};
}
