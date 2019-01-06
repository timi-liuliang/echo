#include <QGraphicsScene>
#include <QGraphicsView>

namespace Echo
{
	void qGraphicsViewDisableViewportAutoUpdate(QWidget* view)
	{
		QGraphicsView* graphicsView = qobject_cast<QGraphicsView*>(view);
		if (graphicsView)
		{
			graphicsView->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
		}
	}

	void qGraphicsViewport(QWidget* view, float& left, float& top, float& width, float& height)
	{
		QGraphicsView* graphicsView = qobject_cast<QGraphicsView*>(view);
		if (graphicsView)
		{
			QRectF rectF = graphicsView->mapToScene(graphicsView->viewport()->geometry()).boundingRect();
			left = rectF.left();
			top = rectF.top();
			width = rectF.width();
			height = rectF.height();
		}
	}

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

	void qGraphicsViewSetScale(QWidget* view, float scaleX, float scaleY)
	{
		QGraphicsView* graphicsView = qobject_cast<QGraphicsView*>(view);
		if (graphicsView)
		{
			graphicsView->scale(scaleX, scaleY);
		}
	}
}