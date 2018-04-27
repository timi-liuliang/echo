#pragma once

namespace Echo
{
	class Module
	{
	public:
		virtual ~Module() {}

		virtual void registerTypes()=0;

	public:
		static void addModule(Module* module);

		// register all module types
		static void registerAllTypes();
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