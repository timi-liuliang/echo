#pragma once

#include "engine/core/base/object.h"
#include "engine/core/math/Math.h"
#include "timeline_header_ruler.h"

#ifdef ECHO_EDITOR_MODE

#include <QHeaderView>
#include "timeline_header_tool_bar.h"

namespace Echo
{
    class QTimelineHeader : public QHeaderView
    {
    public:
        QTimelineHeader(Qt::Orientation orientation,class TimelinePanel* timelinePanel);

        // Get tool bar
        QTimelineHeaderToolBar* getToolBar() { return m_toolBar; }

        // Get main panel
        class TimelinePanel* getTimelinePanel() { return m_timelinePanel; }

    protected:
        // Paint section
        virtual void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

        // Paint timeline ruler
        void paintSectionRuler(QPainter* painter, const QRect& rect) const;

        // Event
        virtual bool eventFilter(QObject* obj, QEvent* event) override;

    protected:
        class TimelinePanel*        m_timelinePanel = nullptr;
        QTimelineHeaderToolBar*     m_toolBar = nullptr;
        mutable TimelineHeaderRuler m_ruler;
    };
}

#endif