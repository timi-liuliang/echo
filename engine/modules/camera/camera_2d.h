#pragma once

#include "engine/core/scene/node.h"
#include "engine/core/camera/Camera.h"

namespace Echo
{
	class Camera2D : public Node
	{
		ECHO_CLASS(Camera2D, Node)

	public:
		Camera2D();
		virtual ~Camera2D();

		// width|height
		float getWidth() const;
		float getHeight() const;

		// sync data to camera
		void syncDataToCamera(Camera* camera);

	protected:
		// update self
		virtual void updateInternal() override;
	};
}
