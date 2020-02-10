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
        ShaderDataModel(){}
        virtual ~ShaderDataModel() {}

        // generat code
        virtual bool generateCode(std::string& paramCode, std::string& shaderCode)=0;
    };
}

