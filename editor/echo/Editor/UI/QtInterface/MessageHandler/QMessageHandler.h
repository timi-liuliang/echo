#pragma once

#include <engine/core/util/StringUtil.h>
#include <QWidget>
#include <QGraphicsItem>

namespace Echo
{
	class Object;
	class ClassMethodBind;
	class QMessageHandler : public QWidget
	{
		Q_OBJECT

	public:
		// Connect
		struct Connect
		{
			String				m_signal;
			void*				m_receiver;
			ClassMethodBind*	m_method;

			Connect()
				: m_receiver(nullptr)
				, m_method(nullptr)
			{}

			Connect( const char* signal, void* receiver, ClassMethodBind* method)
				: m_signal(signal)
				, m_receiver(receiver)
				, m_method(method)
			{}
		};
		typedef vector<Connect>::type ConnectArray;

	public:
		QMessageHandler(QWidget* parent = 0);
		~QMessageHandler();

		// instance
		static QMessageHandler* instance();

		// bind
		void bind(QObject* sender, const char* signal, void* receiver, ClassMethodBind* slot);
		void bind(QGraphicsItem* sender, const char* signal, void* receiver, ClassMethodBind* slot);

	public:
		// on receive QGraphicsItem message
		void onReceiveQGraphicsItemMessage(QGraphicsItem* sender, const String& signal);

	private slots:
		// on receive message
		void onReceiveMessage();

		// on widget destroy
		void onDestroyWidget();

	private:
		map<void*, ConnectArray>::type		m_connects;
	};
}