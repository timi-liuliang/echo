#pragma once

#include "engine/core/base/object.h"
#include "engine/core/gizmos/Gizmos.h"
#include "opendrive.h"

namespace Echo
{
	class OpenDriveDebugDraw : public Object
	{
		ECHO_CLASS(OpenDriveDebugDraw, Object)

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
		Vector3 toDir3(double radian, double h=0.0);
		Vector3 toVec3(double x, double y, double h=0.0);

	protected:
		// Draw reference line
		void drawLine(OpenDrive::Line* line);
		void drawArc(OpenDrive::Arc* arc);
		void drawSpiral(OpenDrive::Spiral* spiral);
		void drawPoly3(OpenDrive::Poly3* poly3);
		void drawParamPoly3(OpenDrive::ParamPoly3* paramPoly3);
		void drawArrow(double endX, double endY, double hdg, Color& color, double length);
		
		// Draw Lane Outer
		void drawLaneOuterBorder(OpenDrive::Road& road, OpenDrive::LaneSection& laneSection);

	private:
		Gizmos*		m_gizmo = nullptr;
		bool		m_isEnable = false;
		bool		m_is2D = false;
		bool		m_isDrawArrow = true;
		Color		m_lineColor = Color::fromRGBA(248, 167, 80);
		Color		m_spiralColor = Color::fromRGBA(1, 106, 163);
		Color		m_arcColor = Color::fromRGBA(144, 207, 77);
		Color		m_poly3Color = Color::fromRGBA(255, 98, 81);
		Color		m_paramPoly3Color = Color::fromRGBA(186, 135, 243);
		Color		m_laneBorderColor = Color::fromRGBA(160, 160, 160);
	};
}
