#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class VideoModule : public Module
	{
		ECHO_SINGLETON_CLASS(VideoModule, Module)

	public:
		VideoModule();
		virtual ~VideoModule();

		// instance
		static VideoModule* instance();

		// register all types of the module
		virtual void registerTypes();

		// update audio module
		virtual void update(float elapsedTime);
	};
}
