#include <QMetaMethod>
#include "QMessageHandler.h"
#include <engine/core/base/class.h>
#include <engine/core/base/class_method_bind.h>

namespace Echo
{
	// extern set sender item function
	extern void qSetSender(QObject* item);
	extern void qSetSenderItem(QGraphicsItem* item);

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
			it->second.emplace_back(signal, (Object*)receiver, slot);
		}
		else
		{
			ConnectArray connects;
			connects.emplace_back(signal, (Object*)receiver, slot);
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
						qSetSender(sd);

						Echo::Variant::CallError error;
						Echo::Object* receiver = (Object*)conn.m_receiver;
						Echo::ClassMethodBind* method = conn.m_method;
						method->call(receiver, nullptr, 0, error);
					}
				}
			}
		}
	}

	void QMessageHandler::bind(QGraphicsItem* sender, const char* signal, void* receiver, ClassMethodBind* slot)
	{
		auto it = m_connects.find(sender);
		if (it != m_connects.end())
		{
			it->second.emplace_back(signal, (Object*)receiver, slot);
		}
		else
		{
			ConnectArray connects;
			connects.emplace_back(signal, (Object*)receiver, slot);
			m_connects[sender] = connects;
		}
	}

	qEventAll& QMessageHandler::getEvent(void* sender)
	{
		auto it = m_events.find(sender);
		if (it != m_events.end())
		{
			return it->second;
		}

		m_events[sender] = qEventAll();
		return getEvent(sender);
	}

	// on receive QWidget message
	void QMessageHandler::onReceiveQObjectMessage(QObject* sender, const String& signal)
	{
		auto it = m_connects.find((QWidget*)sender);
		if (it != m_connects.end())
		{
			ConnectArray& connectArray = it->second;
			for (Connect& conn : connectArray)
			{
				if (Echo::StringUtil::Substr(signal, "(") == Echo::StringUtil::Substr(conn.m_signal, "("))
				{
					qSetSender(sender);

					Echo::Variant::CallError error;
					Echo::Object* receiver = (Object*)conn.m_receiver;
					Echo::ClassMethodBind* method = conn.m_method;
					method->call(receiver, nullptr, 0, error);
				}
			}
		}
	}

	// on receive QGraphicsItem message
	void QMessageHandler::onReceiveQGraphicsItemMessage(QGraphicsItem* sender, const String& signal)
	{
		auto it = m_connects.find((QWidget*)sender);
		if (it != m_connects.end())
		{
			ConnectArray& connectArray = it->second;
			for (Connect& conn : connectArray)
			{
				if (Echo::StringUtil::Substr(signal, "(") == Echo::StringUtil::Substr(conn.m_signal, "("))
				{
					qSetSenderItem(sender);

					Echo::Variant::CallError error;
					Echo::Object* receiver = (Object*)conn.m_receiver;
					Echo::ClassMethodBind* method = conn.m_method;
					method->call(receiver, nullptr, 0, error);
				}
			}
		}
	}

	void QMessageHandler::onDestroyWidget()
	{
		auto it = m_connects.find((QWidget*)sender());
		if (it != m_connects.end())
		{
			m_connects.erase(it);
		}
	}
}