#pragma once

#include <vector>
#include <unordered_map>
#include <QtCore/QUuid>
#include "../base/Export.hpp"
#include "PortType.hpp"
#include "NodeData.hpp"
#include "../base/memory.hpp"

namespace QtNodes
{
    class Connection;
    class NodeDataModel;

    /// Contains vectors of connected input and output connections.
    /// Stores bool for reacting on hovering connections
    class NODE_EDITOR_PUBLIC NodeState
    {
    public:
        using ConnectionPtrSet = std::unordered_map<QUuid, Connection*>;

        enum ReactToConnectionState
        {
            REACTING,
            NOT_REACTING
        };

    public:
        NodeState(std::unique_ptr<NodeDataModel> const &model);

        /// Returns vector of connections ID.
        /// Some of them can be empty (null)
        std::vector<ConnectionPtrSet>const& getEntries(PortType) const;
        std::vector<ConnectionPtrSet>& getEntries(PortType);

        // get connections
        ConnectionPtrSet connections(PortType portType, PortIndex portIndex) const;

        // set|erase connection
        void setConnection(PortType portType, PortIndex portIndex, Connection& connection);
        void eraseConnection(PortType portType, PortIndex portIndex, QUuid id);

        ReactToConnectionState reaction() const;

        PortType reactingPortType() const;
        NodeDataType reactingDataType() const;

        // reaction
        void setReaction(ReactToConnectionState reaction, PortType reactingPortType = PortType::None, NodeDataType reactingDataType = NodeDataType());
        bool isReacting() const;

        // resizing
        void setResizing(bool resizing);
        bool resizing() const;

    protected:
        // sync
        void syncConnections();

    private:
        std::vector<ConnectionPtrSet>   m_inConnections;
        std::vector<ConnectionPtrSet>   m_outConnections;
        ReactToConnectionState          m_reaction;
        PortType                        m_reactingPortType;
        NodeDataType                    m_reactingDataType;
        bool                            m_resizing;
    };
}
