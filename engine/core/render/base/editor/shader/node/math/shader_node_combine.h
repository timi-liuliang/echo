#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class ShaderNodeCombine : public ShaderNode
	{
		Q_OBJECT

	public:
		ShaderNodeCombine();
		virtual ~ShaderNodeCombine() {}

		// name
		virtual QString name() const override { return QStringLiteral("Combine"); }

		// caption
		virtual QString caption() const override { return QStringLiteral("Combine"); }

		// is caption visible
		virtual bool captionVisible() const override { return true; }

		// category
		virtual QString category() const override { return "Math"; }

		// when input changed
		virtual void setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex port) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;
	};
}

#endif