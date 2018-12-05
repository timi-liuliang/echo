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

		// get current register module name
		static const String& getCurrentRegisterModuleName();

		// update all modules every frame(ms)
		static void updateAll(float elapsedTime);

	public:
		// name
		void setName(const char* name) { m_name = name; }
		const String& getName() { return m_name; }

	protected:
		String		m_name;
	};

	template<typename T>
	struct ModuleRegister
	{
		ModuleRegister(const char* name)
		{
			static T* module = new T;
			module->setName(name);

			Echo::Module::addModule(module);
		}
	};
}

#define REGISTER_MODULE(T) static Echo::ModuleRegister<T> G_MODULE_##T##_REGISTER(#T);