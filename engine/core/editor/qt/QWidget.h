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
	void qConnect(QWidget* sender, const char* signal, void* receiver, ClassMethodBind* slot);
	void qConnect(QAction* sender, const char* signal, void* receiver, ClassMethodBind* slot);
	void qConnect(QGraphicsItem* sender, const char* signal, void* receiver, ClassMethodBind* slot);

	// find child widget by name
	QWidget* qFindChild( QWidget* root, const char* name);

	// find child action by name
	QAction* qFindChildAction(QWidget* root, const char* name);

	// delete QWidget
	void qDeleteWidget(QWidget*& widget);
}

#endif