#include "shader_node.h"

#ifdef ECHO_EDITOR_MODE

#include <nodeeditor/Connection>

namespace Echo
{
	ShaderNode::ShaderNode()
		: Object()
	{
	}

	ShaderNode::~ShaderNode()
	{
		EditorApi.showObjectProperty(nullptr);
	}

	void ShaderNode::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNode, getVariableName);
		CLASS_BIND_METHOD(ShaderNode, setVariableName);

		CLASS_REGISTER_PROPERTY(ShaderNode, "Variable", Variant::Type::String, getVariableName, setVariableName);
	}

	unsigned int ShaderNode::nPorts(QtNodes::PortType portType) const
	{
		switch (portType)
		{
		case QtNodes::PortType::In: return m_inputs.size();
		case QtNodes::PortType::Out:return m_outputs.size();
		default:           return 0;
		}
	}

	NodeDataType ShaderNode::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
	{
		if (portType == QtNodes::PortType::In)
		{
			return m_inputDataTypes[portIndex];
		}
		else if (portType == QtNodes::PortType::Out)
		{
			return m_outputs[portIndex]->type();
		}

		return NodeDataType{ "invalid", "invalid" };
	}

	std::shared_ptr<NodeData> ShaderNode::outData(QtNodes::PortIndex portIndex)
	{
		if(m_outputs.size()>portIndex)
			return m_outputs[portIndex];
		else
			return std::make_shared<DataInvalid>(this);
	}

	void ShaderNode::setInData(std::shared_ptr<NodeData> nodeData, int portIndex)
	{
		m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);
	}

	bool ShaderNode::checkValidation()
	{
		m_modelValidationState = QtNodes::NodeValidationState::Valid;
		m_modelValidationError = QStringLiteral("");

		// check 
		if (m_inputDataTypes.size() != m_inputs.size())
		{
			m_modelValidationState = QtNodes::NodeValidationState::Error;
			m_modelValidationError = QStringLiteral("Inputs count error");

			return false;
		}

		// input type check
		for (size_t i = 0; i < m_inputDataTypes.size(); i++)
		{
			if (m_inputs[i] && m_inputs[i]->type().id != m_inputDataTypes[i].id)
			{
				m_modelValidationState = QtNodes::NodeValidationState::Error;
				m_modelValidationError = Echo::StringUtil::Format("Input [%d] type error", i).c_str();

				return false;
			}
		}

		// check invalid input
		for (size_t i = 0; i < m_inputs.size(); i++)
		{
			if (m_inputs[i] && m_inputs[i]->type().id == "invalid")
			{
				m_modelValidationState = QtNodes::NodeValidationState::Error;
				m_modelValidationError = Echo::StringUtil::Format("Input [%d] is invalid", i).c_str();

				return false;
			}
		}

		// check invalid output
		for (size_t i = 0; i < m_inputs.size(); i++)
		{
			if (m_inputs[i] && m_inputs[i]->type().id == "invalid")
			{
				m_modelValidationState = QtNodes::NodeValidationState::Error;
				m_modelValidationError = Echo::StringUtil::Format("Input [%d] is invalid", i).c_str();

				return false;
			}
			else if (m_inputs[i] && m_inputs[i]->type().id == "any")
			{
				std::shared_ptr<ShaderData> internalData = DataAny::getInternalData(m_inputs[i]);
				if (internalData && internalData->type().id == "invalid")
				{
					m_modelValidationState = QtNodes::NodeValidationState::Error;
					m_modelValidationError = Echo::StringUtil::Format("Input [%d] is invalid", i).c_str();

					return false;
				}
			}
		}

		return true;
	}

	QJsonObject ShaderNode::save() const
	{
		QJsonObject nodeJson = NodeDataModel::save();
		saveShaderNode(nodeJson);

		return nodeJson;
	}

	void ShaderNode::restore(QJsonObject const& p)
	{
		restoreShaderNode(p);
	}

	void ShaderNode::saveShaderNode(QJsonObject& p) const
	{
		const Object* obj = this;

		Echo::PropertyInfos properties;
		Echo::Class::getPropertys(getClassName(), (Object*)obj, properties, PropertyInfo::Static | PropertyInfo::Dynamic, true);

		for (Echo::PropertyInfo* prop : properties)
		{
			Echo::Variant variant;
			Echo::Class::getPropertyValue((Object*)obj, prop->m_name, variant);

			p[prop->m_name.c_str()] = variant.toString().c_str();
		}
	}

	void ShaderNode::restoreShaderNode(QJsonObject const& p)
	{
		Echo::Object* obj = this;

		Echo::PropertyInfos properties;
		Echo::Class::getPropertys(getClassName(), obj, properties, 3, true);

		for (Echo::PropertyInfo* prop : properties)
		{
			QJsonValue v = p[prop->m_name.c_str()];
			if (!v.isUndefined())
			{
				Echo::String value = v.toString().toStdString().c_str();

				Echo::Variant variant;
				variant.fromString(prop->m_type, value);
				Echo::Class::setPropertyValue(obj, prop->m_name, variant);
			}
		}
	}

	bool ShaderNode::onNodePressed()
	{
		EditorApi.showObjectProperty(this);
		return true;
	}

	String ShaderNode::getVariableName() const
	{
		if (m_variableName.empty())
		{
			Echo::String variableName = name().toStdString().c_str() + Echo::StringUtil::Format("_%d", m_id);
			variableName = Echo::StringUtil::Replace(variableName, " ", "");

			return variableName;
		}

		return m_variableName;
	}

	void ShaderNode::setVariableName(const String& variableName)
	{ 
		m_variableName = variableName; 

		Q_EMIT captionUpdated();
	}

	i32 ShaderNode::getOutputConnectionCount(QtNodes::PortIndex index)
	{
		i32 count = 0;
		for (const QtNodes::Connection* conn : m_outputConnections)
		{
			if (conn && conn->getPortIndex(QtNodes::PortType::Out) == index)
				count++;
		}


		return count;
	}

	void ShaderNode::outputConnectionCreated(QtNodes::Connection const& conn)
	{
		m_outputConnections.push_back(&conn);
	}

	void ShaderNode::outputConnectionDeleted(QtNodes::Connection const& conn)
	{
		m_outputConnections.erase(std::remove(m_outputConnections.begin(), m_outputConnections.end(), &conn), m_outputConnections.end());
	}
}

#endif
