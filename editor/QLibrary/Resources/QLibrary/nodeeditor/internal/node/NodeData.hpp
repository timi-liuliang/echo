#pragma once

#include <QtCore/QString>
#include "../base/Export.hpp"
#include "Engine/core/util/StringUtil.h"

namespace QtNodes
{
    struct NodeDataType
    {
        Echo::String id;
        Echo::String name;
    };

    /// Class represents data transferred between nodes.
    /// @param type is used for comparing the types
    /// The actual data is stored in subtypes
    class NODE_EDITOR_PUBLIC NodeData
    {
    public:
        virtual ~NodeData() = default;

        // is same type with
        virtual bool sameType(NodeData const &nodeData) const
        {
            return (this->type().id == nodeData.type().id);
        }

        /// Type for inner use
        virtual NodeDataType type() const = 0;
    };
}
