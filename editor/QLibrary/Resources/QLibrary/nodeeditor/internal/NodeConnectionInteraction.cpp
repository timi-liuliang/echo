#include "NodeConnectionInteraction.hpp"

#include "connection/ConnectionGraphicsObject.hpp"
#include "node/NodeGraphicsObject.hpp"
#include "node/NodeDataModel.hpp"
#include "DataModelRegistry.hpp"
#include "scene/FlowScene.hpp"

using QtNodes::NodeConnectionInteraction;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::FlowScene;
using QtNodes::Node;
using QtNodes::Connection;
using QtNodes::NodeDataModel;
using QtNodes::TypeConverter;

namespace QtNodes
{
	NodeConnectionInteraction::NodeConnectionInteraction(Node& node, Connection& connection, FlowScene& scene)
		: m_node(&node)
		, m_connection(&connection)
		, m_scene(&scene)
	{}

	bool NodeConnectionInteraction::canConnect(PortIndex& portIndex, TypeConverter& converter) const
	{
		// 1) Connection requires a port
		PortType requiredPort = connectionRequiredPort();
		if (requiredPort == PortType::None)
		{
			return false;
		}

		// 1.5) Forbid connecting the node to itself
		Node* node = m_connection->getNode(oppositePort(requiredPort));

		if (node == m_node)
			return false;

		// 2) connection point is on top of the node port
		QPointF connectionPoint = connectionEndScenePosition(requiredPort);

		portIndex = nodePortIndexUnderScenePoint(requiredPort, connectionPoint);
		if (portIndex == INVALID)
		{
			return false;
		}

		// 4) Connection type equals node port type, or there is a registered type conversion that can translate between the two

		auto connectionDataType =
			m_connection->dataType(oppositePort(requiredPort));

		auto const& modelTarget = m_node->nodeDataModel();
		NodeDataType candidateNodeDataType = modelTarget->dataType(requiredPort, portIndex);

		if (connectionDataType.id != candidateNodeDataType.id)
		{
			if (requiredPort == PortType::In)
			{
				converter = m_scene->registry().getTypeConverter(connectionDataType, candidateNodeDataType);
			}
			else if (requiredPort == PortType::Out)
			{
				converter = m_scene->registry().getTypeConverter(candidateNodeDataType, connectionDataType);
			}

			return (converter != nullptr);
		}

		return true;
	}

	bool NodeConnectionInteraction::tryConnect() const
	{
		// 1) Check conditions from 'canConnect'
		PortIndex portIndex = INVALID;

		TypeConverter converter;

		if (!canConnect(portIndex, converter))
		{
			return false;
		}

		// delete old connection
		PortType requiredPort = connectionRequiredPort();
		if (!nodePortIsEmpty(requiredPort, portIndex))
		{
			NodeState::ConnectionPtrSet currentConnections = m_node->nodeState().connections(requiredPort, portIndex);
			for (auto connect : currentConnections)
			{
				if (connect.second)
					m_node->nodeDataModel()->scene()->deleteConnection(*connect.second);
			}
		}

		// 1.5) If the connection is possible but a type conversion is needed,
		//      assign a converter to connection
		if (converter)
		{
			m_connection->setTypeConverter(converter);
		}

		// 2) Assign node to required port in Connection
		m_node->nodeState().setConnection(requiredPort, portIndex, *m_connection);

		// 3) Assign Connection to empty port in NodeState
		// The port is not longer required after this function
		m_connection->setNodeToPort(*m_node, requiredPort, portIndex);

		// 4) Adjust Connection geometry
		m_node->nodeGraphicsObject().moveConnections();

		// 5) Poke model to intiate data transfer
		auto outNode = m_connection->getNode(PortType::Out);
		if (outNode)
		{
			PortIndex outPortIndex = m_connection->getPortIndex(PortType::Out);
			outNode->onDataUpdated(outPortIndex);
		}

		return true;
	}

	/// 1) Node and Connection should be already connected
	/// 2) If so, clear Connection entry in the NodeState
	/// 3) Set Connection end to 'requiring a port'
	bool NodeConnectionInteraction::disconnect(PortType portToDisconnect) const
	{
		PortIndex portIndex =
			m_connection->getPortIndex(portToDisconnect);

		NodeState& state = m_node->nodeState();

		// clear pointer to Connection in the NodeState
		state.getEntries(portToDisconnect)[portIndex].clear();

		// 4) Propagate invalid data to IN node
		m_connection->propagateEmptyData();

		// clear Connection side
		m_connection->clearNode(portToDisconnect);

		m_connection->setRequiredPort(portToDisconnect);

		m_connection->getConnectionGraphicsObject().grabMouse();

		return true;
	}

	PortType NodeConnectionInteraction::connectionRequiredPort() const
	{
		auto const& state = m_connection->connectionState();

		return state.requiredPort();
	}

	QPointF NodeConnectionInteraction::connectionEndScenePosition(PortType portType) const
	{
		auto& go = m_connection->getConnectionGraphicsObject();

		ConnectionGeometry& geometry = m_connection->connectionGeometry();

		QPointF endPoint = geometry.getEndPoint(portType);

		return go.mapToScene(endPoint);
	}

	QPointF NodeConnectionInteraction::nodePortScenePosition(PortType portType, PortIndex portIndex) const
	{
		NodeGeometry const& geom = m_node->nodeGeometry();

		QPointF p = geom.portScenePosition(portIndex, portType);

		NodeGraphicsObject& ngo = m_node->nodeGraphicsObject();

		return ngo.sceneTransform().map(p);
	}

	PortIndex NodeConnectionInteraction::nodePortIndexUnderScenePoint(PortType portType, QPointF const& scenePoint) const
	{
		NodeGeometry const& nodeGeom = m_node->nodeGeometry();

		QTransform sceneTransform =
			m_node->nodeGraphicsObject().sceneTransform();

		PortIndex portIndex = nodeGeom.checkHitScenePoint(portType, scenePoint, sceneTransform);

		return portIndex;
	}

	bool NodeConnectionInteraction::nodePortIsEmpty(PortType portType, PortIndex portIndex) const
	{
		NodeState const& nodeState = m_node->nodeState();

		auto const& entries = nodeState.getEntries(portType);

		if (entries[portIndex].empty()) return true;

		const auto outPolicy = m_node->nodeDataModel()->portOutConnectionPolicy(portIndex);
		return (portType == PortType::Out && outPolicy == NodeDataModel::ConnectionPolicy::Many);
	}
}


