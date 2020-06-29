#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class AudioModule : public Module
	{
		ECHO_SINGLETON_CLASS(AudioModule, Module)

	public:
		AudioModule();
		virtual ~AudioModule();

		// instance
		static AudioModule* instance();

		// register all types of the module
		virtual void registerTypes() override;

		// update audio module
		virtual void update(float elapsedTime) override;
	};
}
