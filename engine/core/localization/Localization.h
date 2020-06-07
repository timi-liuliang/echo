#pragma once

#include "engine/core/base/object.h"
#include "Translator.h"

namespace Echo
{
	class Localization : public Object
	{
		ECHO_SINGLETON_CLASS(Localization, Object);

	public:
		virtual ~Localization() {}

		// get instance
		static Localization* instance();

	private:
		Localization() {}

	private:
		map<String, Translator*>::type	m_translators;
	};
}