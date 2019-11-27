#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	// implement by application or dll
	void registerModules();

	class Module : public Object
	{
		ECHO_VIRTUAL_CLASS(Module, Object);

	public:
		virtual ~Module() {}

        // register all types of this module
		virtual void registerTypes() {}

        // update this module
		virtual void update(float elapsedTime) {}

		// enable
		virtual void setEnable(bool isEnable) { m_isEnable = isEnable; }
		bool isEnable() const { return m_isEnable; }

	public:
        // add module by type
        template<typename T> static void addModule(const char* name);
        
        // add module
		static void addModule(Module* module);

		// register all module types
		static void registerAllTypes();

		// get current register module name
		static const char* getCurrentRegisterModuleName();

		// update all modules every frame(ms)
		static void updateAll(float elapsedTime);
        
        // clear all
        static void clear();

	protected:
		bool			m_isEnable = true;
	};
    
    // add module by type
    template<typename T> void Module::addModule(const char* name)
    {
		Class::registerType<T>();

		Echo::Module* module = ECHO_DOWN_CAST<Module*>(Echo::Class::create(name));
        module->setName(name);
        
        Echo::Module::addModule(module);
    }
}

#define DECLARE_MODULE(T) \
void LoadModule_##T() \
{ \
	Echo::Module::addModule<T>(#T); \
}

#define REGISTER_MODULE(T) \
extern void LoadModule_##T(); \
LoadModule_##T();	\
