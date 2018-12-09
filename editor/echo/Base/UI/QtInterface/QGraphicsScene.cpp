#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <engine/core/math/color.h>

namespace Echo
{
	QObject* qGraphicsSceneNew()
	{
		return new QGraphicsScene();
	}

	QGraphicsLineItem* qGraphicsSceneAddLine(QObject* scene, float startX, float startY, float endX, float endY, const Color& color)
	{
		QGraphicsScene* graphicsScene = qobject_cast<QGraphicsScene*>(scene);
		if (graphicsScene)
		{
			QPen pen(QColor::fromRgbF(color.r, color.g, color.b, color.a));
			return graphicsScene->addLine( startX, startY, endX, endY, pen);
		}

		return nullptr;
	}

	QGraphicsItem* qGraphicsSceneAddSimpleText(QObject* scene, const char* txt, const Color& color)
	{
		QGraphicsScene* graphicsScene = qobject_cast<QGraphicsScene*>(scene);
		if (graphicsScene)
		{
			QGraphicsSimpleTextItem* txtItem = graphicsScene->addSimpleText( txt);
			txtItem->setPen( QPen( QColor::fromRgbF( color.r, color.g, color.b, color.a)));
			return txtItem;
		}

		return nullptr;
	}
}