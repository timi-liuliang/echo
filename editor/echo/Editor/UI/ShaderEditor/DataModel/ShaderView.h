#pragma once

#include <nodeeditor/FlowScene>
#include <nodeeditor/FlowView>
#include "ShaderEditor.h"

namespace DataFlowProgramming
{
    class ShaderView : public QtNodes::FlowView
    {
      Q_OBJECT

    public:
        ShaderView(QWidget* parent = Q_NULLPTR);
        ShaderView(QtNodes::FlowScene* scene, QWidget* parent = Q_NULLPTR);
        virtual ~ShaderView();

    public:
        // contex menu
        virtual void contextMenuEvent(QContextMenuEvent* event) override;
    };
}

