#pragma once

#include "shader_data.h"
#include <nodeeditor/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace Echo
{
    class DataTexture : public ShaderData
    {
    public:
        DataTexture(QtNodes::NodeDataModel* dataModel)
        : ShaderData(dataModel)
        {}

        NodeDataType type() const override
        {
            return NodeDataType {"texture", "Texture"};
        }
    };
}
