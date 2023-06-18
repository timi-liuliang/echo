#include "timeline_header_ruler.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

namespace Echo
{
    TimelineHeaderRuler::TimelineHeaderRuler(QWidget* parent)
        : m_hoveredEventIndex(-1)
        , m_selectedEventIndex(-1)
        , m_draggedEventIndex(-1)
    {
        for (int i = 0; i < 200; i++)
        {
            Event a(Echo::StringUtil::Format("%d", i*20).c_str(), i);
            addEvent(a);
        }
    }

    void TimelineHeaderRuler::addEvent(const Event& event)
    {
        m_events.append(event);
    }

    void TimelineHeaderRuler::draw(QPainter& painter, const QRect& rect)
    {
        // Draw the timeline axis
        QPen pen;
        pen.setColor(m_normalColor);
        pen.setWidthF(1.0);

        painter.setPen(pen);
        painter.drawLine(rect.left(), rect.height() - 3, rect.right(), rect.height() - 3);

        // Draw the events
        const int eventWidth = 20;
        const int eventWidthHalf = eventWidth * 0.5;
        for (int i = 0; i < m_events.size(); ++i) 
        {
            const Event& event = m_events[i];
            const bool isHovered = (m_hoveredEventIndex == i);
            const bool isSelected = (m_selectedEventIndex == i);
            const bool isDragged = (m_draggedEventIndex == i);

            if (isDragged) 
            {
                pen.setColor(Qt::red);
                pen.setWidthF(2.0);
            }
            else if (isSelected) 
            {
                pen.setColor(Qt::blue);
                pen.setWidthF(2.0);
            }
            else if (isHovered) 
            {
                pen.setColor(Qt::gray);
                pen.setWidthF(1.5);
            }
            else 
            {
                pen.setColor(m_normalColor);
                pen.setWidthF(1.0);
            }

            i32 centerX = rect.left() + (i + 0.5) * eventWidth;

            painter.setPen(pen);
            painter.drawLine(centerX, rect.height()-4, centerX, rect.height() - 12);

            if (i % 2 == 0)
            {
                const int textHeight = 18;
                const int textWidth = 30;
                const int textWidthHalf = textWidth * 0.5;

                pen.setColor(QColor(0, 0, 0));
                pen.setWidthF(1.0);
                painter.setPen(pen);

                QFont font = painter.font();
                font.setBold(false);
                painter.setFont(font);

                painter.drawText(centerX - textWidthHalf + 1, rect.height() - 10 - textHeight, textWidth, textHeight, Qt::AlignCenter | Qt::TextWordWrap, event.title());
            }
        }
    }
}
