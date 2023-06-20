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
        QTimelineHeader(Qt::Orientation orientation, QWidget* parent = nullptr);

        // Paint section
        virtual void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

    protected:
        // Paint timeline ruler
        void paintSectionRuler(QPainter* painter, const QRect& rect) const;

        // Event
        virtual bool eventFilter(QObject* obj, QEvent* event) override;

    protected:
        QTimelineHeaderToolBar*     m_toolBar = nullptr;
        mutable TimelineHeaderRuler m_ruler;
    };
}

#endif