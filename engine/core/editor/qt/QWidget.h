#pragma once

#include "engine/core/base/class_method_bind.h"

#ifdef ECHO_EDITOR_MODE

class QObject;
class QWidget;
class QAction;
class QTreeWidgetItem;
class QGraphicsItem;
class QGraphicsItemGroup;
class QGraphicsLineItem;
class QGraphicsPathItem;
class QGraphicsPixmapItem;
class QGraphicsPolygonItem;
class QGraphicsProxyWidget;
class QGraphicsRectItem;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneEvent;
class QGraphicsSceneHelpEvent;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
class QGraphicsSimpleTextItem;
class QGraphicsTextItem;

# define QSIGNAL(a)  "2"#a

namespace Echo
{
	// connect signal slot
	typedef void (*qConnectObjectFun)(QObject* sender, const char* signal, void* receiver, ClassMethodBind* slot);
	typedef void (*qConnectWidgetFun)(QWidget* sender, const char* signal, void* receiver, ClassMethodBind* slot);
	typedef void (*qConnectActionFun)(QAction* sender, const char* signal, void* receiver, ClassMethodBind* slot);
	typedef void (*qConnectGraphicsItemFun)(QGraphicsItem* sender, const char* signal, void* receiver, ClassMethodBind* slot);

	// find child widget by name
	typedef QWidget* (*qFindChildFun)( QWidget* root, const char* name);

	// find child action by name
	typedef QAction* (*qFindChildActionFun)(QWidget* root, const char* name);

	// set visible
	typedef void (*qWidgetSetVisibleFun)(QWidget* widget, bool visible);

	// set enabled
	typedef void (*qWidgetSetEnableFun)(QWidget* widget, bool enabled);

	// delete QWidget
	typedef void (*qDeleteWidgetFun)(QWidget*& widget);
}

#endif
