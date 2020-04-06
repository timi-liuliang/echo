#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	class ShaderUniformConfig : public Echo::Object
	{
		ECHO_CLASS(ShaderUniformConfig, Object)

	public:
		ShaderUniformConfig();
		virtual ~ShaderUniformConfig();

		// variable name
		const String& getVariableName() const { return m_variableName; }
		void setVariableName(const String& variableName);

	public:
		DECLARE_SIGNAL(Signal0, onVariableNameChanged)

	protected:
		String		m_variableName;
	};
}