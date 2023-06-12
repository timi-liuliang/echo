#pragma once

#include <QWidget>
#include <QList>
#include <QDateTime>
#include "engine/core/math/Math.h"

namespace Echo
{
    class TimelineRuler : public QWidget
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
        explicit TimelineRuler(QWidget* parent = nullptr);
        void addEvent(const Event& event);
        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

    protected:
        void paintEvent(QPaintEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;

    private:
        QList<Event> m_events;
        int m_hoveredEventIndex;
        int m_selectedEventIndex;
        int m_draggedEventIndex;
        QPoint m_dragStartPos;
    };
}
