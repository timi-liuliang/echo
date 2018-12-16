#include <QGraphicsItem>
#include <engine/core/util/StringUtil.h>

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

	void qGraphicsItemSetVisible(QGraphicsItem* item, bool visible)
	{
		if (item)
		{
			item->setVisible(visible);
		}
	}

	String qGraphicsItemUserData(QGraphicsItem* item)
	{
		return item ? item->data(Qt::UserRole).toString().toStdString().c_str() : StringUtil::BLANK;
	}

	void qGraphicsItemSetUserData(QGraphicsItem* item, const char* userData)
	{
		if (item)
		{
			item->setData(Qt::UserRole, userData);
		}
	}
}