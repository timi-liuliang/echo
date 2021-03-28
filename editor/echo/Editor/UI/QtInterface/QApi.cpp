#include "Studio.h"
#include "QEvent.cx"
#include "QGraphicsItem.cx"
#include "QGraphicsScene.cx"
#include "QGraphicsView.cx"
#include "QUiLoader.cx"
#include "QWidget.cx"

#define BIND_EDITOR_FUN(Fun) this->Fun = Echo::Fun

namespace Studio
{
    // init functions
    void AStudio::initFunctions()
    {
        // QEvent
        BIND_EDITOR_FUN(qObjectGetEventAll);
        BIND_EDITOR_FUN(qGraphicsItemGetEventAll);

		// get sender item
		BIND_EDITOR_FUN(qSenderItem);
		BIND_EDITOR_FUN(qGraphicsSceneNew);
		BIND_EDITOR_FUN(qGraphicsSceneAddWidget);
		BIND_EDITOR_FUN(qGraphicsSceneAddLine);
		BIND_EDITOR_FUN(qGraphicsSceneAddRect);
		BIND_EDITOR_FUN(qGraphicsSceneAddPath);
		BIND_EDITOR_FUN(qGraphicsSceneAddEclipse);
		BIND_EDITOR_FUN(qGraphicsSceneAddPixmap);

		// disable viewport update
		BIND_EDITOR_FUN(qGraphicsViewSceneRect);
		BIND_EDITOR_FUN(qGraphicsViewFitInView);

		// load ui file by path. then return the widget
		BIND_EDITOR_FUN(qLoadUi);

		// connect signal slot
		BIND_EDITOR_FUN(qSender);
		BIND_EDITOR_FUN(qConnectObject);
		BIND_EDITOR_FUN(qConnectWidget);
		BIND_EDITOR_FUN(qConnectAction);
		BIND_EDITOR_FUN(qConnectGraphicsItem);
    }
}
