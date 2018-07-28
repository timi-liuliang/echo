#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	class Module
	{
	public:
		virtual ~Module() {}

		virtual void registerTypes()=0;

		virtual void update(float elapsedTime) {}

	public:
		static void addModule(Module* module);

		// register all module types
		static void registerAllTypes();

		// update all modules every frame(ms)
		static void updateAll(float elapsedTime);
	};

	template<typename T>
	struct ModuleRegister
	{
		ModuleRegister()
		{
			static T* module = new T;
			Echo::Module::addModule(module);
		}
	};
}

#define REGISTER_MODULE(T) static Echo::ModuleRegister<T> G_MODULE_##T##_REGISTER;