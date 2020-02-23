#pragma once

#include "ShaderData.h"
#include <nodeeditor/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace DataFlowProgramming
{
    class DataVector4 : public ShaderData
    {
    public:
        DataVector4(ShaderDataModel* dataModel, const Echo::String& displayText)
            : ShaderData(dataModel)
            , m_displayText(displayText)
        {}

        NodeDataType type() const override
        {
            return NodeDataType {"vec4", m_displayText.c_str()};
        }

    private:
        Echo::String    m_displayText;
    };
}
