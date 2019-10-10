#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class VideoModule : public Module
	{
	public:
		VideoModule();
		virtual ~VideoModule();

		// register all types of the module
		virtual void registerTypes();

		// update audio module
		virtual void update(float elapsedTime);
	};
}
