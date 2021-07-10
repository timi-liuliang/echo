#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class Camera;
	class Camera3D : public Node
	{
		ECHO_CLASS(Camera3D, Node)

	public:
		Camera3D();
		virtual ~Camera3D();

		// near
		float getNear() const { return m_near; }
		void setNear(float nearClip) {m_near = nearClip;}

		// far
		float getFar() const { return m_far; }
		void setFar(float farClip) { m_far = farClip; }

		// sync data to camera
		void syncDataToCamera(Camera* camera);

	protected:
		// update self
		virtual void updateInternal() override;

	protected:
		float	m_near;
		float	m_far;
	};
}
