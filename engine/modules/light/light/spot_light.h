#pragma once

#include "light.h"

namespace Echo
{
	class SpotLight : public Light
	{
		ECHO_CLASS(SpotLight, Light);

	public:
		SpotLight();
		virtual ~SpotLight();

	protected:

	};
}
