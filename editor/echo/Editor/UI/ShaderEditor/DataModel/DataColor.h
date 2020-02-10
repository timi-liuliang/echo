#pragma once

#include <nodeeditor/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace DataFlowProgramming
{
    /// The class can potentially incapsulate any user data which
    /// need to be transferred within the Node Editor graph
    class DataColor : public NodeData
    {
    public:
        DataColor()
            : _number(0.0)
        {}

        DataColor(double const number)
            : _number(number)
        {}

        NodeDataType type() const override
        {
            return NodeDataType {"color", "Color"};
        }

        double number() const { return _number; }

        QString numberAsText() const { return QString::number(_number, 'f'); }

    private:
        double _number;
    };
}
