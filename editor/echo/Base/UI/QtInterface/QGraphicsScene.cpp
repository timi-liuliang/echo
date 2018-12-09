#include <QGraphicsScene>

namespace Echo
{
	QObject* qGraphicsSceneNew()
	{
		return new QGraphicsScene();
	}

	QGraphicsLineItem* qGraphicsSceneAddLine(QObject* scene, float startX, float startY, float endX, float endY)
	{
		QGraphicsScene* graphicsScene = qobject_cast<QGraphicsScene*>(scene);
		if (graphicsScene)
		{
			return graphicsScene->addLine( startX, startY, endX, endY);
		}

		return nullptr;
	}
}