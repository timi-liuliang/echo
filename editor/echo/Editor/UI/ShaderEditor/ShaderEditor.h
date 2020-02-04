#pragma once

#include <QHBoxLayout>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <nodeeditor/NodeDataModel>
#include <engine/core/util/StringUtil.h>
#include <engine/core/render/base/ShaderProgram.h>
#include "ui_ShaderEditor.h"

namespace Studio
{
	class ShaderEditor : public QDockWidget , public Ui_ShaderEditorPanel
	{
		Q_OBJECT

	public:
		ShaderEditor(QWidget* parent = 0);
		~ShaderEditor();
        
        void visitorAllNodes(QtNodes::NodeDataModel* dataModel);
        
        // compile
        void compile();
        
    public slots:
        // open
        void open(const Echo::String& resPath);
        
        // save
        void save();

	private:
        Echo::ShaderProgramPtr  m_shaderProgram;
		QGraphicsScene*		    m_graphicsScene = nullptr;
		QGraphicsView*		    m_graphicsView = nullptr;
        std::string             m_paramCode;
        std::string             m_shaderCode;
        Echo::String            m_result;
	};
}
