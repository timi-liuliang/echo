#pragma once

#include <QHBoxLayout>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <nodeeditor/NodeDataModel>
#include <engine/core/util/StringUtil.h>
#include <engine/core/render/base/shader_program.h>
#include "Scene/ShaderScene.h"
#include "Scene/ShaderView.h"
#include <engine/core/render/base/editor/shader/compiler/shader_compiler.h>
#include "ui_ShaderEditor.h"

namespace Studio
{
	class ShaderEditor : public QDockWidget , public Ui_ShaderEditorPanel
	{
		Q_OBJECT

	public:
		ShaderEditor(QWidget* parent = 0);
		~ShaderEditor();
        
        // show program properties
        void showShaderProgramProperties();

        // visitor nodes
        void visitorAllNodes(QtNodes::NodeDataModel* dataModel);
        void visitorUniformDefaultValues(QtNodes::NodeDataModel* dataModel);
        
        // compile
        void compile();
        
    public slots:
        // open
        void open(const Echo::String& resPath);
        
        // save
        void save();

	private:
        bool                                m_isLoading = false;
        Echo::ShaderProgramPtr              m_shaderProgram;
		DataFlowProgramming::ShaderScene*	m_graphicsScene = nullptr;
		DataFlowProgramming::ShaderView*	m_graphicsView = nullptr;
        Echo::ShaderCompiler                m_shaderCompiler;
	};
}
