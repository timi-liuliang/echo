#pragma once

#include "shader_data.h"

#ifdef ECHO_EDITOR_MODE

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace Echo
{
    class DataMat4 : public ShaderData
    {
    public:
        DataMat4(QtNodes::NodeDataModel* dataModel, const Echo::String& displayText)
            : ShaderData(dataModel)
            , m_displayText(displayText)
        {}

        NodeDataType type() const override
        {
            return NodeDataType {"mat4", m_displayText.c_str()};
        }

    private:
        Echo::String    m_displayText;
    };
}

#endif
