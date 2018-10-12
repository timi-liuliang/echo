#pragma once

#include <engine/core/base/object.h>

namespace Echo
{
	class LightSettings : public Object
	{
		ECHO_SINGLETON_CLASS(LightSettings, Object);

	public:
		// instance
		static LightSettings* instance();

	private:
		LightSettings();
		virtual ~LightSettings();
	};
}