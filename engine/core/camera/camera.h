#pragma once

#include "engine/core/math/Quaternion.h"
#include "engine/core/geom/Frustum.h"
#include "engine/core/util/Array.hpp"
#include "engine/core/render/base/camera/render_camera.h"

namespace Echo
{
	class Ray;
	class Camera : public RenderCamera
	{
	public:
		enum class ProjMode
		{
			PM_PERSPECTIVE,
			PM_ORTHO,
		};

	public:
		Camera(ProjMode mode = ProjMode::PM_PERSPECTIVE);
		virtual ~Camera();

		// proj mode
		ProjMode getProjectionMode() const;
		void setProjectionMode(ProjMode mode);

		// position
		const Vector3& getPosition() const { return m_position; }
		void setPosition(const Vector3& pos);

		// scale
		void setScale(Real scale);
		float getScale() const { return m_scale; }

		// Rotation
		void setOrientation(const Quaternion& rotation);
		const Quaternion& getRotation() const { return m_rotation; }

		// direction
		const Vector3& getForward() const { return m_forward; }

		// up 
		const Vector3& getUp() const { return m_up; }

		// right
		const Vector3& getRight() const { return m_right; }

		// fov
		Real getFov() const;
		void setFov(Real fov);

		// width
		ui32 getWidth() const;
		void setWidth(ui32 width);

		// height
		ui32 getHeight() const;
		void setHeight(ui32 height);

		// near clip
		const Real&	getNear() const;
		void setNear(Real nearClip);

		// far clip
		const Real&	getFar() const;
		void setFar(Real farClip);

		// update
		void update();

		// calculate
		void getCameraRay(Ray& ray, const Vector2& screenPos);

		// Frustum
		Frustum& getFrustum() { return m_frustum; }

		// matrix
		const Matrix4& getViewMatrix() const { return m_matView; }
		const Matrix4& getProjMatrix() const { return m_matProj; }
		const Matrix4& getViewProjMatrix() const { return m_matVP; }

		// Global uniform value
		virtual void* getGlobalUniformValue(const String& name) override;

	protected:
		// Update matrix
		void updateMatrix();

		// Update Frustum
		void updateFrustum();

	protected:
		ProjMode		m_projMode;
		Vector3			m_position;
		Real			m_scale = 1.f;
		Quaternion		m_rotation;
		Vector3			m_forward;
		Vector3			m_up = Vector3::UNIT_Y;
		Vector3			m_right;
		Matrix4			m_matView;
		bool			m_viewDirty = true;
		Real			m_fov = Math::PI_DIV4;
		ui32			m_width;
		ui32			m_height;
		Real			m_nearClip = 0.1f;
		Real			m_farClip = 100.f;
		Matrix4			m_matProj;
		bool			m_projDirty = true;
		Matrix4			m_matVP;
		Frustum			m_frustum;
	};
}
