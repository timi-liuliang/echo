#pragma once

#include "engine/core/math/Math.h"

namespace Echo
{
	class RenderCamera
	{
	public:
		// Get global uniforms
		virtual void* getGlobalUniformValue(const String& name) { return nullptr; }
	};
}