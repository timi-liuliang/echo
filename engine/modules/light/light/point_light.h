#pragma once

#include "light.h"

namespace Echo
{
	class PointLight : public Light
	{
		ECHO_CLASS(PointLight, Light);

	public:
		PointLight();
        virtual ~PointLight();

	protected:

	};
}
