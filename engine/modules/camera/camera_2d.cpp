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

	// sync data to camera
	void Camera2D::syncDataToCamera(Camera* camera)
	{
		// position
		camera->setPosition(getWorldPosition());

		// rotation
		Vector3 up = getWorldOrientation() * Vector3::UNIT_Y;
		camera->setUp(up);

		camera->update();
	}

	// update self
	void Camera2D::update_self()
	{
		if (Engine::instance()->getConfig().m_isGame)
		{
			Camera* camera = NodeTree::instance()->get2dCamera();
			if (camera)
				syncDataToCamera(camera);
		}
	}
}