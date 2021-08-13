#pragma once

#include "engine/core/base/object.h"
#include "engine/core/gizmos/Gizmos.h"
#include "rvo_agent.h"

namespace Echo
{
	class RvoDebugDraw
	{
	public:
		RvoDebugDraw();
		virtual ~RvoDebugDraw();

		// Update
		void update(float elapsedTime);

		// Enable
		void setEnable(bool isEnable);

	private:
		Gizmos*		m_gizmo = nullptr;
		bool		m_isEnable = false;
	};
}
