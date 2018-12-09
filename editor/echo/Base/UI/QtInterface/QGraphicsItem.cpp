#include <QGraphicsItem>

namespace Echo
{
	float qGraphicsItemWidth(QGraphicsItem* item)
	{
		return item ? item->sceneBoundingRect().width() : 0.f;
	}

	// set pos
	void qGraphicsItemSetPos(QGraphicsItem* item, float posX, float posY)
	{
		if (item)
		{
			item->setPos(QPointF(posX, posY));
		}
	}
}