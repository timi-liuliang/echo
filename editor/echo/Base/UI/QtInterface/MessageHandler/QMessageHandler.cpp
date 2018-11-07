#include <QMetaMethod>
#include "QMessageHandler.h"
#include <engine/core/base/class.h>
#include <engine/core/base/class_method_bind.h>

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

	void QMessageHandler::bind(QWidget* sender, const char* signal, void* receiver, ClassMethodBind* slot)
	{
		Echo::String signalName = Echo::StringUtil::Format("%p:%s", &sender, signal);
		signalName = Echo::StringUtil::Substr(signalName, "(");
		m_connects[signalName] = Connect((Object*)receiver, slot);
	}

	void QMessageHandler::onReceiveMessage()
	{
		QObject* sd = sender();
		if (sd)
		{
			int index = senderSignalIndex();
			Echo::String signal = sd->metaObject()->method(index).methodSignature().toStdString().c_str();

			Echo::String signalName = Echo::StringUtil::Format("%p:%s", &sd, signal.c_str());
			signalName = Echo::StringUtil::Substr(signalName, "(");
			auto it = m_connects.find(signalName);
			if(it!=m_connects.end())
			{
				Echo::Variant::CallError error;
				Echo::Object* receiver = (Object*)it->second.m_receiver;
				Echo::ClassMethodBind* method = it->second.m_method;
				method->call(receiver, nullptr, 0, error);
			}
		}
	}
}