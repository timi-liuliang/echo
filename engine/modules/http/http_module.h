#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class HttpModule : public Module
	{
		ECHO_SINGLETON_CLASS(HttpModule, Module)

	public:
		HttpModule();
		virtual ~HttpModule();

		// instance
		static HttpModule* instance();

		// register all types of the module
		virtual void registerTypes() override;

		// update audio module
		virtual void update(float elapsedTime) override;
	};
}
