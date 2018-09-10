#include "camera_3d.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node_tree.h"

namespace Echo
{
	Camera3D::Camera3D()
	{

	}

	Camera3D::~Camera3D()
	{

	}

	void Camera3D::bindMethods()
	{

	}

	// update self
	void Camera3D::update_self()
	{
		if (Engine::instance()->getConfig().m_isGame)
		{
			Camera* camera = NodeTree::instance()->get3dCamera();
			if (camera)
			{
				Vector3 direction = getWorldOrientation() * Vector3::UNIT_Z;
				camera->setDirection(direction);
				camera->setPosition(getWorldPosition());
			}
		}
	}
}