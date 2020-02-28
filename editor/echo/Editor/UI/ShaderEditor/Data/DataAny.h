#pragma once

#include "ShaderData.h"

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace DataFlowProgramming
{
    /// The class can potentially encapsulate any user data which
    /// need to be transferred within the Node Editor graph
    class DataAny : public ShaderData
    {
    public:
        DataAny(std::shared_ptr<ShaderData> origin) : ShaderData(nullptr), m_internalData(origin) {}

        // type
        QtNodes::NodeDataType type() const override { return NodeDataType {"any", "Any"}; }

        // origin
        std::shared_ptr<ShaderData> getInternalData() { return m_internalData; }

    private:
        std::shared_ptr<ShaderData> m_internalData;
    };
}
