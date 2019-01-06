#include <QGraphicsItem>
#include <QGraphicsProxywidget>
#include <engine/core/util/StringUtil.h>
#include <engine/core/math/Vector2.h>

namespace Echo
{
	// current sender item
	static QGraphicsItem* g_senderItem = nullptr;

	void qSetSenderItem(QGraphicsItem* item)
	{
		g_senderItem = item;
	}

	QGraphicsItem* qSenderItem()
	{
		return g_senderItem;
	}

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

	Vector2 qGraphicsItemPos(QGraphicsItem* item)
	{
		return item ? Vector2(item->pos().x(), item->pos().y()) : Vector2::INVALID;
	}
	
	void qGraphicsProxyWidgetSetPos(QGraphicsProxyWidget* item, float posX, float posY)
	{
		if (item)
		{
			item->setPos(QPointF(posX, posY));
		}
	}

	void qGraphicsProxyWidgetSetZValue(QGraphicsProxyWidget* item, float zValue)
	{
		if (item)
		{
			item->setZValue(zValue);
		}
	}

	void qGraphicsItemSetZValue(QGraphicsItem* item, float zValue)
	{
		if (item)
		{
			item->setZValue(zValue);
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

	void qGraphicsItemSetToolTip(QGraphicsItem* item, const char* toolTip)
	{
		if (item)
		{
			item->setToolTip(toolTip);
		}
	}

	void qGraphicsItemSetMoveable(QGraphicsItem* item, bool isMoveable)
	{
		if (item)
		{
			item->setFlag(QGraphicsItem::ItemIsMovable, isMoveable);
		}
	}
}