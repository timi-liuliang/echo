#pragma once

#include "engine/core/base/object.h"
#include "engine/core/math/Math.h"

#ifdef ECHO_EDITOR_MODE

#include <QHeaderView>

namespace Echo
{
    class QTimelineHeader : public QHeaderView
    {
    public:
        QTimelineHeader(Qt::Orientation orientation, QWidget* parent = nullptr);

        // Paint section
        virtual void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
    };
}

#endif