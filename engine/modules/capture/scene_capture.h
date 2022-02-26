#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class SceneCapture : public Node
	{
		ECHO_VIRTUAL_CLASS(SceneCapture, Node)

	public:
		SceneCapture();
		virtual ~SceneCapture();

	protected:
		// update self
		virtual void updateInternal(float elapsedTime) override;

	protected:
		/** Capture priority within the frame to sort scene capture on GPU to resolve interdependencies between multiple capture nodes. Highest come first. */
		i32			m_sortPriority = 0;
	};
}
