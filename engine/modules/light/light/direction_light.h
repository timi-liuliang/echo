#pragma once

#include "light.h"
#include "../shadow/shadow_camera.h"

namespace Echo
{
	class DirectionLight : public Light
	{
		ECHO_CLASS(DirectionLight, Light);

	public:
		DirectionLight();
		virtual ~DirectionLight();

	protected:
		ShadowCamera*		m_shadowCamera = nullptr;
	};
}
