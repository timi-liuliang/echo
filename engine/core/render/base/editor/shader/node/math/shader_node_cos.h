#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeCos : public ShaderNode
    {
        ECHO_CLASS(ShaderNodeCos, ShaderNode)

    public:
        ShaderNodeCos();
        virtual ~ShaderNodeCos() {}

		// name
		virtual QString name() const override { return QStringLiteral("Cos"); }

        // caption
        virtual QString caption() const override { return QStringLiteral("Cos"); }

        // is caption visible
        virtual bool captionVisible() const override { return true; }

		// when input changed
		virtual void setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex port) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

    public:
        // load|save
        virtual QJsonObject save() const override;
        virtual void restore(QJsonObject const &p) override;

    public:

    };
}

#endif
