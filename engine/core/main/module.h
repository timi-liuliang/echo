#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	// implement by application or dll
	void registerModules();

	class Module
	{
	public:
		virtual ~Module() {}

        // register all types of this module
		virtual void registerTypes()=0;

        // update this module
		virtual void update(float elapsedTime) {}

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

	public:
		// name
        void setName(const char* name);
		const char* getName() { return m_name; }

	protected:
		char		m_name[128];
	};
    
    // add module by type
    template<typename T> void Module::addModule(const char* name)
    {
        T* module = EchoNew(T);
        module->setName(name);
        
        Echo::Module::addModule(module);
    }
}

#define REGISTER_MODULE(T) Echo::Module::addModule<T>(#T);
