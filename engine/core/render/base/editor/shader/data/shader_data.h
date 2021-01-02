#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <nodeeditor/NodeData>
#include <nodeeditor/NodeDataModel>
#include "engine/core/util/StringUtil.h"

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace Echo
{
	class ShaderData : public NodeData
	{
	public:
		ShaderData(QtNodes::NodeDataModel* dataModel) : m_dataModel(dataModel) {}
		virtual ~ShaderData() {}

		// data model
		QtNodes::NodeDataModel* getDataModel() { return m_dataModel; }

		// name
		void setVariableName(const Echo::String& name) { m_variableName = name; }
		const Echo::String& getVariableName() const { return m_variableName; }

	protected:
		QtNodes::NodeDataModel*	m_dataModel = nullptr;
		Echo::String			m_variableName;
	};
}

#endif
