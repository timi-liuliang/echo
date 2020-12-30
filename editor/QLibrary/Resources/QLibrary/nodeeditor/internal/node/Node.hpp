#pragma once


#include <QtCore/QObject>
#include <QtCore/QUuid>

#include <QtCore/QJsonObject>

#include "PortType.hpp"

#include "../base/Export.hpp"
#include "NodeState.hpp"
#include "NodeGeometry.hpp"
#include "NodeData.hpp"
#include "NodeGraphicsObject.hpp"
#include "../connection/ConnectionGraphicsObject.hpp"
#include "../base/Serializable.hpp"
#include "../base/memory.hpp"

namespace QtNodes
{
    class Connection;
    class ConnectionState;
    class NodeGraphicsObject;
    class NodeDataModel;
    class NODE_EDITOR_PUBLIC Node : public QObject, public Serializable
    {
      Q_OBJECT

    public:
        /// NodeDataModel should be an rvalue and is moved into the Node
        Node(std::unique_ptr<NodeDataModel> && dataModel);
        virtual ~Node();

    public:
        // load|save
        QJsonObject save() const override;
        void restore(QJsonObject const &json) override;

    public:
        // id
        QUuid id() const;

        // connection
        void reactToPossibleConnection(PortType, NodeDataType const &, QPointF const & scenePoint);
        void resetReactionToConnection();

    public:
        // node graphics object
        NodeGraphicsObject const & nodeGraphicsObject() const;
        NodeGraphicsObject & nodeGraphicsObject();

        // graphics object
        void setGraphicsObject(std::unique_ptr<NodeGraphicsObject>&& graphics);

        // node geometry
        NodeGeometry& nodeGeometry();
        NodeGeometry const& nodeGeometry() const;

        // node state
        NodeState const &nodeState() const;
        NodeState & nodeState();

        // node data model
        NodeDataModel* nodeDataModel() const;

    public Q_SLOTS: // data propagation
        /// Propagates incoming data to the underlying model.
        void propagateData(std::shared_ptr<NodeData> nodeData,PortIndex inPortIndex) const;

        /// Fetches data from model's OUT #index port
        /// and propagates it to the connection
        void onDataUpdated(PortIndex index);

        /// update the graphic part if the size of the embedded widget changes
        void onNodeSizeUpdated();

    private:
        QUuid                               m_uid;                  // addressing
        std::unique_ptr<NodeDataModel>      m_nodeDataModel;        // data
        NodeState                           m_nodeState;            // node state
        NodeGeometry                        m_nodeGeometry;         // painting
        std::unique_ptr<NodeGraphicsObject> m_nodeGraphicsObject;
    };
}
