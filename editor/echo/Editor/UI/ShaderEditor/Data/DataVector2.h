#pragma once

#include "ShaderData.h"
#include <nodeeditor/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace DataFlowProgramming
{
    class DataVector2 : public ShaderData
    {
    public:
        DataVector2(ShaderDataModel* dataModel, const Echo::String& displayText)
            : ShaderData(dataModel)
            , m_displayText(displayText)
        {}

        NodeDataType type() const override
        {
            return NodeDataType {"vec2", m_displayText.c_str()};
        }

    private:
        Echo::String    m_displayText;
    };
}
