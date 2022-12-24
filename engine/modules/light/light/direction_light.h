#pragma once

#include "light.h"
#include "modules/light/shadow/shadow_camera.h"

namespace Echo
{
	class DirectionLight : public Light
	{
		ECHO_CLASS(DirectionLight, Light);

	public:
		DirectionLight();
		virtual ~DirectionLight();

		// Cast shadow
		bool isCastShadow() const { return m_castShadow; }
		void setCastShadow(bool castShadow);

		// Direction
		const Vector3 getDirection() const;

		// Shadow camera
		ShadowCamera* getShadowCamera() { return m_shadowCamera; }

		// Get frustum
		Frustum* getFrustum();

	protected:
		// update self
		virtual void updateInternal(float elapsedTime) override;

	protected:
		bool				m_castShadow = true;
		ShadowCamera*		m_shadowCamera = nullptr;
		i32					m_csmSplitCount = 3.0;
	};
}
