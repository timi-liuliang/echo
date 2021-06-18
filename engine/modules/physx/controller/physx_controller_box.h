#pragma once

#include "engine/core/scene/node.h"
#include "physx_controller.h"

namespace Echo
{
	class PhysxControllerBox : public PhysxController
	{
		ECHO_CLASS(PhysxControllerBox, PhysxController)

	public:
		PhysxControllerBox();
		virtual ~PhysxControllerBox();

	private:
	};
}
