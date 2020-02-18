#pragma once

#include <nodeeditor/NodeDataModel>
#include "Engine/core/util/StringUtil.h"

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace DataFlowProgramming
{
	class ShaderData : public NodeData
	{
	public:
		ShaderData() {}
		virtual ~ShaderData() {}

		// name
		void setVariableName(const Echo::String& name) { m_variableName = name; }
		const Echo::String& getVariableName() const { return m_variableName; }

	protected:
		Echo::String	m_variableName;
	};
}