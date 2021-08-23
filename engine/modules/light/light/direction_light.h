#pragma once

#include "light.h"

namespace Echo
{
	class DirectionLight : public Light
	{
		ECHO_CLASS(DirectionLight, Light);

	public:
		DirectionLight();
		virtual ~DirectionLight();

	protected:

	};
}
