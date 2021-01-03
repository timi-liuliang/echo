#pragma once

#include "shader_data.h"

#ifdef ECHO_EDITOR_MODE

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace Echo
{
    /// The class can potentially encapsulate any user data which
    /// need to be transferred within the Node Editor graph
    class DataFloat : public ShaderData
    {
    public:
        DataFloat(QtNodes::NodeDataModel* dataModel, const Echo::String& displayText)
            : ShaderData(dataModel)
            , m_displayText(displayText)
        {}


        NodeDataType type() const override
        {
            return NodeDataType {"float", m_displayText.c_str()};
        }

    private:
        Echo::String    m_displayText;
    };
}

#endif
