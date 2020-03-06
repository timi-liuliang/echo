#include "OperationRules.h"
#include "Data/DataFloat.h"
#include "Data/DataVector2.h"
#include "Data/DataVector3.h"
#include "Data/DataVector4.h"
#include "Data/DataInvalid.h"

namespace DataFlowProgramming
{
	OperationRules::OperationRules()
	{
		// addition
		m_additionRules.push_back(OperationRule(true, "float", "float", "float"));
		m_additionRules.push_back(OperationRule(true, "float", "vec2", "vec2"));
		m_additionRules.push_back(OperationRule(true, "float", "vec3", "vec3"));
		m_additionRules.push_back(OperationRule(true, "float", "vec4", "vec4"));
		m_additionRules.push_back(OperationRule(true, "float", "color", "vec4"));

		m_additionRules.push_back(OperationRule(true, "vec2", "vec2", "vec2"));
		m_additionRules.push_back(OperationRule(true, "vec3", "vec3", "vec3"));
		m_additionRules.push_back(OperationRule(true, "vec4", "vec4", "vec4"));
		m_additionRules.push_back(OperationRule(true, "vec4", "color", "vec4"));

		m_additionRules.push_back(OperationRule(true, "color", "color", "vec4"));

		// subtraction
		m_substractionRules = m_additionRules;

		// multiplication
		m_multiplicationRules = m_additionRules;

		// division
		m_divisionRules = m_additionRules;

		// dot product
		m_dotProductRules.push_back(OperationRule(true, "vec3", "vec3", "float"));

		// cross product
		m_crossProductRules.push_back(OperationRule(true, "vec3", "vec3", "vec3"));

		// min rules
		m_minRules = m_additionRules;

		// max rules
		m_maxRules = m_additionRules;

		// pow rules
		m_powRules.push_back(OperationRule(false, "float", "float", "float"));
		m_additionRules.push_back(OperationRule(false, "vec2", "float", "vec2"));
		m_additionRules.push_back(OperationRule(false, "vec3", "float", "vec3"));
		m_additionRules.push_back(OperationRule(false, "vec4", "float", "vec4"));
		m_additionRules.push_back(OperationRule(false, "color", "float", "vec4"));
	}

	OperationRules::~OperationRules()
	{

	}

	OperationRules& OperationRules::instance()
	{
		static OperationRules opertaionRules;
		return opertaionRules;
	}

	std::shared_ptr<ShaderData> OperationRules::NewAdditionOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel)
	{
		Echo::String outputType = getOutput(inputA, inputB, m_additionRules);

		return NewShaderData(outputType, dataModel);
	}

	std::shared_ptr<ShaderData> OperationRules::NewSubstractionOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel)
	{
		Echo::String outputType = getOutput(inputA, inputB, m_substractionRules);

		return NewShaderData(outputType, dataModel);
	}

	std::shared_ptr<ShaderData> OperationRules::NewMultiplicationOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel)
	{
		Echo::String outputType = getOutput(inputA, inputB, m_multiplicationRules);

		return NewShaderData(outputType, dataModel);
	}

	std::shared_ptr<ShaderData> OperationRules::NewDivisionOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel)
	{
		Echo::String outputType = getOutput(inputA, inputB, m_divisionRules);

		return NewShaderData(outputType, dataModel);
	}

	std::shared_ptr<ShaderData> OperationRules::NewDotProductOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel)
	{
		Echo::String outputType = getOutput(inputA, inputB, m_dotProductRules);

		return NewShaderData(outputType, dataModel);
	}

	std::shared_ptr<ShaderData> OperationRules::NewCrossProductOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel)
	{
		Echo::String outputType = getOutput(inputA, inputB, m_crossProductRules);

		return NewShaderData(outputType, dataModel);
	}

	std::shared_ptr<ShaderData> OperationRules::NewMinOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel)
	{
		Echo::String outputType = getOutput(inputA, inputB, m_minRules);

		return NewShaderData(outputType, dataModel);
	}

	std::shared_ptr<ShaderData> OperationRules::NewMaxOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel)
	{
		Echo::String outputType = getOutput(inputA, inputB, m_maxRules);

		return NewShaderData(outputType, dataModel);
	}

	std::shared_ptr<ShaderData> OperationRules::NewPowOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel)
	{
		Echo::String outputType = getOutput(inputA, inputB, m_powRules);

		return NewShaderData(outputType, dataModel);
	}

	Echo::String OperationRules::getOutput(const Echo::String& inputA, const Echo::String& inputB, const std::vector<OperationRule>& rules)
	{
		for (const OperationRule& rule : rules)
		{
			if (inputA == rule.m_inputA && inputB == rule.m_inputB)
			{
				return rule.m_output;
			}

			if (rule.m_isSupportCommutativeLaw)
			{
				if (inputA == rule.m_inputB && inputB == rule.m_inputA)
				{
					return rule.m_output;
				}
			}
		}

		return "invalid";
	}

	std::shared_ptr<ShaderData> OperationRules::NewShaderData(const Echo::String& type, ShaderDataModel* dataModel)
	{
		if (type == "float")	return std::make_shared<DataFloat>(dataModel, "float");
		else if (type == "vec2")	return std::make_shared<DataVector2>(dataModel, "vec2");
		else if (type == "vec3")	return std::make_shared<DataVector3>(dataModel, "vec3");
		else if (type == "vec4")	return std::make_shared<DataVector4>(dataModel, "vec4");
		else						return std::make_shared<DataInvalid>(dataModel);
	}
}