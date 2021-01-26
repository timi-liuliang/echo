#include "Connection.hpp"

#include <cmath>
#include <utility>

#include <QtWidgets/QtWidgets>
#include <QtGlobal>

#include "../node/Node.hpp"
#include "../scene/FlowScene.hpp"
#include "../scene/FlowView.hpp"

#include "../node/NodeGeometry.hpp"
#include "../node/NodeGraphicsObject.hpp"
#include "../node/NodeDataModel.hpp"

#include "ConnectionState.hpp"
#include "ConnectionGeometry.hpp"
#include "ConnectionGraphicsObject.hpp"

using QtNodes::Connection;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::ConnectionState;
using QtNodes::Node;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::ConnectionGraphicsObject;
using QtNodes::ConnectionGeometry;
using QtNodes::TypeConverter;

Connection::
Connection(PortType portType,
           Node& node,
           PortIndex portIndex)
  : _uid(QUuid::createUuid())
  , m_outPortIndex(INVALID)
  , m_inPortIndex(INVALID)
  , _connectionState()
{
  setNodeToPort(node, portType, portIndex);

  setRequiredPort(oppositePort(portType));
}


Connection::Connection(Node& nodeIn,PortIndex portIndexIn, Node& nodeOut, PortIndex portIndexOut, TypeConverter typeConverter)
  : _uid(QUuid::createUuid())
  , m_outNode(&nodeOut)
  , m_inNode(&nodeIn)
  , m_outPortIndex(portIndexOut)
  , m_inPortIndex(portIndexIn)
  , _connectionState()
  , _converter(std::move(typeConverter))
{
	setNodeToPort(nodeIn, PortType::In, portIndexIn);
	setNodeToPort(nodeOut, PortType::Out, portIndexOut);
}


Connection::
~Connection()
{
  if (complete()) connectionMadeIncomplete(*this);
  propagateEmptyData();

  if (m_inNode)
  {
    m_inNode->nodeGraphicsObject().update();
  }

  if (m_outNode)
  {
    m_outNode->nodeGraphicsObject().update();
  }
}


QJsonObject
Connection::
save() const
{
  QJsonObject connectionJson;

  if (m_inNode && m_outNode)
  {
    connectionJson["in_id"] = m_inNode->id().toString();
    connectionJson["in_index"] = m_inPortIndex;

    connectionJson["out_id"] = m_outNode->id().toString();
    connectionJson["out_index"] = m_outPortIndex;

    if (_converter)
    {
      auto getTypeJson = [this](PortType type)
      {
        QJsonObject typeJson;
        NodeDataType nodeType = this->dataType(type);
        typeJson["id"] = nodeType.id.c_str();
        typeJson["name"] = nodeType.name.c_str();

        return typeJson;
      };

      QJsonObject converterTypeJson;

      converterTypeJson["in"] = getTypeJson(PortType::In);
      converterTypeJson["out"] = getTypeJson(PortType::Out);

      connectionJson["converter"] = converterTypeJson;
    }
  }

  return connectionJson;
}


QUuid Connection::id() const
{
    return _uid;
}

bool Connection::complete() const
{
  return m_inNode != nullptr && m_outNode != nullptr;
}

void Connection::setRequiredPort(PortType dragging)
{
  _connectionState.setRequiredPort(dragging);

  switch (dragging)
  {
    case PortType::Out:
      m_outNode      = nullptr;
      m_outPortIndex = INVALID;
      break;

    case PortType::In:
      m_inNode      = nullptr;
      m_inPortIndex = INVALID;
      break;

    default:
      break;
  }
}


PortType
Connection::
requiredPort() const
{
  return _connectionState.requiredPort();
}


void
Connection::
setGraphicsObject(std::unique_ptr<ConnectionGraphicsObject>&& graphics)
{
  _connectionGraphicsObject = std::move(graphics);

  // This function is only called when the ConnectionGraphicsObject
  // is newly created. At this moment both end coordinates are (0, 0)
  // in Connection G.O. coordinates. The position of the whole
  // Connection G. O. in scene coordinate system is also (0, 0).
  // By moving the whole object to the Node Port position
  // we position both connection ends correctly.

  if (requiredPort() != PortType::None)
  {

    PortType attachedPort = oppositePort(requiredPort());

    PortIndex attachedPortIndex = getPortIndex(attachedPort);

    auto node = getNode(attachedPort);

    QTransform nodeSceneTransform =
      node->nodeGraphicsObject().sceneTransform();

    QPointF pos = node->nodeGeometry().portScenePosition(attachedPortIndex,
                                                         attachedPort,
                                                         nodeSceneTransform);

    _connectionGraphicsObject->setPos(pos);
  }

  _connectionGraphicsObject->move();
}



PortIndex
Connection::
getPortIndex(PortType portType) const
{
  PortIndex result = INVALID;

  switch (portType)
  {
    case PortType::In:
      result = m_inPortIndex;
      break;

    case PortType::Out:
      result = m_outPortIndex;

      break;

    default:
      break;
  }

  return result;
}


void Connection::setNodeToPort(Node& node, PortType portType, PortIndex portIndex)
{
	bool wasIncomplete = !complete();

	auto& nodeWeak = getNode(portType);

	nodeWeak = &node;

	if (portType == PortType::Out)
		m_outPortIndex = portIndex;
	else
		m_inPortIndex = portIndex;

	_connectionState.setNoRequiredPort();

	updated(*this);
	if (complete() && wasIncomplete) 
    {
		connectionCompleted(*this);
	}
}


void
Connection::
removeFromNodes() const
{
  if (m_inNode)
    m_inNode->nodeState().eraseConnection(PortType::In, m_inPortIndex, id());

  if (m_outNode)
    m_outNode->nodeState().eraseConnection(PortType::Out, m_outPortIndex, id());
}


ConnectionGraphicsObject&
Connection::
getConnectionGraphicsObject() const
{
  return *_connectionGraphicsObject;
}


ConnectionState&
Connection::
connectionState()
{
  return _connectionState;
}


ConnectionState const&
Connection::
connectionState() const
{
  return _connectionState;
}


ConnectionGeometry&
Connection::
connectionGeometry()
{
  return _connectionGeometry;
}


ConnectionGeometry const&
Connection::
connectionGeometry() const
{
  return _connectionGeometry;
}


Node*
Connection::
getNode(PortType portType) const
{
  switch (portType)
  {
    case PortType::In:
      return m_inNode;
      break;

    case PortType::Out:
      return m_outNode;
      break;

    default:
      // not possible
      break;
  }
  return nullptr;
}


Node*&
Connection::
getNode(PortType portType)
{
  switch (portType)
  {
    case PortType::In:
      return m_inNode;
      break;

    case PortType::Out:
      return m_outNode;
      break;

    default:
      // not possible
      break;
  }
  Q_UNREACHABLE();
}


void
Connection::
clearNode(PortType portType)
{
  if (complete()) {
    connectionMadeIncomplete(*this);
  }

  getNode(portType) = nullptr;

  if (portType == PortType::In)
    m_inPortIndex = INVALID;
  else
    m_outPortIndex = INVALID;
}


NodeDataType
Connection::
dataType(PortType portType) const
{
  if (m_inNode && m_outNode)
  {
    auto const & model = (portType == PortType::In) ?
                        m_inNode->nodeDataModel() :
                        m_outNode->nodeDataModel();
    PortIndex index = (portType == PortType::In) ? 
                      m_inPortIndex :
                      m_outPortIndex;

    return model->dataType(portType, index);
  }
  else 
  {
    Node* validNode;
    PortIndex index = INVALID;

    if ((validNode = m_inNode))
    {
      index    = m_inPortIndex;
      portType = PortType::In;
    }
    else if ((validNode = m_outNode))
    {
      index    = m_outPortIndex;
      portType = PortType::Out;
    }

    if (validNode)
    {
      auto const &model = validNode->nodeDataModel();

      return model->dataType(portType, index);
    }
  }

  Q_UNREACHABLE();
}


void
Connection::
setTypeConverter(TypeConverter converter)
{
  _converter = std::move(converter);
}


void
Connection::
propagateData(std::shared_ptr<NodeData> nodeData) const
{
  if (m_inNode)
  {
    if (_converter)
    {
      nodeData = _converter(nodeData);
    }

    m_inNode->propagateData(nodeData, m_inPortIndex);
  }
}


void
Connection::
propagateEmptyData() const
{
  std::shared_ptr<NodeData> emptyData;

  propagateData(emptyData);
}
