#pragma once

#include "shader_data.h"
#include <nodeeditor/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace Echo
{
    /// The class can potentially encapsulate any user data which
    /// need to be transferred within the Node Editor graph
    class DataInvalid : public ShaderData
    {
    public:
        DataInvalid(QtNodes::NodeDataModel* dataModel)
            : ShaderData(dataModel)
        {}


        NodeDataType type() const override
        {
            return NodeDataType {"invalid", "invalid"};
        }
    };
}
