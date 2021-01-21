#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeTextureSize : public ShaderNode
	{
		ECHO_CLASS(ShaderNodeTextureSize, ShaderNode)

	public:
		ShaderNodeTextureSize();
		virtual ~ShaderNodeTextureSize();

		// name
		virtual QString name() const override { return "TextureSize"; }

		// caption
		virtual QString caption() const override { return "TextureSize"; }

		// category
		virtual QString category() const override { return "Texture"; }

		// set in
		virtual void setInData(std::shared_ptr<NodeData> nodeData, int portIndex) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;
	};

#endif
}
