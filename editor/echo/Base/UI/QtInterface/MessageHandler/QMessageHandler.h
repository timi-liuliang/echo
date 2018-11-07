#pragma once

#include <engine/core/util/StringUtil.h>
#include <QWidget>

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
			void*				m_receiver;
			ClassMethodBind*	m_method;

			Connect()
				: m_receiver(nullptr)
				, m_method(nullptr)
			{}

			Connect(void* receiver, ClassMethodBind* method)
				: m_receiver(receiver)
				, m_method(method)
			{}
		};

	public:
		QMessageHandler(QWidget* parent = 0);
		~QMessageHandler();

		// instance
		static QMessageHandler* instance();

		// bind
		void bind(QWidget* sender, const char* signal, void* receiver, ClassMethodBind* slot);

	private slots:
		// on receive message
		void onReceiveMessage();

	private:
		map<String, Connect>::type		m_connects;
	};
}