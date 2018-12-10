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

	QGraphicsItem* qGraphicsSceneAddPath(QObject* scene, const vector<Vector2>::type paths, float widthf, const Color& color)
	{
		QGraphicsScene* graphicsScene = qobject_cast<QGraphicsScene*>(scene);
		if (graphicsScene)
		{
			QPen pen(QColor::fromRgbF(color.r, color.g, color.b, color.a));
			pen.setWidthF(widthf);

			if (paths.size() > 1)
			{
				QPainterPath painterPath(QPointF(paths[0].x, paths[0].y));
				for (size_t i = 1; i<paths.size(); i++)
				{
					painterPath.lineTo(QPointF(paths[i].x, paths[i].y));
				}

				return graphicsScene->addPath(painterPath, pen);
			}
			else
			{
				return graphicsScene->addPath(QPainterPath(), pen);
			}
		}

		return nullptr;
	}

	QGraphicsItem* qGraphicsSceneAddSimpleText(QObject* scene, const char* txt, const Color& color)
	{
		QGraphicsScene* graphicsScene = qobject_cast<QGraphicsScene*>(scene);
		if (graphicsScene)
		{
			QGraphicsSimpleTextItem* txtItem = graphicsScene->addSimpleText( txt);
			QColor qColor = QColor::fromRgbF(color.r, color.g, color.b, color.a);
			QPen pen = txtItem->pen();
			pen.setColor(qColor);
			txtItem->setPen( pen);

			return txtItem;
		}

		return nullptr;
	}
}