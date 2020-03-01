#pragma once

#include <engine/core/util/StringUtil.h>
#include "Data/ShaderData.h"

namespace DataFlowProgramming
{
	class OperationRules
	{
	public:
		// Operation Rule
		struct OperationRule
		{
			bool          m_isSupportCommutativeLaw;
			Echo::String  m_inputA;
			Echo::String  m_inputB;
			Echo::String  m_output;

			OperationRule(bool isSupportCommutativeLaw, const Echo::String& inputA, const Echo::String& inputB, const Echo::String& output)
				: m_isSupportCommutativeLaw(isSupportCommutativeLaw), m_inputA(inputA), m_inputB(inputB), m_output(output)
			{}
		};

	public:
		OperationRules();
		~OperationRules();

		// instance
		static OperationRules& instance();

	public:
		// addition
		std::shared_ptr<ShaderData> NewAdditionOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewSubstractionOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewMultiplicationOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewDivisionOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewDotProductOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewCrossProductOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);

	private:
		// get output
		Echo::String getOutput(const Echo::String& inputA, const Echo::String& inputB, const std::vector<OperationRule>& rules);

		// new shaderData
		std::shared_ptr<ShaderData> NewShaderData(const Echo::String& type, ShaderDataModel* dataModel);

	private:
		std::vector<OperationRule>	m_additionRules;
		std::vector<OperationRule>	m_substractionRules;
		std::vector<OperationRule>	m_multiplicationRules;
		std::vector<OperationRule>	m_divisionRules;
		std::vector<OperationRule>	m_dotProductRules;
		std::vector<OperationRule>	m_crossProductRules;
	};
}