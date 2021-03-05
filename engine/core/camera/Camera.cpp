#include "engine/core/log/Log.h"
#include "engine/core/camera/Camera.h"
#include "engine/core/render/base/renderer.h"
#include "engine/core/render/base/view_port.h"
#include "engine/core/geom/Ray.h"

namespace Echo
{
	Camera::Camera(ProjMode mode)
		: m_projMode(mode)
	{
		m_position = Vector3(-150.0f, 150.0f, -150.0f);
		m_dir = Vector3::ZERO - m_position;
		m_dir.normalize();

		m_matView.identity();
		m_matProj.identity();

		m_width = (Real)Renderer::instance()->getWindowWidth();
		m_height = (Real)Renderer::instance()->getWindowHeight();
		m_aspect = (Real)m_width / (Real)m_height;
	}

	Camera::~Camera()
	{
	}

	void Camera::setPosition( const Vector3& pos )
	{
		m_position = pos;
		m_isViewDirty = true;
	}

	void Camera::setDirection(const Vector3& dir)
	{
		EchoAssert(dir != m_up);

		m_dir = dir;
		m_dir.normalize();
		m_isViewDirty = true;
	}

	void Camera::setUp(const Vector3& vUp)
	{
		m_up = vUp;
		m_isViewDirty = true;
	}

	void Camera::setProjectionMode(ProjMode mode)
	{
		m_projMode = mode;
		m_isProjDirty = true;
	}

	Camera::ProjMode Camera::getProjectionMode() const
	{
		return m_projMode;
	}

	void Camera::setFov(Real fov)
	{
		m_fov = fov;
		m_isProjDirty = true;
		m_isViewDirty = true;
	}

	void Camera::setWidth(ui32 width)
	{
		m_width = width;
		m_isProjDirty = true;
		m_isViewDirty = true;
	}

	void Camera::setHeight(ui32 height)
	{
		m_height = height;
		m_isProjDirty = true;
		m_isViewDirty = true;
	}

	void Camera::setScale(Real scale) 
	{ 
		m_scale = scale; 
		m_isProjDirty = true;
		m_isViewDirty = true;
	}

	void Camera::setNearClip(Real nearClip)
	{
		m_nearClip = nearClip;
		m_isProjDirty = true;
		m_isViewDirty = true;
	}

	void Camera::setFarClip(Real farClip)
	{
		m_farClip = farClip;
		m_isProjDirty = true;
		m_isViewDirty = true;
	}

	Real Camera::getFov() const
	{
		return m_fov;
	}

	ui32 Camera::getWidth() const
	{
		return m_width;
	}

	ui32 Camera::getHeight() const
	{
		return m_height;
	}

	const Real& Camera::getNear() const
	{
		return m_nearClip;
	}

	const Real& Camera::getFar() const
	{
		return m_farClip;
	}

	void Camera::getCameraRay(Ray& ray, const Vector2& screenPos)
	{
		Vector2 vDepth;
		Renderer::instance()->getDepthRange(vDepth);
		Vector3 v0(screenPos, vDepth.x);
		Vector3 v1(screenPos, vDepth.y);

		Renderer::instance()->unproject(ray.m_origin, v0, m_matVP);

		Vector3 vTarget;
		Renderer::instance()->unproject(vTarget, v1, m_matVP);
		ray.m_dir = vTarget - ray.m_origin;
		ray.m_dir.normalize();
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
		if(m_isViewDirty)
		{
			Vector3 xAxis;					// right
			Vector3 yAxis;					// up axis
			Vector3 zAxis = -m_dir;			// forward

			Vector3::Cross(xAxis, m_up, zAxis);		xAxis.normalize();
			Vector3::Cross(yAxis, zAxis, xAxis);	yAxis.normalize();

			m_matView = Matrix4(
				xAxis.x,						  yAxis.x,							zAxis.x,							0,
				xAxis.y,						  yAxis.y,						    zAxis.y,							0,
				xAxis.z,						  yAxis.z,							zAxis.z,							0,
				-Vector3::Dot(xAxis, m_position), -Vector3::Dot(yAxis, m_position), -Vector3::Dot(zAxis, m_position),	1
			);

			m_right = xAxis;

			Renderer::instance()->convertMatView(m_matView);
		}

		if(m_isProjDirty)
		{
			switch (m_projMode)
			{
			case ProjMode::PM_PERSPECTIVE:
				{
					m_aspect = (Real)m_width / (Real)m_height;
					Matrix4::PerspectiveFovRH(m_matProj, m_fov, m_aspect, m_nearClip, m_farClip);
					Renderer::instance()->convertMatProj(m_matProj, m_matProj);
				}
				break;
			case ProjMode::PM_ORTHO:
				{
					Matrix4::OrthoRH(m_matProj, (Real)m_width * m_scale, (Real)m_height * m_scale, m_nearClip, m_farClip);
					Renderer::instance()->convertMatOrho(m_matProj, m_matProj, m_nearClip, m_farClip);
				} break;
			default: ;
			}
		}

		if(m_isViewDirty || m_isProjDirty)
		{
			m_matVP = m_matView * m_matProj;

			m_isViewDirty = false;
			m_isProjDirty = false;
		}
	}
}
