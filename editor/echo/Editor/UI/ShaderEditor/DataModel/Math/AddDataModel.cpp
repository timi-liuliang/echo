#include "AddDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector3.h"
#include "DataVector4.h"
#include "ShaderScene.h"

namespace DataFlowProgramming
{
    AddDataModel::AddDataModel()
    {
        m_inputDataTypes = 
        {
            {"vec4", "A"},
            {"vec4", "B"},
        };

        m_inputs.resize(m_inputDataTypes.size());

        m_outputs.resize(1);
        m_outputs[0] = std::make_shared<DataVector4>(this, "");
        m_outputs[0]->setVariableName(getVariableName());
    }

    QJsonObject AddDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();
        return modelJson;
    }

    void AddDataModel::restore(QJsonObject const &p)
    {
    }

    unsigned int AddDataModel::nPorts(PortType portType) const
    {
        unsigned int result = 1;

        switch (portType)
        {
            case PortType::In: result = m_inputs.size(); break;
            case PortType::Out:result = m_outputs.size(); break;
            default:                       break;
        }

        return result;
    }

    NodeDataType AddDataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        if(portType==PortType::In)
        {
            return m_inputDataTypes[portIndex];
        }
        else if (portType == PortType::Out)
        {
            return m_outputs[portIndex]->type();
        }
        
        return NodeDataType {"unknown", "Unknown"};
    }

    std::shared_ptr<NodeData> AddDataModel::outData(PortIndex portIndex)
    {
        return m_outputs[portIndex];
    }

    void AddDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex port)
    {
        m_inputs[port] = std::dynamic_pointer_cast<ShaderData>(nodeData);
    }

    bool AddDataModel::generateCode(ShaderCompiler& compiler)
    {
        if (m_inputs[0] && m_inputs[1])
        {
            compiler.addCode(Echo::StringUtil::Format("\tvec4 %s = %s + %s;\n", 
                m_outputs[0]->getVariableName().c_str(),
                m_inputs[0]->getVariableName().c_str(),
                m_inputs[1]->getVariableName().c_str()));
        }

        return true;
    }
}
