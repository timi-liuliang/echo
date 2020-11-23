#include "QGraphicsItem.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	QPixmap QGraphicsItemToPixmap(QGraphicsItem* item, float alpha)
	{
		QRectF boundingRect = item->sceneBoundingRect();
		QPixmap pixmap(boundingRect.size().toSize());
		pixmap.fill(Qt::transparent);
		if (item->scene())
		{
			QPainter painter(&pixmap);
			painter.setRenderHint(QPainter::Antialiasing);
			item->scene()->render(&painter, QRectF(), boundingRect);
			painter.end();
		}

		if (alpha < 1.f)
		{
			QPixmap alphaPixmap(boundingRect.size().toSize());
			QPainter painter(&alphaPixmap);
			painter.setOpacity(alpha);
			painter.drawPixmap(QPoint(), pixmap);
			painter.end();
			return alphaPixmap;
		}

		return pixmap;
	}
}

#endif
