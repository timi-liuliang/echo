#include "ShaderDataModel.h"

namespace DataFlowProgramming
{
	ShaderDataModel::ShaderDataModel()
		: NodeDataModel()
	{
		static Echo::ui32 id = 0;
		m_id = id++;

		m_name = caption().toStdString().c_str() + Echo::StringUtil::Format("_%d", m_id);
	}
}