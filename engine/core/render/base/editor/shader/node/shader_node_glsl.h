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

		// name
		virtual QString name() const override { return "GLSL"; }

		// inputs
		const String& getParms() const { return m_parameters; }
		void setParms(const String& inputs);

		// code
		const String& getCode() const { return m_code; }
		void setCode(const String& code) { m_code = code; }

		// return type
		const StringOption& getReturnType() const { return m_returnType; }
		void setReturnType(const StringOption& type);

	public:
		// get input data types
		QtNodes::NodeDataTypes getInputDataTypes(const String& inputs);

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) override;

	private:
		String			m_parameters;
		String			m_code;
		StringOption	m_returnType = StringOption("float", { "float", "vec2", "vec3", "vec4" });
	};

#endif
}
