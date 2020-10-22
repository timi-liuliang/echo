#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/math/Math.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/scene/node.h"
#include "engine/core/render/base/material.h"
#include "engine/modules/anim/anim_property.h"
#include <nlohmann/json.hpp>

#ifdef ECHO_EDITOR_MODE

using namespace Echo;

namespace Fbx
{
	class Loader
	{
	public:
		Loader();
		~Loader();
		bool load(const ResourcePath& path);

	private:
		ResourcePath		m_path;
	};
}
#endif
