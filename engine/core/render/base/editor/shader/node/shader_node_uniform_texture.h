#pragma once

#include "shader_node_uniform.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeUniformTexture : public ShaderNodeUniform
	{
		ECHO_CLASS(ShaderNodeUniformTexture, ShaderNodeUniform)

	public:
		ShaderNodeUniformTexture();
		virtual ~ShaderNodeUniformTexture();

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
#endif
}