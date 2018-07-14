#pragma once

#include "engine/core/Math/Quaternion.h"
#include "engine/core/geom/Frustum.h"
#include "engine/core/Util/Array.hpp"

namespace Echo
{
	/**
	 * 摄像机接口
	 */
	class Ray;
	class Camera
	{
	public:
		enum ProjMode
		{
			PM_PERSPECTIVE,
			PM_ORTHO,
			PM_UI,				// UI专用
		};

	public:
		Camera(ProjMode mode = PM_PERSPECTIVE, bool isFixedYaw = true);
		virtual ~Camera();

		// 更新
		virtual void frameMove(float elapsedTime) {}

		virtual void setPosition(const Vector3& pos);
		virtual void setDirection(const Vector3& dir);
		virtual void setUp(const Vector3& vUp);

		// 获取摄像机位置
		const Vector3& getPosition() const { return m_position; }

		// 获取摄像机位置指针
		Vector3* getPositionPtr() { return &m_position; }

		// 获取朝向
		const Quaternion& getOritation() const { return m_oritation; }

		// 获取朝向
		const Vector3& getDirection() const { return m_dir; }

		// 获取右方向 
		const Vector3& getRight() const { return m_right; }

		// 获取上方向
		const Vector3& getUp() const { return m_up; }

		// 获取观察矩阵
		const Matrix4& getViewMatrix() const { return m_matView; }

		// 获取投影矩阵
		const Matrix4& getProjMatrix() const { return m_matProj; }

		// 获取观察投影矩阵
		const Matrix4& getViewProjMatrix() const { return m_matVP; }

		void			setProjectionMode(ProjMode mode);
		ProjMode		getProjectionMode() const;

		void setFixedYawAxis(bool fixed) { m_bFixedYawAxis = fixed; }
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
		const Frustum&	getFrustum() const;
		Real			getNearClipWidth() const;
		Real			getNearClipHeight() const;

		void getCameraRay(Ray& ray, const Vector2& screenPos);
		void unProjectionMousePos( Vector3& from, Vector3& to, const Vector2& screenPos );

		virtual void	update();
		virtual void    needUpdate();

	protected:
		Vector3			m_position;
		Vector3			m_dir;
		Quaternion		m_oritation;
		Vector3			m_up;
		Vector3			m_right;
		bool			m_bFixedYawAxis;
		Vector3			m_fixedYawAxis;
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
		Frustum			m_frustum;
	};
}