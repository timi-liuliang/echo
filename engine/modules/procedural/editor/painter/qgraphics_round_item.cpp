#include "qgraphics_round_item.h"

namespace Procedural
{
#ifdef ECHO_EDITOR_MODE
	QVariant QGraphicsRoundRectItem::itemChange(GraphicsItemChange change, const QVariant& value)
	{
		if (change == ItemPositionChange && m_pgNode)
		{
			m_pgNode->setPosition(Echo::Vector2(pos().x(), pos().y()));
		}

		return QGraphicsRectItem::itemChange(change, value);
	}
#endif
}
