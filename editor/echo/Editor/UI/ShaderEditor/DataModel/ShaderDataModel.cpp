#include "ShaderDataModel.h"

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

		return m_variableName; 
	}
}