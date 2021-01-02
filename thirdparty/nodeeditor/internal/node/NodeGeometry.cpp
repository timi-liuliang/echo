#include "NodeGeometry.hpp"
#include <iostream>
#include <cmath>
#include "PortType.hpp"
#include "NodeState.hpp"
#include "NodeDataModel.hpp"
#include "Node.hpp"
#include "NodeGraphicsObject.hpp"
#include "../style/StyleCollection.hpp"

using QtNodes::NodeGeometry;
using QtNodes::NodeDataModel;
using QtNodes::PortIndex;
using QtNodes::PortType;
using QtNodes::Node;

namespace QtNodes
{
	NodeGeometry::NodeGeometry(std::unique_ptr<NodeDataModel> const& dataModel)
		: m_width(100)
		, m_height(150)
		, m_inputPortWidth(70)
		, m_outputPortWidth(70)
		, m_entryHeight(20)
		, m_spacing(20)
		, m_hovered(false)
		, m_draggingPos(-1000, -1000)
		, m_dataModel(dataModel)
		, m_fontMetrics(QFont())
		, m_boldFontMetrics(QFont())
	{
		QFont f; f.setBold(true);

		m_boldFontMetrics = QFontMetrics(f);
	}

	unsigned int NodeGeometry::nOutputs() const
	{
		return m_dataModel->nPorts(PortType::Out);
	}

	unsigned int NodeGeometry::nInputs() const
	{
		return m_dataModel->nPorts(PortType::In);
	}

	QRectF NodeGeometry::entryBoundingRect() const
	{
		double const addon = 0.0;
		return QRectF(0 - addon, 0 - addon, m_entryWidth + 2 * addon, m_entryHeight + 2 * addon);
	}

	QRectF NodeGeometry::boundingRect() const
	{
		auto const& nodeStyle = StyleCollection::nodeStyle();
		double addon = 4 * nodeStyle.ConnectionPointDiameter;

		return QRectF(0 - addon, 0 - addon, m_width + 2 * addon, m_height + 2 * addon);
	}

	void NodeGeometry::recalculateSize() const
	{
		m_entryHeight = m_fontMetrics.height();

		{
			unsigned int maxNumOfEntries = std::max(nInputs(), nOutputs());
			unsigned int step = m_entryHeight + m_spacing;
			m_height = step * maxNumOfEntries;
		}

		if (auto w = m_dataModel->embeddedWidget())
		{
			m_height = std::max(m_height, static_cast<unsigned>(w->height()));
		}

		m_height += captionHeight();

		m_inputPortWidth = portWidth(PortType::In);
		m_outputPortWidth = portWidth(PortType::Out);

		m_width = m_inputPortWidth + m_outputPortWidth + 2 * m_spacing;

		if (auto w = m_dataModel->embeddedWidget())
		{
			m_width += w->width();
		}

		m_width = std::max(m_width, captionWidth());

		if (m_dataModel->validationState() != NodeValidationState::Valid)
		{
			m_width = std::max(m_width, validationWidth());
			m_height += validationHeight() + m_spacing;
		}
	}

	void NodeGeometry::recalculateSize(QFont const& font) const
	{
		QFontMetrics fontMetrics(font);
		QFont boldFont = font;

		boldFont.setBold(true);

		QFontMetrics boldFontMetrics(boldFont);

		if (m_boldFontMetrics != boldFontMetrics)
		{
			m_fontMetrics = fontMetrics;
			m_boldFontMetrics = boldFontMetrics;

			recalculateSize();
		}
	}

	QPointF NodeGeometry::portScenePosition(PortIndex index, PortType portType, QTransform const& t) const
	{
		auto const& nodeStyle = StyleCollection::nodeStyle();

		unsigned int step = m_entryHeight + m_spacing;

		QPointF result;

		double totalHeight = 0.0;
		totalHeight += captionHeight();
		totalHeight += step * index;

		// TODO: why?
		totalHeight += step / 2.0;

		switch (portType)
		{
		case PortType::Out:
		{
			double x = m_width + nodeStyle.ConnectionPointDiameter;
			result = QPointF(x, totalHeight);
			break;
		}

		case PortType::In:
		{
			double x = 0.0 - nodeStyle.ConnectionPointDiameter;
			result = QPointF(x, totalHeight);
			break;
		}

		default:
			break;
		}

		return t.map(result);
	}

	PortIndex NodeGeometry::checkHitScenePoint(PortType portType, QPointF const scenePoint, QTransform const& sceneTransform) const
	{
		auto const& nodeStyle = StyleCollection::nodeStyle();

		PortIndex result = INVALID;

		if (portType == PortType::None)
			return result;

		double const tolerance = 2.0 * nodeStyle.ConnectionPointDiameter;

		unsigned int const nItems = m_dataModel->nPorts(portType);

		for (unsigned int i = 0; i < nItems; ++i)
		{
			auto pp = portScenePosition(i, portType, sceneTransform);

			QPointF p = pp - scenePoint;
			auto    distance = std::sqrt(QPointF::dotProduct(p, p));

			if (distance < tolerance)
			{
				result = PortIndex(i);
				break;
			}
		}

		return result;
	}

	QRect NodeGeometry::resizeRect() const
	{
		unsigned int rectSize = 7;

		return QRect(m_width - rectSize,
			m_height - rectSize,
			rectSize,
			rectSize);
	}

	QPointF NodeGeometry::widgetPosition() const
	{
		if (auto w = m_dataModel->embeddedWidget())
		{
			if (w->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag)
			{
				// If the widget wants to use as much vertical space as possible, place it immediately after the caption.
				return QPointF(m_spacing + portWidth(PortType::In), captionHeight());
			}
			else
			{
				if (m_dataModel->validationState() != NodeValidationState::Valid)
				{
					return QPointF(m_spacing + portWidth(PortType::In),
						(captionHeight() + m_height - validationHeight() - m_spacing - w->height()) / 2.0);
				}

				return QPointF(m_spacing + portWidth(PortType::In),
					(captionHeight() + m_height - w->height()) / 2.0);
			}
		}

		return QPointF();
	}

	int NodeGeometry::equivalentWidgetHeight() const
	{
		if (m_dataModel->validationState() != NodeValidationState::Valid)
		{
			return height() - captionHeight() + validationHeight();
		}

		return height() - captionHeight();
	}

	unsigned int NodeGeometry::captionHeight() const
	{
		if (!m_dataModel->captionVisible())
			return 0;

		QString name = m_dataModel->caption();

		return m_boldFontMetrics.boundingRect(name).height();
	}

	unsigned int NodeGeometry::captionWidth() const
	{
		if (!m_dataModel->captionVisible())
			return 0;

		QString name = m_dataModel->caption();

		return m_boldFontMetrics.boundingRect(name).width();
	}

	unsigned int NodeGeometry::validationHeight() const
	{
		QString msg = m_dataModel->validationMessage();

		return m_boldFontMetrics.boundingRect(msg).height();
	}

	unsigned int NodeGeometry::validationWidth() const
	{
		QString msg = m_dataModel->validationMessage();

		return m_boldFontMetrics.boundingRect(msg).width();
	}

	QPointF NodeGeometry::calculateNodePositionBetweenNodePorts(PortIndex targetPortIndex, PortType targetPort, Node* targetNode, PortIndex sourcePortIndex, PortType sourcePort, Node* sourceNode,Node& newNode)
	{
		//Calculating the nodes position in the scene. It'll be positioned half way between the two ports that it "connects". 
		//The first line calculates the halfway point between the ports (node position + port position on the node for both nodes averaged).
		//The second line offsets this coordinate with the size of the new node, so that the new nodes center falls on the originally
		//calculated coordinate, instead of it's upper left corner.
		auto converterNodePos = (sourceNode->nodeGraphicsObject().pos() + sourceNode->nodeGeometry().portScenePosition(sourcePortIndex, sourcePort) +
			targetNode->nodeGraphicsObject().pos() + targetNode->nodeGeometry().portScenePosition(targetPortIndex, targetPort)) / 2.0f;
		converterNodePos.setX(converterNodePos.x() - newNode.nodeGeometry().width() / 2.0f);
		converterNodePos.setY(converterNodePos.y() - newNode.nodeGeometry().height() / 2.0f);
		return converterNodePos;
	}

	unsigned int NodeGeometry::portWidth(PortType portType) const
	{
		unsigned width = 0;

		for (auto i = 0ul; i < m_dataModel->nPorts(portType); ++i)
		{
			QString name;

			if (m_dataModel->portCaptionVisible(portType, i))
				name = m_dataModel->portCaption(portType, i);
			else
				name = m_dataModel->dataType(portType, i).name.c_str();

			width = std::max(unsigned(m_fontMetrics.width(name)), width);
		}

		return width;
	}
}
