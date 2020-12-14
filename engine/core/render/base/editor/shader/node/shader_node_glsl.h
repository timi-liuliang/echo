#pragma once

#include "shader_node.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeGLSL : public ShaderNode
	{
		ECHO_CLASS(ShaderNodeGLSL, ShaderNode)

	public:
		ShaderNodeGLSL();
		virtual ~ShaderNodeGLSL();

		// code
		const String& getCode() const { return m_code; }
		void setCode(const String& code) { m_code = code; }

	private:
		String		m_code;
	};

#endif
}
