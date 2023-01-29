#include "module.h"
#include "engine/core/base/object.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/io/io.h"

namespace Echo
{
	static Module*				  g_currentModule = nullptr;
	static vector<Module*>::type* g_modules = nullptr;

	void Module::bindMethods()
	{
		CLASS_BIND_METHOD(Module, setEnable);
		CLASS_BIND_METHOD(Module, isEnable);

		CLASS_REGISTER_PROPERTY(Module, "Enable", Variant::Type::Bool, isEnable, setEnable);
	}

	const String& Module::getResPath()
	{
		return StringUtil::BLANK;
	}

	vector<Module*>::type* Module::getAllModules()
	{
		return g_modules;
	}

	void Module::addModule(Module* module)
	{
		if (!g_modules)
			g_modules = new vector<Module*>::type;

		g_modules->push_back(module);
	}
    
    void Module::clear()
    {
        if(g_modules)
        {
            // clear modules
            for(Module* module : *g_modules)
                EchoSafeDelete( module, Module);
            
            g_modules->clear();
            
            // delete self
            delete g_modules;
            g_modules = nullptr;
        }
        
        g_currentModule = nullptr;
    }

	void Module::registerAllTypes()
	{
		registerModules();

	#ifdef ECHO_GAME_SOURCE
		registerGameModules();
	#endif

		if (g_modules)
		{
			for (Module* module : *g_modules)
			{
				g_currentModule = module;
				module->registerTypes();

				IO::instance()->addModuleResPath(module->getName(), module->getResPath());
			}
		}
	}

	const char* Module::getCurrentRegisterModuleName()
	{
		return g_currentModule ? g_currentModule->getName().c_str() : "";
	}

	void Module::updateAll(float elapsedTime)
	{
		if (g_modules)
		{
			for (Module* module : *g_modules)
			{
				module->update(elapsedTime);
			}
		}
	}
}
