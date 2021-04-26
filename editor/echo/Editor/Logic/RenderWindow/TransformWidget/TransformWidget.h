#pragma once

#include <vector>
#include <engine/core/math/Math.h>
#include <engine/core/scene/node.h>
#include <engine/core/geom/Box3.h>
#include <engine/core/geom/Ray.h>
#include <engine/core/gizmos/Gizmos.h>
#include <engine/core/main/Engine.h>

namespace Studio
{
	class TransformWidget
	{
	public:
		enum class EditType
		{
			Translate,
			Rotate,
			Scale,
		};

		enum class MoveType
		{
			None = -1, 
			XAxis,
			YAxis,
			ZAxis,
			XYPlane,
			YZPlane,
			XZPlane,
		};

		enum class RotateType
		{
			None = -1,
			XAxis,
			YAxis,
			ZAxis,
		};

		enum class ScaleType
		{
			None = -1,
			XAxis,
			YAxis,
			ZAxis,
			All,
		};

		class Listener
		{
		public:
			// on translate
			virtual void onTranslate(const Echo::Vector3& trans) {}
			virtual void onRotate(const Echo::Vector3& rotate) {}
			virtual void onScale(const Echo::Vector3& scale) {}
		};

	public:
		TransformWidget();

		// mouse event
		bool onMouseDown(const Echo::Vector2& localPos);
		void onMouseMove(const Echo::Vector2& localPos);
		void onMouseUp();

		// set listener
		void setListener(Listener* listener) { m_listener = listener; }

		// position
		void setPosition(const Echo::Vector3& pos);

		// visible
		void setVisible(bool visible);

		// set render type
		void setRenderType2d(bool is2d);

		// set edit type
		void SetEditType(EditType type);

		// set scale
		void  setScale(float fScale);

		// check move type
		bool isMoveType(MoveType type) const { return m_moveType == type; }
		bool isRotateType(RotateType type) const { return m_rotateType == type; }
		bool isScaleType(ScaleType type) const { return m_scaleType==type; }

	private:
		// draw
		void draw();
		void drawCone(float radius, float height, const Echo::Transform& transform, const Echo::Color& color);
		void drawBox(const Echo::Vector3& extent, const Echo::Transform& transform, const Echo::Color& color);
		void drawCircle(float radius, const Echo::Transform& transform, const Echo::Color& color);

		// update collision box
		void updateTranslateCollisionBox();

		// translate
		void onTranslate(const Echo::Vector3& trans);
		void onRotate(const Echo::Vector3& rotate);

	private:
		// translate help function
		float translateOnAxis(const Echo::Ray& ray0, const Echo::Ray& ray1, const Echo::Vector3& entityPos, const Echo::Vector3& translateAxis);
		Echo::Vector3* translateOnPlane(Echo::Vector3* pOut, const Echo::Plane& plane, const Echo::Ray& ray0, const Echo::Ray& ray1);

		// rotate help function
		float rotateOnPlane(const Echo::Plane& plane, const Echo::Ray& ray0, const Echo::Ray& ray1);

	private:
		bool						m_is2d = false;
		float						m_2dDepth = 250.f;
		Echo::Vector2				m_mousePos;
		Echo::Vector3				m_position;
		Echo::Gizmos*				m_axis = nullptr;
		Echo::array<Echo::Box3, 6>	m_moveBoxs;
		EditType					m_editType;
		MoveType					m_moveType;
		RotateType					m_rotateType;
		ScaleType					m_scaleType;
		bool						m_isVisible = true;
		float						m_scale = 1.f;
		Listener*					m_listener = nullptr;
	};
}