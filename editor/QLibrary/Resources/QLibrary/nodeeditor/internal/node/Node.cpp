#include "Node.hpp"
#include <QtCore/QObject>
#include <utility>
#include <iostream>
#include "../scene/FlowScene.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeDataModel.hpp"
#include "../connection/ConnectionGraphicsObject.hpp"
#include "../connection/ConnectionState.hpp"

using QtNodes::Node;
using QtNodes::NodeGeometry;
using QtNodes::NodeState;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeGraphicsObject;
using QtNodes::PortIndex;
using QtNodes::PortType;

namespace QtNodes
{
	Node::Node(std::unique_ptr<NodeDataModel>&& dataModel)
		: m_uid(QUuid::createUuid())
		, m_nodeDataModel(std::move(dataModel))
		, m_nodeState(m_nodeDataModel)
		, m_nodeGeometry(m_nodeDataModel)
		, m_nodeGraphicsObject(nullptr)
	{
		m_nodeGeometry.recalculateSize();

		// propagate data: model => node
		QObject::connect(m_nodeDataModel.get(), &NodeDataModel::dataUpdated, this, &Node::onDataUpdated);
		QObject::connect(m_nodeDataModel.get(), &NodeDataModel::embeddedWidgetSizeUpdated, this, &Node::onNodeSizeUpdated);
	}

	Node::~Node() = default;

	QJsonObject Node::save() const
	{
		QJsonObject nodeJson;

		nodeJson["id"] = m_uid.toString();
		nodeJson["model"] = m_nodeDataModel->save();

		QJsonObject obj;
		obj["x"] = m_nodeGraphicsObject->pos().x();
		obj["y"] = m_nodeGraphicsObject->pos().y();
		nodeJson["position"] = obj;

		return nodeJson;
	}

	void Node::restore(QJsonObject const& json)
	{
		m_uid = QUuid(json["id"].toString());

		QJsonObject positionJson = json["position"].toObject();
		QPointF     point(positionJson["x"].toDouble(), positionJson["y"].toDouble());

		m_nodeGraphicsObject->setPos(point);
		m_nodeDataModel->restore(json["model"].toObject());
	}

	QUuid Node::id() const
	{
		return m_uid;
	}

	void Node::reactToPossibleConnection(PortType reactingPortType, NodeDataType const& reactingDataType, QPointF const& scenePoint)
	{
		QTransform const t = m_nodeGraphicsObject->sceneTransform();

		QPointF p = t.inverted().map(scenePoint);

		m_nodeGeometry.setDraggingPosition(p);

		m_nodeGraphicsObject->update();

		m_nodeState.setReaction(NodeState::REACTING, reactingPortType, reactingDataType);
	}

	void Node::resetReactionToConnection()
	{
		m_nodeState.setReaction(NodeState::NOT_REACTING);
		m_nodeGraphicsObject->update();
	}

	NodeGraphicsObject const& Node::nodeGraphicsObject() const
	{
		return *m_nodeGraphicsObject.get();
	}

	NodeGraphicsObject& Node::nodeGraphicsObject()
	{
		return *m_nodeGraphicsObject.get();
	}

	void Node::setGraphicsObject(std::unique_ptr<NodeGraphicsObject> && graphics)
	{
		m_nodeGraphicsObject = std::move(graphics);

		m_nodeGeometry.recalculateSize();
	}

	NodeGeometry& Node::nodeGeometry()
	{
		return m_nodeGeometry;
	}

	NodeGeometry const& Node::nodeGeometry() const
	{
		return m_nodeGeometry;
	}

	NodeState const& Node::nodeState() const
	{
		return m_nodeState;
	}

	NodeState& Node::nodeState()
	{
		return m_nodeState;
	}

	NodeDataModel* Node::nodeDataModel() const
	{
		return m_nodeDataModel.get();
	}

	void Node::propagateData(std::shared_ptr<NodeData> nodeData, PortIndex inPortIndex) const
	{
		m_nodeDataModel->setInData(std::move(nodeData), inPortIndex);

		//Recalculate the nodes visuals. A data change can result in the node taking more space than before, so this forces a recalculate+repaint on the affected node
		m_nodeGraphicsObject->setGeometryChanged();
		m_nodeGeometry.recalculateSize();
		m_nodeGraphicsObject->update();
		m_nodeGraphicsObject->moveConnections();
	}

	void Node::onDataUpdated(PortIndex index)
	{
		auto nodeData = m_nodeDataModel->outData(index);
		auto connections = m_nodeState.connections(PortType::Out, index);

		for (auto const& c : connections)
			c.second->propagateData(nodeData);
	}

	void Node::onNodeSizeUpdated()
	{
		if (nodeDataModel()->embeddedWidget())
		{
			nodeDataModel()->embeddedWidget()->adjustSize();
		}

		nodeGeometry().recalculateSize();
		for (PortType type : {PortType::In, PortType::Out})
		{
			for (auto& conn_set : nodeState().getEntries(type))
			{
				for (auto& pair : conn_set)
				{
					Connection* conn = pair.second;
					conn->getConnectionGraphicsObject().move();
				}
			}
		}
	}

}
