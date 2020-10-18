#pragma once

#include "engine/core/base/class_method_bind.h"

#ifdef ECHO_EDITOR_MODE

#include <QAction>
#include <QSplitter>
#include <QMenu>
#include <QSpinBox>
#include <QHeaderView>
#include <QCheckBox>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtWidgets/QGraphicsItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QGraphicsProxyWidget>
#include <QIcon>
#include <QDialog>
#include <QToolButton>

class QObject;
class QWidget;
class QTreeWidgetItem;
class QGraphicsPolygonItem;
class QGraphicsProxyWidget;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneEvent;
class QGraphicsSceneHelpEvent;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;

# define QSIGNAL(a)  "2"#a

namespace Echo
{
	// sender
	typedef QObject* (*qSenderFun)();

	// connect signal slot
	typedef void (*qConnectObjectFun)(QObject* sender, const char* signal, void* receiver, ClassMethodBind* slot);
	typedef void (*qConnectWidgetFun)(QWidget* sender, const char* signal, void* receiver, ClassMethodBind* slot);
	typedef void (*qConnectActionFun)(QAction* sender, const char* signal, void* receiver, ClassMethodBind* slot);
	typedef void (*qConnectGraphicsItemFun)(QGraphicsItem* sender, const char* signal, void* receiver, ClassMethodBind* slot);

	// find child widget by name
	typedef QWidget* (*qFindChildFun)( QWidget* root, const char* name);

	// find child action by name
	typedef QAction* (*qFindChildActionFun)(QWidget* root, const char* name);
}

#endif
