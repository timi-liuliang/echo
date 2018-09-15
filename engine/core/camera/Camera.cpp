#include "engine/core/log/Log.h"
#include "engine/core/camera/Camera.h"
#include "engine/core/render/interface/Renderer.h"
#include "engine/core/render/interface/Viewport.h"
#include "engine/core/render/interface/RenderTargetManager.h"
#include "engine/core/geom/Ray.h"

namespace Echo
{
	Camera::Camera(ProjMode mode)
		: m_projMode(mode)
		, m_scale( 1.f)
	{
		m_up = Vector3::UNIT_Y;
		m_position = Vector3(-150.0f, 150.0f, -150.0f);
		m_dir = Vector3::ZERO - m_position;
		m_dir.normalize();

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

	void Camera::setScale(Real scale) 
	{ 
		m_scale = scale; 
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

	const Real& Camera::getNearClip() const
	{
		return m_nearClip;
	}

	const Real& Camera::getFarClip() const
	{
		return m_farClip;
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
		if(m_bNeedUpdateView)
		{
			// calc axis
			Vector3 xAxis;
			Vector3 yAxis;
			Vector3 zAxis = -m_dir;
			Vector3::Cross(xAxis, m_up, zAxis);
			Vector3::Cross(yAxis, zAxis, m_right);

			m_matView = Matrix4(
				xAxis.x,							yAxis.x,						zAxis.x,							0,
				xAxis.y,							yAxis.y,						zAxis.y,							0,
				xAxis.z,							yAxis.z,						zAxis.z,							0,
				-Vector3::Dot(m_right, m_position), -Vector3::Dot(yAxis, m_position), -Vector3::Dot(zAxis, m_position),	1
			);

			m_right = xAxis;
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
					Matrix4::OrthoRH(m_matProj, (Real)m_width * m_scale, (Real)m_height * m_scale, m_nearClip, m_farClip);
					Renderer::instance()->convertMatOrho(m_matProj, m_matProj, m_nearClip, m_farClip);
				} break;
			default: ;
			}
		}

		if(m_bNeedUpdateView || m_bNeedUpdateProj)
		{
			m_matVP = m_matView * m_matProj;

			m_bNeedUpdateView = false;
			m_bNeedUpdateProj = false;
		}
	}
}
