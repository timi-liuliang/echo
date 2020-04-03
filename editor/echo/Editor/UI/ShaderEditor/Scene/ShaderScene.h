#pragma once

#include <nodeeditor/FlowScene>

namespace Studio { class ShaderEditor; }

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
        void setShaderEditor(class Studio::ShaderEditor* editor) { m_shaderEditor = editor; }

        // compile
        void compile();

        // get shader template node
        QtNodes::Node* getShaderTemplateNode();

    public:
        // override remove node
        virtual void removeNode(QtNodes::Node& node) override;

    public slots:
        // show menu
        void onShowShaderNodeMenu(QtNodes::Node& node, const QPointF& pos);

        // double clicked
        void onDoubleClickedNode(QtNodes::Node& node);

    protected:
        class Studio::ShaderEditor* m_shaderEditor = nullptr;
    };
}

