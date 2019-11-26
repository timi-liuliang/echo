#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class UiModule : public Module
	{
		ECHO_SINGLETON_CLASS(UiModule, Module)

	public:
		UiModule();
		virtual ~UiModule();

		// instance
		static UiModule* instance();

		// register all types of the module
		virtual void registerTypes() override;

		// get font
	};
}
