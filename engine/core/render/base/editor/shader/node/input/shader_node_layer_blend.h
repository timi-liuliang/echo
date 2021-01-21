#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class ShaderNodeLayerBlend : public ShaderNode
	{
		ECHO_CLASS(ShaderNodeLayerBlend, ShaderNode)

	public:
		ShaderNodeLayerBlend();
		virtual ~ShaderNodeLayerBlend() {}

		// name
		virtual QString name() const override { return QStringLiteral("LayerBlend"); }

		// caption
		QString caption() const override { return QStringLiteral("LayerBlend"); }

		// is caption visible
		bool captionVisible() const override { return true; }

		// category
		virtual QString category() const override { return "Inputs"; }

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) override;

	private:
		Echo::i32	m_maxOutputNumber = 4;
	};
}

#endif
