#pragma once

#include "ShaderData.h"
#include <nodeeditor/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace DataFlowProgramming
{
    /// The class can potentially encapsulate any user data which
    /// need to be transferred within the Node Editor graph
    class DataInvalid : public ShaderData
    {
    public:
        DataInvalid(ShaderDataModel* dataModel)
            : ShaderData(dataModel)
        {}


        NodeDataType type() const override
        {
            return NodeDataType {"invalid", "invalid"};
        }
    };
}
