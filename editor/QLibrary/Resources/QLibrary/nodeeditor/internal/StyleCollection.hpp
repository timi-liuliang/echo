#pragma once

#include "NodeStyle.hpp"
#include "ConnectionStyle.hpp"
#include "FlowViewStyle.hpp"

namespace QtNodes
{
    class StyleCollection
    {
    public:
        // node style
        static NodeStyle const& nodeStyle();
        static void setNodeStyle(NodeStyle);

        // connection style
        static ConnectionStyle const& connectionStyle();
        static void setConnectionStyle(ConnectionStyle);

        // flow view style
        static FlowViewStyle const& flowViewStyle();
        static void setFlowViewStyle(FlowViewStyle);

    private:
        StyleCollection() = default;
        StyleCollection(StyleCollection const&) = delete;
        
        // instance
        static StyleCollection& instance();

        // operate "="
        StyleCollection& operator=(StyleCollection const&) = delete;

    private:
        NodeStyle       _nodeStyle;
        ConnectionStyle _connectionStyle;
        FlowViewStyle   _flowViewStyle;
    };
}
