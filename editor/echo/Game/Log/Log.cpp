#include "Log.h"
#include <QFile>
#include <engine/core/util/StringUtil.h>

namespace Game
{
	GameLog::GameLog(const Echo::String& name)
		: Log(name)
	{

	}

	/** »’÷æ ‰≥ˆ */
	void GameLog::logMessage(LogLevel level, const Echo::String &msg)
	{
		Echo::String msgs = Echo::StringUtil::Format("@:-log@:%d@:%s", level, msg.c_str());

		// write to stderr
		QFile fileout;
		fileout.open(stdout, QIODevice::WriteOnly);
		fileout.write(msgs.c_str(), msgs.length());
		fileout.close();
	}
}