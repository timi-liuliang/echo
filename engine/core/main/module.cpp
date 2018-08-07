#include "module.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/modules/register_module.cxx"

namespace Echo
{
	static vector<Module*>::type* g_modules = nullptr;

	void Module::addModule(Module* module)
	{
		if (!g_modules)
			g_modules = new vector<Module*>::type;

		g_modules->push_back(module);
	}

	void Module::registerAllTypes()
	{
		if (g_modules)
		{
			for (Module* module : *g_modules)
			{
				module->registerTypes();
			}
		}
	}

	// update all modules every frame(ms)
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
