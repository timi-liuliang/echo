#pragma once

#include <QWidget>
#include <QList>
#include <QDateTime>
#include "engine/core/math/Math.h"

namespace Echo
{
    class TimelineHeaderRuler
    {
        //Q_OBJECT

    public:
        class Event
        {
        public:
            Event(const QString& title, const i32 frame) : m_title(title), m_frame(frame){}

            const QString& title() const { return m_title; }
            const i32 frame() const { return m_frame; }

        private:
            QString m_title;
            i32     m_frame;
        };

    public:
        explicit TimelineHeaderRuler(QWidget* parent = nullptr);
        void addEvent(const Event& event);

        // Draw
        void draw(QPainter& painter, const QRect& rect);

    private:
        QColor m_normalColor = QColor(200,200, 200);
        QList<Event> m_events;
        int m_hoveredEventIndex;
        int m_selectedEventIndex;
        int m_draggedEventIndex;
        QPoint m_dragStartPos;
    };
}
