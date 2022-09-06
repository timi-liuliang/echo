#pragma once

#include <QHBoxLayout>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <nodeeditor/NodeDataModel>
#include <engine/core/util/StringUtil.h>
#include <engine/core/render/base/shader/shader_program.h>
#include "Scene/ShaderScene.h"
#include "Scene/ShaderView.h"
#include <engine/core/render/base/shader/editor/compiler/shader_compiler_opaque.h>
#include <engine/core/render/base/shader/editor/compiler/shader_compiler_transparent.h>
#include <engine/core/render/base/shader/editor/compiler/shader_compiler_lighting.h>
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
        // Replace template
        void replaceTemplate();

        // open
        void open(const Echo::String& resPath);
        
        // save
        void save();

    protected:
        // Get template name
        Echo::String getTemplateName();

	private:
        QTimer				                m_timer;
        bool                                m_isLoading = false;
        Echo::ShaderProgramPtr              m_shaderProgram;
		DataFlowProgramming::ShaderScene*	m_graphicsScene = nullptr;
		DataFlowProgramming::ShaderView*	m_graphicsView = nullptr;
        Echo::ShaderCompiler*               m_shaderCompiler = nullptr;
	};
}
