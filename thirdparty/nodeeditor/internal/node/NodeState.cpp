#include "NodeState.hpp"
#include "NodeDataModel.hpp"
#include "../connection/Connection.hpp"

using QtNodes::NodeState;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::Connection;

namespace QtNodes
{
	NodeState::NodeState(std::unique_ptr<NodeDataModel> const& model)
		: m_reaction(NOT_REACTING)
		, m_reactingPortType(PortType::None)
		, m_resizing(false)
	{
		reset(model);
	}

	void NodeState::reset(std::unique_ptr<NodeDataModel> const& model)
	{
		m_inConnections.resize(model->nPorts(PortType::In));
		m_outConnections.resize(model->nPorts(PortType::Out));
	}

	std::vector<NodeState::ConnectionPtrSet> const& NodeState::getEntries(PortType portType) const
	{
		if (portType == PortType::In)
			return m_inConnections;
		else
			return m_outConnections;
	}

	std::vector<NodeState::ConnectionPtrSet>& NodeState::getEntries(PortType portType)
	{
		if (portType == PortType::In)
			return m_inConnections;
		else
			return m_outConnections;
	}

	NodeState::ConnectionPtrSet NodeState::connections(PortType portType, PortIndex portIndex) const
	{
		std::vector<NodeState::ConnectionPtrSet> const& connections = getEntries(portType);
		return connections[portIndex];
	}

	void NodeState::setConnection(PortType portType, PortIndex portIndex, Connection& connection)
	{
		std::vector<ConnectionPtrSet>& connections = getEntries(portType);
		if(connections.size()>portIndex)
			connections.at(portIndex).insert(std::make_pair(connection.id(), &connection));
	}

	void NodeState::eraseConnection(PortType portType, PortIndex portIndex, QUuid id)
	{
		std::vector<NodeState::ConnectionPtrSet>& entries = getEntries(portType);
		if (portIndex < entries.size())
		{
			entries[portIndex].erase(id);
		}
	}

	NodeState::ReactToConnectionState NodeState::reaction() const
	{
		return m_reaction;
	}

	PortType NodeState::reactingPortType() const
	{
		return m_reactingPortType;
	}

	NodeDataType NodeState::reactingDataType() const
	{
		return m_reactingDataType;
	}

	void NodeState::setReaction(ReactToConnectionState reaction, PortType reactingPortType, NodeDataType reactingDataType)
	{
		m_reaction = reaction;
		m_reactingPortType = reactingPortType;
		m_reactingDataType = std::move(reactingDataType);
	}

	bool NodeState::isReacting() const
	{
		return m_reaction == REACTING;
	}

	void NodeState::setResizing(bool resizing)
	{
		m_resizing = resizing;
	}

	bool NodeState::resizing() const
	{
		return m_resizing;
	}
}
