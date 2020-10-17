#pragma once

#include <engine/core/util/StringUtil.h>
#include "Data/ShaderData.h"

namespace DataFlowProgramming
{
	class OperationRules
	{
	public:
		// Operation Rule
		struct OperationRule2_1
		{
			bool          m_isSupportCommutativeLaw;
			Echo::String  m_inputA;
			Echo::String  m_inputB;
			Echo::String  m_output;

			OperationRule2_1(bool isSupportCommutativeLaw, const Echo::String& inputA, const Echo::String& inputB, const Echo::String& output)
				: m_isSupportCommutativeLaw(isSupportCommutativeLaw), m_inputA(inputA), m_inputB(inputB), m_output(output)
			{}
		};

		struct OperationRule3_1
		{
			Echo::String  m_inputA;
			Echo::String  m_inputB;
			Echo::String  m_inputC;
			Echo::String  m_output;

			OperationRule3_1(const Echo::String& inputA, const Echo::String& inputB, const Echo::String& inputC, const Echo::String& output)
				: m_inputA(inputA), m_inputB(inputB), m_inputC(inputC), m_output(output)
			{}
		};

	public:
		OperationRules();
		~OperationRules();

		// instance
		static OperationRules& instance();

	public:
		// new shaderData
		std::shared_ptr<ShaderData> NewShaderData(const Echo::String& type, ShaderDataModel* dataModel);

		// addition
		std::shared_ptr<ShaderData> NewAdditionOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewSubstractionOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewMultiplicationOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewDivisionOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewDotProductOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewCrossProductOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewMinOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewMaxOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);
		std::shared_ptr<ShaderData> NewPowOutput(const Echo::String& inputA, const Echo::String& inputB, ShaderDataModel* dataModel);

		// lerp
		std::shared_ptr<ShaderData> NewMixOutput(const Echo::String& inputA, const Echo::String& inputB, const Echo::String& inputC, ShaderDataModel* dataModel);

	private:
		// get output
		Echo::String getOutput(const Echo::String& inputA, const Echo::String& inputB, const std::vector<OperationRule2_1>& rules);
		Echo::String getOutput(const Echo::String& inputA, const Echo::String& inputB, const Echo::String& inputC, const std::vector<OperationRule3_1>& rules);

	private:
		std::vector<OperationRule2_1>	m_additionRules;
		std::vector<OperationRule2_1>	m_substractionRules;
		std::vector<OperationRule2_1>	m_multiplicationRules;
		std::vector<OperationRule2_1>	m_divisionRules;
		std::vector<OperationRule2_1>	m_dotProductRules;
		std::vector<OperationRule2_1>	m_crossProductRules;
		std::vector<OperationRule2_1>	m_minRules;
		std::vector<OperationRule2_1>	m_maxRules;
		std::vector<OperationRule2_1>	m_powRules;
		std::vector<OperationRule3_1>   m_mixRules;				// mix, smoothstep
	};
}