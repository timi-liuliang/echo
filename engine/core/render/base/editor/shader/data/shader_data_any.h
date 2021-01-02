#pragma once

#include "shader_data.h"

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace Echo
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
        static std::shared_ptr<ShaderData> getInternalData(std::shared_ptr<ShaderData> input)
        {
            auto anyData = std::dynamic_pointer_cast<DataAny>(input);
            if (anyData)
                return anyData->m_internalData;

            return nullptr;
        }

    private:
        std::shared_ptr<ShaderData> m_internalData;
    };
}
