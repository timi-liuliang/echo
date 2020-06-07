#include "Localization.h"

namespace Echo
{
	Localization* Localization::instance()
	{
		static Localization* inst = EchoNew(Localization);
		return inst;
	}

	void Localization::bindMethods()
	{
	}
}