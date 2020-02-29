#include "ShaderDataModel.h"
#include "Data/DataAny.h"

namespace DataFlowProgramming
{
	ShaderDataModel::ShaderDataModel()
		: NodeDataModel()
	{
		static Echo::ui32 id = 0;
		m_id = id++;
	}

	const Echo::String ShaderDataModel::getVariableName()
	{ 
		m_variableName = caption().toStdString().c_str() + Echo::StringUtil::Format("_%d", m_id);
		m_variableName = Echo::StringUtil::Replace(m_variableName, " ", "");

		return m_variableName; 
	}

	bool ShaderDataModel::checkValidation()
	{
		m_modelValidationState = NodeValidationState::Valid;
		m_modelValidationError = QStringLiteral("");

		// check 
		if (m_inputDataTypes.size() != m_inputs.size())
		{
			m_modelValidationState = NodeValidationState::Error;
			m_modelValidationError = QStringLiteral("Inputs count error");

			return false;
		}

		// input type check
		for (size_t i = 0; i < m_inputDataTypes.size(); i++)
		{
			if (m_inputs[i] && m_inputs[i]->type().id != m_inputDataTypes[i].id)
			{
				m_modelValidationState = NodeValidationState::Error;
				m_modelValidationError = Echo::StringUtil::Format("Input [%d] type error", i).c_str();

				return false;
			}
		}

		// check invalid input
		for (size_t i = 0; i < m_inputs.size(); i++)
		{
			if (m_inputs[i] && m_inputs[i]->type().id == "invalid")
			{
				m_modelValidationState = NodeValidationState::Error;
				m_modelValidationError = Echo::StringUtil::Format("Input [%d] is invalid", i).c_str();

				return false;
			}
		}

		// check invalid output
		for (size_t i = 0; i < m_inputs.size(); i++)
		{
			if (m_inputs[i] && m_inputs[i]->type().id == "invalid")
			{
				m_modelValidationState = NodeValidationState::Error;
				m_modelValidationError = Echo::StringUtil::Format("Input [%d] is invalid", i).c_str();

				return false;
			}
			else if (m_inputs[i] && m_inputs[i]->type().id == "any")
			{
				std::shared_ptr<ShaderData> internalData = DataAny::getInternalData(m_inputs[i]);
				if (internalData && internalData->type().id == "invalid")
				{
					m_modelValidationState = NodeValidationState::Error;
					m_modelValidationError = Echo::StringUtil::Format("Input [%d] is invalid", i).c_str();

					return false;
				}
			}
		}

		return true;
	}
}