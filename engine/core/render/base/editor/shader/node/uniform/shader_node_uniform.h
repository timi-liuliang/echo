#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeUniform : public ShaderNode
	{
		ECHO_VIRTUAL_CLASS(ShaderNodeUniform, ShaderNode)

	public:
		ShaderNodeUniform();
		virtual ~ShaderNodeUniform();

		// caption
		virtual QString caption() const override { return getVariableName().c_str(); }

		// is caption visible
		virtual bool captionVisible() const override { return m_isUniform; }

		// category
		virtual QString category() const override { return "Uniforms"; }

		// constant
		virtual bool isUniform() const { return m_isUniform; }
		void setUniform(bool isUniform) { m_isUniform = isUniform; }

	protected:
		bool        m_isUniform = false;
	};
#endif
}