#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNode : public Object
	{
		ECHO_CLASS(ShaderNode, Object)

	public:
		ShaderNode();
		virtual ~ShaderNode();

	private:
	};

#endif
}
