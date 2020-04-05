#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include <string>
#include "DataFloat.h"
#include "QColorSelect.h"
#include "shader_uniform_config.h"
#include "ShaderDataModel.h"
#include "Compiler/ShaderCompiler.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    class ShaderUniformDataModel : public ShaderDataModel
    {
      Q_OBJECT

    public:
        ShaderUniformDataModel();
        virtual ~ShaderUniformDataModel() {}

		// caption
		virtual QString caption() const override;

        // caption visible
        virtual bool captionVisible() const override { return m_isParameter; }

        // variable name
        virtual Echo::String getVariableName() const;

        // show menu
        virtual void showMenu(const QPointF& pos);

        // uniform config
        void saveUniformConfig(QJsonObject& p) const;
		void restoreUniformConfig(QJsonObject const& p);

    public:
        // slot
        virtual void onDoubleClicked() override;

	protected:
		bool                        m_isParameter = false;
		Echo::ShaderUniformConfig*  m_uniformConfig = nullptr;

		QMenu*                      m_menu = nullptr;
		QAction*                    m_setAsParameter = nullptr;
		QAction*                    m_setAsConstant = nullptr;
    };
}

