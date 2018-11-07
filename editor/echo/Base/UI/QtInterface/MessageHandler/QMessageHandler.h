#pragma once

#include <engine/core/util/StringUtil.h>
#include <QWidget>

namespace Echo
{
	class QMessageHandler : public QWidget
	{
		Q_OBJECT

	public:
		QMessageHandler(QWidget* parent = 0);
		~QMessageHandler();

		// instance
		static QMessageHandler* instance();

	private slots:
		// on receive message
		void onReceiveMessage();
	};
}