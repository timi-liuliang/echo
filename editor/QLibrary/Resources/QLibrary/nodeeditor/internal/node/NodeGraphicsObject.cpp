#include "NodeGraphicsObject.hpp"
#include <iostream>
#include <cstdlib>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QGraphicsEffect>
#include "../connection/ConnectionGraphicsObject.hpp"
#include "../connection/ConnectionState.hpp"
#include "../scene/FlowScene.hpp"
#include "NodePainter.hpp"
#include "Node.hpp"
#include "NodeDataModel.hpp"
#include "../NodeConnectionInteraction.hpp"
#include "../style/StyleCollection.hpp"

using QtNodes::NodeGraphicsObject;
using QtNodes::Node;
using QtNodes::FlowScene;

namespace QtNodes
{
	NodeGraphicsObject::NodeGraphicsObject(FlowScene& scene, Node& node)
		: m_scene(scene)
		, m_node(node)
		, m_locked(false)
		, m_proxyWidget(nullptr)
	{
		m_scene.addItem(this);

		setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, true);
		setFlag(QGraphicsItem::ItemIsMovable, true);
		setFlag(QGraphicsItem::ItemIsFocusable, true);
		setFlag(QGraphicsItem::ItemIsSelectable, true);
		setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

		setCacheMode(QGraphicsItem::DeviceCoordinateCache);

		auto const& nodeStyle = node.nodeDataModel()->nodeStyle();

		{
			auto effect = new QGraphicsDropShadowEffect;
			effect->setOffset(4, 4);
			effect->setBlurRadius(20);
			effect->setColor(nodeStyle.ShadowColor);

			setGraphicsEffect(effect);
		}

		setOpacity(nodeStyle.Opacity);

		setAcceptHoverEvents(true);

		setZValue(0);

		embedQWidget();

		// connect to the move signals to emit the move signals in FlowScene
		auto onMoveSlot = [this]()
		{
			m_scene.nodeMoved(m_node, pos());
		};

		connect(this, &QGraphicsObject::xChanged, this, onMoveSlot);
		connect(this, &QGraphicsObject::yChanged, this, onMoveSlot);
	}

	NodeGraphicsObject::~NodeGraphicsObject()
	{
		m_scene.removeItem(this);
	}

	Node& NodeGraphicsObject::node()
	{
		return m_node;
	}

	Node const& NodeGraphicsObject::node() const
	{
		return m_node;
	}

	void NodeGraphicsObject::embedQWidget()
	{
		NodeGeometry& geom = m_node.nodeGeometry();

		if (auto w = m_node.nodeDataModel()->embeddedWidget())
		{
			m_proxyWidget = new QGraphicsProxyWidget(this);

			m_proxyWidget->setWidget(w);

			m_proxyWidget->setPreferredWidth(5);

			geom.recalculateSize();

			if (w->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag)
			{
				// If the widget wants to use as much vertical space as possible, set it to have the geom's equivalentWidgetHeight.
				m_proxyWidget->setMinimumHeight(geom.equivalentWidgetHeight());
			}

			m_proxyWidget->setPos(geom.widgetPosition());

			update();

			m_proxyWidget->setOpacity(1.0);
			m_proxyWidget->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
		}
	}

	QRectF NodeGraphicsObject::boundingRect() const
	{
		return m_node.nodeGeometry().boundingRect();
	}

	void NodeGraphicsObject::setGeometryChanged()
	{
		prepareGeometryChange();
	}

	void NodeGraphicsObject::updateProxyWidgetPos()
	{
		QWidget* w = m_node.nodeDataModel()->embeddedWidget();
		if (w && m_proxyWidget)
		{
			m_proxyWidget->setPos(m_node.nodeGeometry().widgetPosition());
		}
	}

	void NodeGraphicsObject::moveConnections() const
	{
		NodeState const& nodeState = m_node.nodeState();

		for (PortType portType : {PortType::In, PortType::Out})
		{
			auto const& connectionEntries = nodeState.getEntries(portType);
			for (auto const& connections : connectionEntries)
			{
				for (auto& con : connections)
					con.second->getConnectionGraphicsObject().move();
			}
		}
	}

	void NodeGraphicsObject::lock(bool locked)
	{
		m_locked = locked;

		setFlag(QGraphicsItem::ItemIsMovable, !locked);
		setFlag(QGraphicsItem::ItemIsFocusable, !locked);
		setFlag(QGraphicsItem::ItemIsSelectable, !locked);
	}

	void NodeGraphicsObject::paint(QPainter* painter, QStyleOptionGraphicsItem const* option, QWidget*)
	{
		painter->setClipRect(option->exposedRect);

		NodePainter::paint(painter, m_node, m_scene);
	}

	QVariant NodeGraphicsObject::itemChange(GraphicsItemChange change, const QVariant& value)
	{
		if (change == ItemPositionChange && scene())
		{
			moveConnections();
		}

		return QGraphicsItem::itemChange(change, value);
	}

	void NodeGraphicsObject::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		if (m_locked)
			return;

		// deselect all other items after this one is selected
		if (!isSelected() &&
			!(event->modifiers() & Qt::ControlModifier))
		{
			m_scene.clearSelection();
		}

		for (PortType portToCheck : {PortType::In, PortType::Out})
		{
			NodeGeometry const& nodeGeometry = m_node.nodeGeometry();

			// TODO do not pass sceneTransform
			int const portIndex = nodeGeometry.checkHitScenePoint(portToCheck,
				event->scenePos(),
				sceneTransform());

			if (portIndex != INVALID)
			{
				NodeState const& nodeState = m_node.nodeState();

				std::unordered_map<QUuid, Connection*> connections =
					nodeState.connections(portToCheck, portIndex);

				// start dragging existing connection
				if (!connections.empty() && portToCheck == PortType::In)
				{
					auto con = connections.begin()->second;

					NodeConnectionInteraction interaction(m_node, *con, m_scene);

					interaction.disconnect(portToCheck);
				}
				else // initialize new Connection
				{
					if (portToCheck == PortType::Out)
					{
						auto const outPolicy = m_node.nodeDataModel()->portOutConnectionPolicy(portIndex);
						if (!connections.empty() &&
							outPolicy == NodeDataModel::ConnectionPolicy::One)
						{
							m_scene.deleteConnection(*connections.begin()->second);
						}
					}

					// todo add to FlowScene
					auto connection = m_scene.createConnection(portToCheck,
						m_node,
						portIndex);

					m_node.nodeState().setConnection(portToCheck,
						portIndex,
						*connection);

					connection->getConnectionGraphicsObject().grabMouse();
				}
			}
		}

		auto pos = event->pos();
		auto& geom = m_node.nodeGeometry();
		auto& state = m_node.nodeState();

		if (m_node.nodeDataModel()->resizable() && geom.resizeRect().contains(QPoint(pos.x(), pos.y())))
		{
			state.setResizing(true);
		}

		m_scene.nodePressed(node());
	}

	void NodeGraphicsObject::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
	{
		auto& geom = m_node.nodeGeometry();
		auto& state = m_node.nodeState();

		if (state.resizing())
		{
			auto diff = event->pos() - event->lastPos();

			if (auto w = m_node.nodeDataModel()->embeddedWidget())
			{
				prepareGeometryChange();

				auto oldSize = w->size();

				oldSize += QSize(diff.x(), diff.y());

				w->setFixedSize(oldSize);

				m_proxyWidget->setMinimumSize(oldSize);
				m_proxyWidget->setMaximumSize(oldSize);
				m_proxyWidget->setPos(geom.widgetPosition());

				geom.recalculateSize();
				update();

				moveConnections();

				event->accept();
			}
		}
		else
		{
			QGraphicsObject::mouseMoveEvent(event);

			if (event->lastPos() != event->pos())
				moveConnections();

			event->ignore();
		}

		QRectF r = scene()->sceneRect();

		r = r.united(mapToScene(boundingRect()).boundingRect());

		scene()->setSceneRect(r);
	}

	void NodeGraphicsObject::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
	{
		auto& state = m_node.nodeState();

		state.setResizing(false);

		QGraphicsObject::mouseReleaseEvent(event);

		// position connections precisely after fast node move
		moveConnections();
	}

	void NodeGraphicsObject::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
	{
		// bring all the colliding nodes to background
		QList<QGraphicsItem*> overlapItems = collidingItems();

		for (QGraphicsItem* item : overlapItems)
		{
			if (item->zValue() > 0.0)
			{
				item->setZValue(0.0);
			}
		}

		// bring this node forward
		setZValue(1.0);

		m_node.nodeGeometry().setHovered(true);
		update();
		m_scene.nodeHovered(node(), event->screenPos());
		event->accept();
	}

	void NodeGraphicsObject::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
	{
		m_node.nodeGeometry().setHovered(false);
		update();
		m_scene.nodeHoverLeft(node());
		event->accept();
	}

	void NodeGraphicsObject::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
	{
		auto pos = event->pos();
		auto& geom = m_node.nodeGeometry();

		if (m_node.nodeDataModel()->resizable() &&
			geom.resizeRect().contains(QPoint(pos.x(), pos.y())))
		{
			setCursor(QCursor(Qt::SizeFDiagCursor));
		}
		else
		{
			setCursor(QCursor());
		}

		event->accept();
	}

	void NodeGraphicsObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsItem::mouseDoubleClickEvent(event);

		m_scene.nodeDoubleClicked(node());
	}

	void NodeGraphicsObject::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
	{
		m_scene.nodeContextMenu(node(), mapToScene(event->pos()));
	}
}
