#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class NetModule : public Module
	{
		ECHO_SINGLETON_CLASS(NetModule, Module)

	public:
		NetModule();
		virtual ~NetModule();

		// instance
		static NetModule* instance();

		// register all types of the module
		virtual void registerTypes() override;

		// update audio module
		virtual void update(float elapsedTime) override;
	};
}
