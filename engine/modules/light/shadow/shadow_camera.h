#pragma once

#include <engine/core/math/Math.h>
#include "engine/core/geom/AABB.h"
#include "engine/core/geom/Frustum.h"
#include "engine/core/render/base/camera/render_camera.h"

namespace Echo
{
	class ShadowCamera : public RenderCamera
	{
	public:
		ShadowCamera();
		~ShadowCamera();

		// Update
		void update(const AABB* visibleActorsAABB);

		// Set direction
		void setDirection(const Vector3& dir);

		// View|ViewProj Matrix
		const Matrix4& getViewProjMatrix() const { return m_viewProj; }
		const Matrix4& getViewMatrix() const { return m_view; }

		// Visible actors's aabb
		AABB& getVisibleActorsAABB() { return m_visibleActorsAABB; }

		// Get frustum
		Frustum* getFrustum() { return &m_frustum; }

		// Global uniform value
		virtual void* getGlobalUniformValue(const String& name) override;

	private:
		void calcOrthoRH(Matrix4& oOrth, const AABB& box, const Matrix4& viewMat);

	private:
		float		m_width;
		float		m_height;
		float		m_near;
		float		m_far;
		Vector3		m_position;
		Vector3     m_forward = -Vector3::UNIT_Z;
		Vector3		m_up;
		Vector3		m_right;
		Matrix4		m_viewProj = Matrix4::IDENTITY;
		Matrix4		m_view;
		Matrix4		m_proj;
		AABB 		m_visibleActorsAABB;
		Frustum		m_frustum;
	};
}