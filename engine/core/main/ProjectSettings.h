#pragma once

#include "engine/core/base/variant.h"
#include "engine/core/Util/StringUtil.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include "engine/core/resource/Res.h"

namespace Echo
{
	/**
	 * 项目文件
	 */
	class ProjectSettings : public Res
	{
		ECHO_RES(ProjectSettings, Res, ".echo", Res::create<ProjectSettings>, Res::load);

	public:
		ProjectSettings();
		~ProjectSettings();

	protected:
		// get property value
		virtual bool getPropertyValue(const String& propertyName, Variant& oVar);

	private:
		i32				m_windowWidth;
		i32				m_windowHeight;
		ResourcePath	m_mainScene;
	};
}
