#pragma once

#include "ShaderData.h"
#include <nodeeditor/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace DataFlowProgramming
{
    /// The class can potentially encapsulate any user data which
    /// need to be transferred within the Node Editor graph
    class DataTexture : public ShaderData
    {
    public:
      DataTexture()
        : _number(0.0)
      {}

      DataTexture(double const number)
        : _number(number)
      {}

      NodeDataType type() const override
      {
        return NodeDataType {"texture", "Texture"};
      }

      double number() const { return _number; }

      QString numberAsText() const { return QString::number(_number, 'f'); }

    private:
        double _number;
    };
}
