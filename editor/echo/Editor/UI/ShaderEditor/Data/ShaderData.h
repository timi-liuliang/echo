#pragma once

#include <nodeeditor/NodeData>
#include "Engine/core/util/StringUtil.h"

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace DataFlowProgramming
{
	class ShaderDataModel;
	class ShaderData : public NodeData
	{
	public:
		ShaderData(ShaderDataModel* dataModel) : m_dataModel(dataModel) {}
		virtual ~ShaderData() {}

		// data model
		ShaderDataModel* getDataModel() { return m_dataModel; }

		// name
		void setVariableName(const Echo::String& name) { m_variableName = name; }
		const Echo::String& getVariableName() const { return m_variableName; }

	protected:
		ShaderDataModel*	m_dataModel = nullptr;
		Echo::String		m_variableName;
	};
}