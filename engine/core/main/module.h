#pragma once

namespace Echo
{
	class Module
	{
	public:
		virtual ~Module() {}

        // register all types of this module
		virtual void registerTypes()=0;

        // update this module
		virtual void update(float elapsedTime) {}

	public:
		static void addModule(Module* module);

		// register all module types
		static void registerAllTypes();

		// get current register module name
		static const char* getCurrentRegisterModuleName();

		// update all modules every frame(ms)
		static void updateAll(float elapsedTime);

	public:
		// name
        void setName(const char* name);
		const char* getName() { return m_name; }

	protected:
		char		m_name[128];
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
