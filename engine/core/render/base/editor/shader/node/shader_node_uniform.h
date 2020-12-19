#pragma once

#include "shader_node.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeUniform : public ShaderNode
	{
		ECHO_CLASS(ShaderNodeUniform, ShaderNode)

	public:
		ShaderNodeUniform();
		virtual ~ShaderNodeUniform();

		// constant
		virtual bool isExport() const { return m_isExport; }
		void setExport(bool isExport) { m_isExport = isExport; }

		// variable name
		const String& getVariableName() const { return m_variableName; }
		void setVariableName(const String& variableName);

	public:
		DECLARE_SIGNAL(Signal0, onVariableNameChanged)

	protected:
		bool        m_isExport = false;
		String		m_variableName;
	};
#endif
}