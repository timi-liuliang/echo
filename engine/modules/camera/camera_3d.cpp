#include "camera_3d.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node_tree.h"

namespace Echo
{
	Camera3D::Camera3D()
		: m_near( 0.3f)
		, m_far( 150.f)
	{

	}

	Camera3D::~Camera3D()
	{

	}

	void Camera3D::bindMethods()
	{
		CLASS_BIND_METHOD(Camera3D, getNear);
		CLASS_BIND_METHOD(Camera3D, setNear);
		CLASS_BIND_METHOD(Camera3D, getFar);
		CLASS_BIND_METHOD(Camera3D, setFar);

		CLASS_REGISTER_PROPERTY(Camera3D, "Near", Variant::Type::Real, getNear, setNear);
		CLASS_REGISTER_PROPERTY(Camera3D, "Far", Variant::Type::Real, getFar, setFar);
	}

	void Camera3D::syncDataToCamera(Camera* camera)
	{
		Vector3 direction = getWorldOrientation() * Vector3::UNIT_Z;
		camera->setDirection(direction);
		camera->setPosition(getWorldPosition());
		camera->setNearClip(m_near);
		camera->setFarClip(m_far);
	}

	void Camera3D::updateInternal(float elapsedTime)
	{
		if (Engine::instance()->getConfig().m_isGame)
		{
			Camera* camera = NodeTree::instance()->get3dCamera();
			if (camera)
			{
				syncDataToCamera(camera);
			}
		}
	}
}
