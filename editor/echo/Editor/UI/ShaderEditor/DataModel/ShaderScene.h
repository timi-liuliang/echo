#pragma once

#include <nodeeditor/FlowScene>
#include "ShaderEditor.h"

namespace DataFlowProgramming
{
    class ShaderScene : public QtNodes::FlowScene
    {
      Q_OBJECT

    public:
        ShaderScene(std::shared_ptr<QtNodes::DataModelRegistry> registry, QObject* parent = Q_NULLPTR);
        ShaderScene(QObject* parent = Q_NULLPTR);
        virtual ~ShaderScene();

        // shader editor
        void setShaderEditor(Studio::ShaderEditor* editor) { m_shaderEditor = editor; }

        // compile
        void compile();

    public:
        // override remove node
        virtual void removeNode(QtNodes::Node& node) override;

    protected:
        Studio::ShaderEditor* m_shaderEditor = nullptr;
    };
}

