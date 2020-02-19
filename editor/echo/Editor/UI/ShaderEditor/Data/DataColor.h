#pragma once

#include "ShaderData.h"
#include <nodeeditor/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace DataFlowProgramming
{
    /// The class can potentially encapsulate any user data which
    /// need to be transferred within the Node Editor graph
    class DataColor : public ShaderData
    {
    public:
        DataColor(ShaderDataModel* dataModel, Echo::String& displayText)
			: ShaderData(dataModel)
			, m_displayText(displayText)
		{}

        NodeDataType type() const override
        {
            return NodeDataType {"color", m_displayText.c_str() };
        }

    private:
        Echo::String    m_displayText;
    };
}
