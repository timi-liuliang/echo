#include "engine/core/log/Log.h"
#include "engine/core/camera/camera.h"
#include "engine/core/render/base/renderer.h"
#include "engine/core/render/base/misc/view_port.h"
#include "engine/core/geom/Ray.h"

namespace Echo
{
	Camera::Camera(ProjMode mode)
		: m_projMode(mode)
	{
		m_position = Vector3(-150.0f, 150.0f, -150.0f);
		m_forward = -Vector3::UNIT_Z;
		m_up = Vector3::UNIT_Y;
		Vector3::Cross(m_right, m_forward, m_up);

		m_matView.identity();
		m_matProj.identity();

		m_width = (Real)Renderer::instance()->getWindowWidth();
		m_height = (Real)Renderer::instance()->getWindowHeight();
	}

	Camera::~Camera()
	{
	}

	void Camera::setPosition( const Vector3& pos )
	{
		m_position = pos;
		m_viewDirty = true;
	}

	void Camera::setOrientation(const Quaternion& rotation)
	{
		m_rotation = rotation;

		m_forward = m_rotation.rotateVec3(-Vector3::UNIT_Z);
		m_up = m_rotation.rotateVec3(Vector3::UNIT_Y);
		Vector3::Cross(m_right, m_forward, m_up);

		m_viewDirty = true;
	}

	void Camera::setProjectionMode(ProjMode mode)
	{
		m_projMode = mode;
		m_projDirty = true;
	}

	Camera::ProjMode Camera::getProjectionMode() const
	{
		return m_projMode;
	}

	void Camera::setFov(Real fov)
	{
		m_fov = fov;
		m_projDirty = true;
		m_viewDirty = true;
	}

	void Camera::setWidth(ui32 width)
	{
		m_width = width;
		m_projDirty = true;
		m_viewDirty = true;
	}

	void Camera::setHeight(ui32 height)
	{
		m_height = height;
		m_projDirty = true;
		m_viewDirty = true;
	}

	void Camera::setScale(Real scale) 
	{ 
		m_scale = scale; 
		m_projDirty = true;
		m_viewDirty = true;
	}

	void Camera::setNear(Real nearClip)
	{
		m_nearClip = nearClip;
		m_projDirty = true;
		m_viewDirty = true;
	}

	void Camera::setFar(Real farClip)
	{
		m_farClip = farClip;
		m_projDirty = true;
		m_viewDirty = true;
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
		Vector2 depthRange;
		Renderer::instance()->getDepthRange(depthRange);
		Vector3 v0(screenPos, depthRange.x);
		Vector3 v1(screenPos, depthRange.y);

		Renderer::instance()->unproject(ray.m_origin, v0, m_matVP);

		Vector3 target;
		Renderer::instance()->unproject(target, v1, m_matVP);
		ray.m_dir = target - ray.m_origin;
		ray.m_dir.normalize();
	}

	void Camera::update()
	{
		updateMatrix();
		updateFrustum();
	}

	void Camera::updateMatrix()
	{
		if (m_viewDirty)
		{
			Vector3 xAxis = m_right;	// right
			Vector3 yAxis = m_up;		// up axis
			Vector3 zAxis = -m_forward;	// forward

			m_matView = Matrix4(
				xAxis.x, yAxis.x, zAxis.x, 0,
				xAxis.y, yAxis.y, zAxis.y, 0,
				xAxis.z, yAxis.z, zAxis.z, 0,
				-Vector3::Dot(xAxis, m_position), -Vector3::Dot(yAxis, m_position), -Vector3::Dot(zAxis, m_position), 1
			);

			m_right = xAxis;

			Renderer::instance()->convertMatView(m_matView);
		}

		if (m_projDirty)
		{
			switch (m_projMode)
			{
			case ProjMode::PM_PERSPECTIVE:
			{
				float aspect = (Real)m_width / (Real)m_height;
				Matrix4::PerspectiveFovRH(m_matProj, m_fov, aspect, m_nearClip, m_farClip);
				Renderer::instance()->convertMatProj(m_matProj, m_matProj);
			}
			break;
			case ProjMode::PM_ORTHO:
			{
				Matrix4::OrthoRH(m_matProj, (Real)m_width * m_scale, (Real)m_height * m_scale, m_nearClip, m_farClip);
				Renderer::instance()->convertMatOrho(m_matProj, m_matProj, m_nearClip, m_farClip);
			} break;
			default:;
			}
		}

		if (m_viewDirty || m_projDirty)
		{
			m_matVP = m_matView * m_matProj;

			m_viewDirty = false;
			m_projDirty = false;
		}
	}

	void Camera::updateFrustum()
	{
		if (m_projMode == ProjMode::PM_PERSPECTIVE)
		{
			m_frustum.setPerspective(m_fov, m_width, m_height, m_nearClip, m_farClip);
			m_frustum.build(m_position, m_forward, m_up);
		}
		else
		{
			m_frustum.setOrtho(m_width * m_scale, m_height * m_scale, m_nearClip, m_farClip);
			m_frustum.build(m_position, m_forward, m_up);
		}
	}
}
