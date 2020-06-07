#include "Translator.h"

namespace Echo
{
	Translator::Translator()
	{

	}

	void Translator::bindMethods()
	{
	}

	Res* Translator::load(const ResourcePath& path)
	{
		if (!path.isEmpty())
		{
			Translator* res = EchoNew(Translator);
			return res;
		}

		return nullptr;
	}

	void Translator::save()
	{

	}
}