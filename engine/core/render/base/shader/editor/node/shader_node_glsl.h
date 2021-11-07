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

		// caption
		virtual QString caption() const override { return m_funName.empty() ? "GLSL" : m_funName.c_str(); }

		// category
		virtual QString category() const override { return "Custom"; }

	public:
		// caption
		String getFunctionName() const;
		void setFunctionName(const String& funName);

		// inputs
		const String& getParms() const { return m_parameters; }
		void setParms(const String& params);

		// code
		String getCode() const;
		void setCode(const String& code);

		// return type
		const StringOption& getReturnType() const { return m_returnType; }
		void setReturnType(const StringOption& type);

	public:
		// get input data types
		QtNodes::NodeDataTypes getInputDataTypes(const String& inputs);

		// check validation
		virtual bool checkValidation() override;

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) override;

	private:
		// is params valid
		bool isParamsValid(const String& params);

		// calculate hash
		ui32 getHash();

	private:
		String			m_funName;
		String			m_parameters;
		String			m_body = "{\n\treturn 1.0;\n}";
		StringOption	m_returnType = StringOption("float", { "float", "vec2", "vec3", "vec4" });
	};

#endif
}
