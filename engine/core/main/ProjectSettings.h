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
		ECHO_RES(ProjectSettings, Res, ".echo");

	public:
		ProjectSettings();
		~ProjectSettings();

	private:
	};
}
