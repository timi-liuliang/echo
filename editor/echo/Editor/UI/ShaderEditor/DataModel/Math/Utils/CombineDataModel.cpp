#include "CombineDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector2.h"
#include "DataVector3.h"
#include "DataVector4.h"
#include "DataInvalid.h"
#include "ShaderScene.h"
#include "OperationRules.h"

namespace DataFlowProgramming
{
	CombineDataModel::CombineDataModel()
	{
		m_inputDataTypes =
		{
			{"float", "r"},
			{"float", "g"},
			{"float", "b"},
			{"float", "a"},
		};

		m_inputs.resize(m_inputDataTypes.size());

		m_outputs.resize(3);
		m_outputs[0] = std::make_shared<DataVector4>(this, "rgba");
		m_outputs[1] = std::make_shared<DataVector3>(this, "rgb");
		m_outputs[2] = std::make_shared<DataVector2>(this, "rg");
	}

	QJsonObject CombineDataModel::save() const
	{
		QJsonObject modelJson = NodeDataModel::save();
		return modelJson;
	}

	void CombineDataModel::restore(QJsonObject const& p)
	{
	}

	unsigned int CombineDataModel::nPorts(PortType portType) const
	{
		switch (portType)
		{
		case PortType::In: return m_inputs.size();
		case PortType::Out:return m_outputs.size();
		default:           return 0;
		}
	}

	NodeDataType CombineDataModel::dataType(PortType portType, PortIndex portIndex) const
	{
		if (portType == PortType::In)
		{
			return m_inputDataTypes[portIndex];
		}
		else if (portType == PortType::Out)
		{
			return m_outputs[portIndex]->type();
		}

		return NodeDataType{ "invalid", "invalid" };
	}

	std::shared_ptr<NodeData> CombineDataModel::outData(PortIndex portIndex)
	{
		return m_outputs[portIndex];
	}

	void CombineDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex portIndex)
	{
		m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);

		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s", getVariableName().c_str()));
		m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s.xyz", getVariableName().c_str()));
		m_outputs[2]->setVariableName(Echo::StringUtil::Format("%s.xy", getVariableName().c_str()));

		for (size_t i = 0; i < m_outputs.size(); i++)
		{
			Q_EMIT dataUpdated(i);
		}

		checkValidation();
	}

	bool CombineDataModel::generateCode(ShaderCompiler& compiler)
	{
		Echo::String x = m_inputs[0] ? m_inputs[0]->getVariableName() : "0.0";
		Echo::String y = m_inputs[1] ? m_inputs[1]->getVariableName() : "0.0";
		Echo::String z = m_inputs[2] ? m_inputs[2]->getVariableName() : "0.0";
		Echo::String w = m_inputs[3] ? m_inputs[3]->getVariableName() : "0.0";

		compiler.addCode(Echo::StringUtil::Format("\tvec4 %s = vec4(%s, %s, %s, %s);\n", getVariableName().c_str(), x.c_str(), y.c_str(), z.c_str(), w.c_str()));

		return true;
	}
}
