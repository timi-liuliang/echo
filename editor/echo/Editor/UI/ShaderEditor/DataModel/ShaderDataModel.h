#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include <string>
#include "DataFloat.h"
#include "QColorSelect.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    /// The model dictates the number of inputs and outputs for the Node.
    /// In this example it has no logic.
    class ShaderDataModel : public NodeDataModel
    {
      Q_OBJECT

    public:
        ShaderDataModel();
        virtual ~ShaderDataModel() {}

		/// Caption is used in GUI
        virtual QString caption() const = 0;

        // variable name
        const Echo::String getVariableName();

        // generate code
        virtual bool generateCode(std::string& macroCode, std::string& paramCode, std::string& shaderCode)=0;

        // show menu
        virtual void showMenu(const QPointF& pos) {}

    private:
        Echo::ui32        m_id = 0;
        bool              m_isUsed = true;
        Echo::String      m_variableName;
    };
}

