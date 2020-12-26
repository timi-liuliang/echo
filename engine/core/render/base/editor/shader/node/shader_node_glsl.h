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

		// inputs
		const String& getInputs() const { return m_inputs; }
		void setInputs(const String& inputs) { m_inputs = inputs; }

		// code
		const String& getCode() const { return m_code; }
		void setCode(const String& code) { m_code = code; }

		// return type
		const StringOption& getReturnType() const { return m_returnType; }
		void setReturnType(const StringOption& type) { m_returnType.setValue(type.getValue()); }

	public:
		// get input data types
		virtual DataTypes getInputDataTypes() override;

	private:
		String			m_inputs;
		String			m_code;
		StringOption	m_returnType = StringOption("float", { "float", "vec2", "vec3", "vec4" });
	};

#endif
}
