#include <engine/core/editor/qt/QWidgets.h>
#include "MessageHandler/QMessageHandler.h"

namespace Echo
{
	// query widget's qEventAll
	const qEventAll& qGetEventAll(QObject* sender)
	{
		return QMessageHandler::instance()->getEvent(sender);
	}
}