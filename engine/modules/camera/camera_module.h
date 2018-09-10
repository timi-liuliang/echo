#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class CameraModule : public Module
	{
	public:
		CameraModule();

		// register all types of the module
		virtual void registerTypes();
	};
}