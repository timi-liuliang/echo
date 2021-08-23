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

		// Enable
		void setEnable(bool isEnable);
		bool isEnable() const { return m_isEnable; }

		// refresh
		void onDriveChanged(class OpenDrive* drive);

		// Update
		void update(float elapsedTime);

	private:
		Gizmos*		m_gizmo = nullptr;
		bool		m_isEnable = false;
	};
}
