#pragma once

#include "engine/core/util/StringUtil.h"
#include "engine/core/render/base/shader/shader_program.h"

namespace Echo
{
	class GltfMaterial
	{
	public:
		// get shader
		static ShaderProgramPtr getPbrMetalicRoughnessContent();
	};
}
