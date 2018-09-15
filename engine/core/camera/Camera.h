#pragma once

#include "engine/core/math/Quaternion.h"
#include "engine/core/geom/Frustum.h"
#include "engine/core/util/Array.hpp"

namespace Echo
{
	class Ray;
	class Camera
	{
	public:
		enum ProjMode
		{
			PM_PERSPECTIVE,
			PM_ORTHO,
			PM_UI,
		};

	public:
		Camera(ProjMode mode = PM_PERSPECTIVE);
		virtual ~Camera();

		// set
		void setPosition(const Vector3& pos);
		void setDirection(const Vector3& dir);
		void setUp(const Vector3& vUp);

		const Vector3& getPosition() const { return m_position; }
		const Vector3& getDirection() const { return m_dir; }
		const Vector3& getRight() const { return m_right; }

		// 获取上方向
		const Vector3& getUp() const { return m_up; }
		const Matrix4& getViewMatrix() const { return m_matView; }
		const Matrix4& getProjMatrix() const { return m_matProj; }
		const Matrix4& getViewProjMatrix() const { return m_matVP; }

		void	setProjectionMode(ProjMode mode);
		ProjMode getProjectionMode() const;

		void setFov(Real fov);
		void setWidth(Real width);
		void setHeight(Real height);
		void setScale(Real scale);
		void			setNearClip(Real nearClip);
		void			setFarClip(Real farClip);
		Real			getFov() const;
		Real			getWidth() const;
		Real			getHeight() const;
		const Real&		getNearClip() const;
		const Real&		getFarClip() const;

		void getCameraRay(Ray& ray, const Vector2& screenPos);
		void unProjectionMousePos( Vector3& from, Vector3& to, const Vector2& screenPos );

		void update();
		void needUpdate();

	protected:
		Vector3			m_position;
		Vector3			m_dir;
		Vector3			m_up;
		Vector3			m_right;
		Matrix4			m_matView;
		bool			m_bNeedUpdateView;
		ProjMode		m_projMode;
		Real			m_fov;
		Real			m_width;
		Real			m_height;
		Real			m_scale;
		Real			m_aspect;
		Real			m_nearClip;
		Real			m_farClip;
		Matrix4			m_matProj;
		bool			m_bNeedUpdateProj;
		Matrix4			m_matVP;
	};
}
