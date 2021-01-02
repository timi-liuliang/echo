#pragma once

#include <QtCore/QRectF>
#include <QtCore/QPointF>
#include <QtGui/QTransform>
#include <QtGui/QFontMetrics>

#include "PortType.hpp"
#include "../base/Export.hpp"
#include "../base/memory.hpp"

namespace QtNodes
{
    class NodeState;
    class NodeDataModel;
    class Node;

    class NODE_EDITOR_PUBLIC NodeGeometry
    {
        typedef std::unique_ptr<NodeDataModel> NodeDataModelPtr;

    public:
        NodeGeometry(std::unique_ptr<NodeDataModel> const &dataModel);

        unsigned int height() const { return m_height; }
        void setHeight(unsigned int h) { m_height = h; }

        unsigned int width() const { return m_width; }
        void setWidth(unsigned int w) { m_width = w; }

        unsigned int entryHeight() const { return m_entryHeight; }
        void setEntryHeight(unsigned int h) { m_entryHeight = h; }

        unsigned int entryWidth() const { return m_entryWidth; }
        void setEntryWidth(unsigned int w) { m_entryWidth = w; }

        unsigned int spacing() const { return m_spacing; }
        void setSpacing(unsigned int s) { m_spacing = s; }

		bool hovered() const { return m_hovered; }
		void setHovered(unsigned int h) { m_hovered = h; }

		unsigned int nOutputs() const;
		unsigned int nInputs() const;

        QPointF const& draggingPos() const { return m_draggingPos; }
        void setDraggingPosition(QPointF const& pos) { m_draggingPos = pos; }

    public:
        QRectF entryBoundingRect() const;
        QRectF boundingRect() const;

        /// Updates size unconditionally
        void recalculateSize() const;

        /// Updates size if the QFontMetrics is changed
        void recalculateSize(QFont const &font) const;

        // TODO removed default QTransform()
        QPointF portScenePosition(PortIndex index, PortType portType, QTransform const & t = QTransform()) const;
        
        // hit scene point
        PortIndex checkHitScenePoint(PortType portType, QPointF point, QTransform const & t = QTransform()) const;

        QRect resizeRect() const;

        /// Returns the position of a widget on the Node surface
        QPointF widgetPosition() const;

        /// Returns the maximum height a widget can be without causing the node to grow.
        int equivalentWidgetHeight() const;

        unsigned int validationHeight() const;
        unsigned int validationWidth() const;
  
        static QPointF calculateNodePositionBetweenNodePorts(PortIndex targetPortIndex, PortType targetPort, Node* targetNode, PortIndex sourcePortIndex, PortType sourcePort, Node* sourceNode,Node& newNode);
    
    private:
        // caption
        unsigned int captionHeight() const;
        unsigned int captionWidth() const;

        unsigned int portWidth(PortType portType) const;

    private:
	    mutable unsigned int    m_width;
	    mutable unsigned int    m_height;
	    unsigned int            m_entryWidth;
	    mutable unsigned int    m_inputPortWidth;
	    mutable unsigned int    m_outputPortWidth;
	    mutable unsigned int    m_entryHeight;
	    unsigned int            m_spacing;
        bool                    m_hovered;
        QPointF                 m_draggingPos;
        NodeDataModelPtr const& m_dataModel;
        mutable QFontMetrics    m_fontMetrics;
        mutable QFontMetrics    m_boldFontMetrics;
    };
}
