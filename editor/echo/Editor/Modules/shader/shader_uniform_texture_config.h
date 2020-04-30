#pragma once

#include "shader_uniform_config.h"

namespace Echo
{
	class ShaderUniformTexture : public ShaderUniform
	{
		ECHO_CLASS(ShaderUniformTexture, ShaderUniform)

	public:
		ShaderUniformTexture();
		virtual ~ShaderUniformTexture();

		// constant
		virtual bool isExport() const override { return true; }

		// type
		const StringOption& getType() { return m_type; }
		void setType(const StringOption& type) { m_type.setValue(type.getValue()); }

		// atla
		void setAtla(bool isAtla) { m_isAtla = isAtla; }
		bool isAtla() const { return m_isAtla; }

	protected:
		StringOption	m_type =  StringOption("General", { "General", "NormalMap"});
		bool			m_isAtla = false;
	};
}