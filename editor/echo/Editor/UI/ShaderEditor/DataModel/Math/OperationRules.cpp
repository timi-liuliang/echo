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
		m_substractionRules.push_back(OperationRule(true, "float", "float", "float"));
		m_substractionRules.push_back(OperationRule(true, "float", "vec2", "vec2"));
		m_substractionRules.push_back(OperationRule(true, "float", "vec3", "vec3"));
		m_substractionRules.push_back(OperationRule(true, "float", "vec4", "vec4"));
		m_substractionRules.push_back(OperationRule(true, "float", "color", "vec4"));

		m_substractionRules.push_back(OperationRule(true, "vec2", "vec2", "vec2"));
		m_substractionRules.push_back(OperationRule(true, "vec3", "vec3", "vec3"));
		m_substractionRules.push_back(OperationRule(true, "vec4", "vec4", "vec4"));
		m_substractionRules.push_back(OperationRule(true, "vec4", "color", "vec4"));

		m_substractionRules.push_back(OperationRule(true, "color", "color", "vec4"));

		// multiplication
		m_multiplicationRules.push_back(OperationRule(true, "float", "float", "float"));
		m_multiplicationRules.push_back(OperationRule(true, "float", "vec2", "vec2"));
		m_multiplicationRules.push_back(OperationRule(true, "float", "vec3", "vec3"));
		m_multiplicationRules.push_back(OperationRule(true, "float", "vec4", "vec4"));
		m_multiplicationRules.push_back(OperationRule(true, "float", "color", "vec4"));

		m_multiplicationRules.push_back(OperationRule(true, "vec2", "vec2", "vec2"));
		m_multiplicationRules.push_back(OperationRule(true, "vec3", "vec3", "vec3"));
		m_multiplicationRules.push_back(OperationRule(true, "vec4", "vec4", "vec4"));
		m_multiplicationRules.push_back(OperationRule(true, "vec4", "color", "vec4"));

		m_multiplicationRules.push_back(OperationRule(true, "color", "color", "vec4"));

		// division
		m_divisionRules.push_back(OperationRule(true, "float", "float", "float"));
		m_divisionRules.push_back(OperationRule(true, "float", "vec2", "vec2"));
		m_divisionRules.push_back(OperationRule(true, "float", "vec3", "vec3"));
		m_divisionRules.push_back(OperationRule(true, "float", "vec4", "vec4"));
		m_divisionRules.push_back(OperationRule(true, "float", "color", "vec4"));

		m_divisionRules.push_back(OperationRule(true, "vec2", "vec2", "vec2"));
		m_divisionRules.push_back(OperationRule(true, "vec3", "vec3", "vec3"));
		m_divisionRules.push_back(OperationRule(true, "vec4", "vec4", "vec4"));
		m_divisionRules.push_back(OperationRule(true, "vec4", "color", "vec4"));

		m_divisionRules.push_back(OperationRule(true, "color", "color", "vec4"));
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