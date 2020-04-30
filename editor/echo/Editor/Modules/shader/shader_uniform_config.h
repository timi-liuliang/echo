#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	class ShaderUniform : public Echo::Object
	{
		ECHO_CLASS(ShaderUniform, Object)

	public:
		ShaderUniform();
		virtual ~ShaderUniform();

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
}