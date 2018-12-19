#include <QtWidgets/QWidget>
#include <QAction>
#include <QGraphicsItem>
#include <engine/core/base/class_method_bind.h>
#include "QMessageHandler.h"

namespace Echo
{
	void qConnect(QObject* sender, const char* signal, void* receiver, ClassMethodBind* slot)
	{
		if (sender)
		{
			QMessageHandler::instance()->bind(sender, signal, receiver, slot);
			QObject::connect(sender, signal, QMessageHandler::instance(), SLOT(onReceiveMessage()));
		}
	}

	void qConnect(QWidget* sender, const char* signal, void* receiver, ClassMethodBind* slot)
	{
		if (sender)
		{
			QMessageHandler::instance()->bind( sender, signal, receiver, slot);
			QObject::connect(sender, signal, QMessageHandler::instance(), SLOT(onReceiveMessage()));
		}
	}

	void qConnect(QAction* sender, const char* signal, void* receiver, ClassMethodBind* slot)
	{
		if (sender)
		{
			QMessageHandler::instance()->bind(sender, signal, receiver, slot);
			QObject::connect(sender, signal, QMessageHandler::instance(), SLOT(onReceiveMessage()));
		}
	}

	void qConnect(QGraphicsItem* sender, const char* signal, void* receiver, ClassMethodBind* slot)
	{
		if (sender)
		{
			QMessageHandler::instance()->bind( sender, signal, receiver, slot);
		}
	}

	QWidget* qFindChild(QWidget* root, const char* name)
	{
		QWidget* widget = root->findChild<QWidget*>(name);
		return widget;
	}

	QAction* qFindChildAction(QWidget* root, const char* name)
	{
		QAction* action = root->findChild<QAction*>(name);
		return action;
	}

	void qWidgetSetVisible(QWidget* widget, bool visible)
	{
		if (widget)
		{
			widget->setVisible(visible);
		}
	}

	void qDeleteWidget(QWidget*& widget)
	{
		if (widget)
		{
			delete widget;
			widget = nullptr;
		}
	}
}
