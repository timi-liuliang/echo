#pragma once

#include "engine/core/base/object.h"
#include "engine/core/gizmos/Gizmos.h"
#include "opendrive.h"

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

	public:
		// Convert
		Vector3 toVec3(double x, double y, double h=0.0);

	protected:
		// Draw
		void drawLine(OpenDrive::Line* line);
		void drawArc(OpenDrive::Arc* arc);
		void drawSpiral(OpenDrive::Spiral* spiral);
		void drawArrow(double endX, double endY, double hdg, Color& color, double length);

	private:
		Gizmos*		m_gizmo = nullptr;
		bool		m_isEnable = false;
		bool		m_is2D = false;
	};
}
