#pragma once

#include "scene_capture.h"

namespace Echo
{
	class SceneCapture3D : public SceneCapture
	{
		ECHO_CLASS(SceneCapture3D, SceneCapture)

	public:
		SceneCapture3D();
		virtual ~SceneCapture3D();

	protected:
		// update self
		virtual void updateInternal(float elapsedTime) override;
	};
}