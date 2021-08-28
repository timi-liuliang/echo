#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class TFLiteModule : public Module
	{
		ECHO_SINGLETON_CLASS(TFLiteModule, Module)

	public:
		// Debug draw option
		enum DebugDrawOption
		{
			None,
			Editor,
			Game,
			All
		};

	public:
		TFLiteModule();
		virtual ~TFLiteModule();

		// Instance
		static TFLiteModule* instance();

		// Register all types of the module
		virtual void registerTypes() override;

	private:
	};
}
