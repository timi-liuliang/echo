#include "engine/core/log/LogManager.h"
#include "Engine/core/Camera/Camera.h"
#include "engine/core/render/render/Renderer.h"
#include "engine/core/render/render/Viewport.h"
#include "engine/core/render/RenderTargetManager.h"
#include "Engine/core/Geom/Ray.h"

namespace Echo
{
	Camera::Camera(ProjMode mode, bool isFixedYaw)
		: m_bFixedYawAxis(isFixedYaw)
		, m_projMode(mode)
	{
		m_up = Vector3::UNIT_Y;
		m_position = Vector3(-150.0f, 150.0f, -150.0f);
		m_dir = Vector3::ZERO - m_position;
		m_dir.normalize();

		m_fixedYawAxis = Vector3::UNIT_Y;
		m_matView.identity();
		m_bNeedUpdateView = true;

		Viewport* pViewport = Renderer::instance()->getFrameBuffer()->getViewport();
		if(pViewport)
		{
			m_width = (Real)pViewport->getWidth();
			m_height = (Real)pViewport->getHeight();
			m_aspect = (Real)m_width / (Real)m_height;
		}
		else
		{
			m_width = 0.f;
			m_height = 0.f;
			m_aspect = 1.0f;
		}

		m_fov = Math::PI_DIV4;

		m_nearClip = 0.1f;
		m_farClip = 100.0f;
		m_matProj.identity();
		m_bNeedUpdateProj = true;
	}

	Camera::~Camera()
	{
	}

	void Camera::setPosition( const Vector3& pos )
	{
		m_position = pos;
		m_bNeedUpdateView = true;
	}

	void Camera::setDirection(const Vector3& dir)
	{
		EchoAssert(dir != m_up);

		m_dir = dir;
		m_dir.normalize();
		m_bNeedUpdateView = true;
	}

	void Camera::setUp(const Vector3& vUp)
	{
		m_up = vUp;
		m_bNeedUpdateView = true;
	}

	void Camera::yaw(Real rad)
	{
		Matrix4 matRot;
		matRot.rotateAxisReplace(m_fixedYawAxis, rad);

		m_dir = matRot.transform(m_dir);
		m_up = matRot.transform(m_up);
		m_bNeedUpdateView = true;
	}

	void Camera::pitch(Real rad)
	{
		Matrix4 matRot;
		matRot.rotateAxisReplace(m_right, rad);

		m_dir = matRot.transform(m_dir);
		m_up = matRot.transform(m_up);
		m_bNeedUpdateView = true;
	}

	void Camera::roll(Real rad)
	{
		Matrix4 matRot;
		matRot.rotateAxisReplace(m_dir, rad);

		m_dir = matRot.transform(m_dir);
		m_up = matRot.transform(m_up);
		m_bNeedUpdateView = true;
	}

	void Camera::rotate(const Vector3& vAxis, Real rad)
	{
		Matrix4 matRot;
		matRot.rotateAxisReplace(vAxis, rad);

		m_dir = matRot.transform(m_dir);
		m_up = matRot.transform(m_up);
		m_bNeedUpdateView = true;
	}

	void Camera::move(const Vector3& offset)
	{
		m_position += offset;
		m_bNeedUpdateView = true;
	}

	void Camera::setProjectionMode(ProjMode mode)
	{
		m_projMode = mode;
		m_bNeedUpdateProj = true;
	}

	Camera::ProjMode Camera::getProjectionMode() const
	{
		return m_projMode;
	}

	void Camera::setFov(Real fov)
	{
		m_fov = fov;
		m_bNeedUpdateProj = true;
		m_bNeedUpdateView = true;
	}

	void Camera::setWidth(Real width)
	{
		m_width = width;
		m_bNeedUpdateProj = true;
		m_bNeedUpdateView = true;
	}

	void Camera::setHeight(Real height)
	{
		m_height = height;
		m_bNeedUpdateProj = true;
		m_bNeedUpdateView = true;
	}

	void Camera::setNearClip(Real nearClip)
	{
		m_nearClip = nearClip;
		m_bNeedUpdateProj = true;
		m_bNeedUpdateView = true;
	}

	void Camera::setFarClip(Real farClip)
	{
		m_farClip = farClip;
		m_bNeedUpdateProj = true;
		m_bNeedUpdateView = true;
	}

	Real Camera::getFov() const
	{
		return m_fov;
	}

	Real Camera::getWidth() const
	{
		return m_width;
	}

	Real Camera::getHeight() const
	{
		return m_height;
	}

	Real Camera::getNearClip() const
	{
		return m_nearClip;
	}

	Real Camera::getFarClip() const
	{
		return m_farClip;
	}

	Real Camera::getNearClipWidth() const
	{
		return Math::Abs(m_frustum[Frustum::CORNER_NLT].x - m_frustum[Frustum::CORNER_NRT].x);
	}

	Real Camera::getNearClipHeight() const
	{
		return Math::Abs(m_frustum[Frustum::CORNER_NLT].y - m_frustum[Frustum::CORNER_NLB].y);
	}

	const Frustum& Camera::getFrustum() const
	{
		return m_frustum;
	}

	void Camera::getCameraRay(Ray& ray, const Vector2& screenPos)
	{
		Vector2 vDepth;
		Renderer::instance()->getDepthRange(vDepth);
		Vector3 v0(screenPos, vDepth.x);
		Vector3 v1(screenPos, vDepth.y);

		Renderer::instance()->unproject(ray.o, v0, m_matVP);

		Vector3 vTarget;
		Renderer::instance()->unproject(vTarget, v1, m_matVP);
		ray.dir = vTarget - ray.o;
		ray.dir.normalize();
	}

	void Camera::unProjectionMousePos( Vector3& from, Vector3& to, const Vector2& screenPos )
	{
		Vector2 vDepth;
		Renderer::instance()->getDepthRange(vDepth);
		Vector3 v0(screenPos, vDepth.x);
		Vector3 v1(screenPos, vDepth.y);

		Renderer::instance()->unproject(from, v0, m_matVP);

		Vector3 vTarget;
		Renderer::instance()->unproject(to, v1, m_matVP);
	}

	void Camera::update()
	{
		Vector3 vUp;
		Vector3 zAxis;
		if(m_bNeedUpdateView)
		{
			
			if(m_bFixedYawAxis)
				vUp = m_fixedYawAxis;
			else
				vUp = m_up;

			vUp.normalize();

			zAxis = -m_dir;
			zAxis.normalize();

			Vector3::Cross(m_right, vUp, zAxis);
			m_right.normalize();

			Vector3::Cross(vUp, zAxis, m_right);

			m_matView = Matrix4(	m_right.x, vUp.x, zAxis.x, 0,
				m_right.y, vUp.y, zAxis.y, 0,
				m_right.z, vUp.z, zAxis.z, 0,
				-Vector3::Dot(m_right, m_position),
				-Vector3::Dot(vUp, m_position),
				-Vector3::Dot(zAxis, m_position),
				1);

			zAxis.normalize();
			
			if(!m_bFixedYawAxis)
				m_up = vUp;

			if( m_bFixedYawAxis )
			{
				Vector3 zAdjustVec = -m_dir;
				zAdjustVec.normalize();
				Vector3 xVec = Vector3::UNIT_Y.cross( zAdjustVec );
				xVec.normalize();

				Vector3 yVec = zAdjustVec.cross( xVec );
				yVec.normalize();

				m_oritation.fromAxes(xVec, yVec, zAdjustVec );
			}
			else
			{
				Vector3 zAdjustVec = -m_dir;
				zAdjustVec.normalize();
				Vector3 xVec = m_up.cross( zAdjustVec );
				xVec.normalize();

				Vector3 yVec = zAdjustVec.cross( xVec );
				yVec.normalize();

				m_oritation.fromAxes(xVec, yVec, zAdjustVec );
			}
		}

		if(m_bNeedUpdateProj)
		{
			switch (m_projMode)
			{
			case PM_PERSPECTIVE:
				{
					m_aspect = (Real)m_width / (Real)m_height;
					Matrix4::PerspectiveFovRH(m_matProj, m_fov, m_aspect, m_nearClip, m_farClip);
					Renderer::instance()->convertMatProj(m_matProj, m_matProj);
				}
				break;
			case PM_ORTHO:
				{
					Matrix4::OrthoRH(m_matProj, (Real)m_width, (Real)m_height, m_nearClip, m_farClip);
					Renderer::instance()->convertMatOrho(m_matProj, m_matProj, m_nearClip, m_farClip);
				} break;
			default: ;
			}
		}

		if(m_bNeedUpdateView || m_bNeedUpdateProj)
		{
			m_matVP = m_matView * m_matProj;

			if(m_projMode == PM_PERSPECTIVE)
			{
				Real tanHalfFov = Math::Tan(m_fov * 0.5f);
				Real nearHalfH = tanHalfFov * m_nearClip;
				Real nearHalfW = nearHalfH * m_aspect;
				Real farHalfH = tanHalfFov * m_farClip;
				Real farHalfW = farHalfH * m_aspect;

				Vector3 vNearCenter = m_position - zAxis * m_nearClip;
				Vector3 vFarCenter = m_position - zAxis * m_farClip;

				m_frustum[Frustum::CORNER_NLT] = vNearCenter - m_right * nearHalfW + vUp * nearHalfH;
				m_frustum[Frustum::CORNER_NRB] = vNearCenter + m_right * nearHalfW - vUp * nearHalfH;
				m_frustum[Frustum::CORNER_NLB] = vNearCenter - m_right * nearHalfW - vUp * nearHalfH;
				m_frustum[Frustum::CORNER_NRT] = vNearCenter + m_right * nearHalfW + vUp * nearHalfH;

				m_frustum[Frustum::CORNER_FLT] = vFarCenter - m_right * farHalfW + vUp * farHalfH;
				m_frustum[Frustum::CORNER_FRB] = vFarCenter + m_right * farHalfW - vUp * farHalfH;
				m_frustum[Frustum::CORNER_FLB] = vFarCenter - m_right * farHalfW - vUp * farHalfH;
				m_frustum[Frustum::CORNER_FRT] = vFarCenter + m_right * farHalfW + vUp * farHalfH;
			}
			else
			{
				Real halfH = m_width * 0.5f;
				Real halfW = m_height * 0.5f;
				Vector3 vNearCenter = m_position - zAxis * m_nearClip;
				Vector3 vFarCenter = m_position - zAxis * m_farClip;

				m_frustum[Frustum::CORNER_NLT] = vNearCenter - m_right * halfW + vUp * halfH;
				m_frustum[Frustum::CORNER_NRB] = vNearCenter + m_right * halfW - vUp * halfH;
				m_frustum[Frustum::CORNER_NLB] = vNearCenter - m_right * halfW - vUp * halfH;
				m_frustum[Frustum::CORNER_NRT] = vNearCenter + m_right * halfW + vUp * halfH;

				m_frustum[Frustum::CORNER_FLT] = vFarCenter - m_right * halfW + vUp * halfH;
				m_frustum[Frustum::CORNER_FRB] = vFarCenter + m_right * halfW - vUp * halfH;
				m_frustum[Frustum::CORNER_FLB] = vFarCenter - m_right * halfW - vUp * halfH;
				m_frustum[Frustum::CORNER_FRT] = vFarCenter + m_right * halfW + vUp * halfH;
			}

			// Calculate near plane of frustum.
			m_frustum[Frustum::FP_NEAR].n.x = m_matVP.m03 + m_matVP.m02;
			m_frustum[Frustum::FP_NEAR].n.y = m_matVP.m13 + m_matVP.m12;
			m_frustum[Frustum::FP_NEAR].n.z = m_matVP.m23 + m_matVP.m22;
			m_frustum[Frustum::FP_NEAR].d = m_matVP.m33 + m_matVP.m32;
			m_frustum[Frustum::FP_NEAR].normalize();

			// Calculate far plane of frustum.
			m_frustum[Frustum::FP_FAR].n.x = m_matVP.m03 - m_matVP.m02;
			m_frustum[Frustum::FP_FAR].n.y = m_matVP.m13 - m_matVP.m12;
			m_frustum[Frustum::FP_FAR].n.z = m_matVP.m23 - m_matVP.m22;
			m_frustum[Frustum::FP_FAR].d = m_matVP.m33 - m_matVP.m32;
			m_frustum[Frustum::FP_FAR].normalize();

			//  Calculate left plane of frustum.
			m_frustum[Frustum::FP_LEFT].n.x = m_matVP.m03 + m_matVP.m00;
			m_frustum[Frustum::FP_LEFT].n.y = m_matVP.m13 + m_matVP.m10;
			m_frustum[Frustum::FP_LEFT].n.z = m_matVP.m23 + m_matVP.m20;
			m_frustum[Frustum::FP_LEFT].d = m_matVP.m33 + m_matVP.m30;
			m_frustum[Frustum::FP_LEFT].normalize();

			// Calculate right plane of frustum.
			m_frustum[Frustum::FP_RIGHT].n.x = m_matVP.m03 - m_matVP.m00;
			m_frustum[Frustum::FP_RIGHT].n.y = m_matVP.m13 - m_matVP.m10;
			m_frustum[Frustum::FP_RIGHT].n.z = m_matVP.m23 - m_matVP.m20;
			m_frustum[Frustum::FP_RIGHT].d = m_matVP.m33 - m_matVP.m30;
			m_frustum[Frustum::FP_RIGHT].normalize();

			// Calculate top plane of frustum.
			m_frustum[Frustum::FP_TOP].n.x = m_matVP.m03 - m_matVP.m01;
			m_frustum[Frustum::FP_TOP].n.y = m_matVP.m13 - m_matVP.m11;
			m_frustum[Frustum::FP_TOP].n.z = m_matVP.m23 - m_matVP.m21;
			m_frustum[Frustum::FP_TOP].d = m_matVP.m33 - m_matVP.m31;
			m_frustum[Frustum::FP_TOP].normalize();

			// Calculate bottom plane of frustum.
			m_frustum[Frustum::FP_BOTTOM].n.x = m_matVP.m03 + m_matVP.m01;
			m_frustum[Frustum::FP_BOTTOM].n.y = m_matVP.m13 + m_matVP.m11;
			m_frustum[Frustum::FP_BOTTOM].n.z = m_matVP.m23 + m_matVP.m21;
			m_frustum[Frustum::FP_BOTTOM].d = m_matVP.m33 + m_matVP.m31;
			m_frustum[Frustum::FP_BOTTOM].normalize();

			m_bNeedUpdateView = false;
			m_bNeedUpdateProj = false;
		}
	}

	void Camera::needUpdate()
	{
		m_bNeedUpdateView = true;
		m_bNeedUpdateProj = true;
	}

	// ¿ËÂ¡
	void Camera::clone(Camera* other)
	{
		m_position = other->m_position;
		m_dir = other->m_dir;
		m_oritation = other->m_oritation;
		m_up = other->m_up;
		m_right = other->m_right;
		m_bFixedYawAxis = other->m_bFixedYawAxis;
		m_fixedYawAxis = other->m_fixedYawAxis;
		m_matView = other->m_matView;
		m_bNeedUpdateView = other->m_bNeedUpdateView;
		m_projMode = other->m_projMode;
		m_fov = other->m_fov;
		m_width = other->m_width;
		m_height = other->m_height;
		m_aspect = other->m_aspect;
		m_nearClip = other->m_nearClip;
		m_farClip = other->m_farClip;
		m_matProj = other->m_matProj;
		m_bNeedUpdateProj = other->m_bNeedUpdateProj;
		m_matVP = other->m_matVP;
		m_frustum = other->m_frustum;
	}
}
