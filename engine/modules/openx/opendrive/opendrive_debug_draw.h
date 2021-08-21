#pragma once

#include "engine/core/base/object.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
	class OpenDriveDebugDraw
	{
	public:
		OpenDriveDebugDraw();
		virtual ~OpenDriveDebugDraw();

		// Update
		void update(float elapsedTime, class OpenDrive* drive);

		// Enable
		void setEnable(bool isEnable);

	private:
		Gizmos*		m_gizmo = nullptr;
		bool		m_isEnable = false;
	};
}
