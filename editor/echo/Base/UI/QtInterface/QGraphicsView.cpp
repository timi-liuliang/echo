#include <QGraphicsScene>
#include <QGraphicsView>

namespace Echo
{
	void qGraphicsViewSetScene(QWidget* view, QObject* scene)
	{
		QGraphicsView* graphicsView = qobject_cast<QGraphicsView*>(view);
		QGraphicsScene* graphicsScene = qobject_cast<QGraphicsScene*>(scene);
		if (graphicsView && graphicsScene)
		{
			graphicsView->setScene(graphicsScene);
			graphicsView->setRenderHint( QPainter::Antialiasing, true);
		}
	}
}