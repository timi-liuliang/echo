#include "camera_2d.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node_tree.h"

namespace Echo
{
	Camera2D::Camera2D()
	{

	}

	Camera2D::~Camera2D()
	{

	}

	void Camera2D::bindMethods()
	{

	}

	// update self
	void Camera2D::update_self()
	{
		if (Engine::instance()->getConfig().m_isGame)
		{
			Camera* camera = NodeTree::instance()->get2dCamera();
			if (camera)
			{
				// position
				Vector3 position = getWorldPosition();
				position.z = camera->getPosition().z;
				camera->setPosition( position);

				// rotation
				Vector3 up = getWorldOrientation() * Vector3::UNIT_Y;
				camera->setUp(up);
			}
		}
	}
}