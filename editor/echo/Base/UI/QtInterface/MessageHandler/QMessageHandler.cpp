#include "QMessageHandler.h"
#include <engine/core/base/class.h>

namespace Echo
{
	QMessageHandler::QMessageHandler(QWidget* parent)
		: QWidget(parent)
	{
		setHidden(true);
	}

	QMessageHandler::~QMessageHandler()
	{

	}

	QMessageHandler* QMessageHandler::instance()
	{
		static QMessageHandler* inst = new QMessageHandler(nullptr);
		return inst;
	}

	void QMessageHandler::onReceiveMessage()
	{
		QObject* sd = sender();
		int a = 10;

		//Echo::Variant::CallError error;
		//Echo::Object* receiver;
		//Echo::ClassMethodBind* method;
		//method->call(receiver, nullptr, 0, error);
	}
}