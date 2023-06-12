#include "timeline_ruler.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

namespace Echo
{
    TimelineRuler::TimelineRuler(QWidget* parent)
        : QWidget(parent)
        , m_hoveredEventIndex(-1)
        , m_selectedEventIndex(-1)
        , m_draggedEventIndex(-1)
    {
        setMouseTracking(true); // receive mouse move events even when no button is pressed

        for (int i = 0; i < 100; i++)
        {
            Event a("0", i);
            addEvent(a);
        }
    }

    void TimelineRuler::addEvent(const Event& event)
    {
        m_events.append(event);
        update(); // repaint the control to show the new event
    }

    QSize TimelineRuler::minimumSizeHint() const
    {
        return QSize(100, 46); // a reasonable minimum size for the control
    }

    QSize TimelineRuler::sizeHint() const
    {
        return QSize(400, 46); // a reasonable default size for the control
    }

    void TimelineRuler::paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing); // smooth out lines and curves

        const int margin = 10;
        const int height = this->height() - 2 * margin;

        // Draw the timeline axis
        painter.drawLine(margin, margin + height / 2, width() - margin, margin + height / 2);

        // Draw the events
        const int eventWidth = (width() - 2 * margin) / m_events.size();
        for (int i = 0; i < m_events.size(); ++i) {
            const Event& event = m_events[i];
            const int x1 = margin + i * eventWidth;
            const int x2 = x1 + eventWidth;
            const int y1 = margin;
            const int y2 = margin + height;
            const bool isHovered = (m_hoveredEventIndex == i);
            const bool isSelected = (m_selectedEventIndex == i);
            const bool isDragged = (m_draggedEventIndex == i);
            QPen pen;
            if (isDragged) {
                pen.setColor(Qt::red);
                pen.setWidthF(2.0);
            }
            else if (isSelected) {
                pen.setColor(Qt::blue);
                pen.setWidthF(2.0);
            }
            else if (isHovered) {
                pen.setColor(Qt::gray);
                pen.setWidthF(1.5);
            }
            else {
                pen.setColor(Qt::black);
                pen.setWidthF(1.0);
            }
            painter.setPen(pen);
            painter.drawRect(x1, y1, x2 - x1, y2 - y1);
            painter.drawText(QRectF(x1 + 5, y1 + 5, x2 - x1 - 10, y2 - y1 - 10), Qt::AlignTop | Qt::TextWordWrap, event.title());
        }
    }

    void TimelineRuler::mousePressEvent(QMouseEvent* event)
    {
        const int margin = 10;
        const int height = this->height() - 2 * margin;
        const int eventWidth = (width() - 2 * margin) / m_events.size();
        bool isHovered = false;
        bool isSelected = false;

        for (int i = 0; i < m_events.size(); ++i) 
        {
            const int x1 = margin + i * eventWidth;
            const int x2 = x1 + eventWidth;
            const int y1 = margin;
            const int y2 = margin + height;

            if (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier) 
            {
                if (isHovered)
                {
                    m_selectedEventIndex = i;
                    update();
                    m_draggedEventIndex = i;
                    m_dragStartPos = event->pos();
                    event->accept();
                    return;
                }
            }
            else if (event->button() == Qt::RightButton && isSelected) 
            {
                m_events.removeAt(i);
                m_selectedEventIndex = -1;
                update();
                event->accept();
                return;
            }
        }

        m_selectedEventIndex = -1;
        update();
        event->ignore();
    }

    void TimelineRuler::mouseMoveEvent(QMouseEvent* e)
    {
        if (m_draggedEventIndex >= 0) 
        {
            QPoint delta = e->pos() - m_dragStartPos;
            const int margin = 10;
            const int height = this->height() - 2 * margin;
            const int eventWidth = (width() - 2 * margin) / m_events.size();
            const Event& event = m_events[m_draggedEventIndex];
            const i32 frame = event.frame(); // move the start time by the mouse x-delta in minutes
            Event newEvent(event.title(), frame+1);
            m_events.replace(m_draggedEventIndex, newEvent);
            update();
            e->accept();
        }
        else 
        {
            const int margin = 10;
            const int height = this->height() - 2 * margin;
            const int eventWidth = (width() - 2 * margin) / m_events.size();

            bool foundHoveredEvent = false;
            for (int i = 0; i < m_events.size(); ++i) 
            {
                const int x1 = margin + i * eventWidth;
                const int x2 = x1 + eventWidth;
                const int y1 = margin;
                const int y2 = margin + height;
                const bool wasHovered = (m_hoveredEventIndex == i);
                const bool isHovered = (e->pos().x() >= x1 && e->pos().x() < x2 && e->pos().y() >= y1 && e->pos().y() < y2);
                if (isHovered) 
                {
                    m_hoveredEventIndex = i;
                    foundHoveredEvent = true;
                    update();
                }
                else if (wasHovered) 
                {
                    m_hoveredEventIndex = -1;
                    update();
                }
            }

            if (!foundHoveredEvent) 
            {
                m_hoveredEventIndex = -1;
                update();
            }

            e->ignore();
        }
    }

    void TimelineRuler::mouseReleaseEvent(QMouseEvent* event)
    {
        if (m_draggedEventIndex >= 0)
        {
            m_draggedEventIndex = -1;
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
}
