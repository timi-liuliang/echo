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

	void QMessageHandler::bind(QObject* sender, const char* signal, void* receiver, ClassMethodBind* slot)
	{
		auto it = m_connects.find( sender);
		if (it != m_connects.end())
		{
			it->second.push_back(Connect( signal, (Object*)receiver, slot));
		}
		else
		{
			ConnectArray connects;
			connects.push_back(Connect(signal, (Object*)receiver, slot));
			m_connects[sender] = connects;

			QObject::connect(sender, SIGNAL(destroyed()), this, SLOT(onDestroyWidget()));
		}
	}

	void QMessageHandler::onReceiveMessage()
	{
		QObject* sd = sender();
		if (sd)
		{
			int index = senderSignalIndex();
			Echo::String signal = String("2") + sd->metaObject()->method(index).methodSignature().toStdString().c_str();
			auto it = m_connects.find((QWidget*)sd);
			if(it!=m_connects.end())
			{
				ConnectArray& connectArray = it->second;
				for (Connect& conn : connectArray)
				{
					if (Echo::StringUtil::Substr(signal, "(") == Echo::StringUtil::Substr(conn.m_signal, "("))
					{
						Echo::Variant::CallError error;
						Echo::Object* receiver = (Object*)conn.m_receiver;
						Echo::ClassMethodBind* method = conn.m_method;
						method->call(receiver, nullptr, 0, error);
					}
				}
			}
		}
	}

	// on widget destroy
	void QMessageHandler::onDestroyWidget()
	{
		auto it = m_connects.find((QWidget*)sender());
		if (it != m_connects.end())
		{
			m_connects.erase(it);
		}
	}
}