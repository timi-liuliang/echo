#include "LuaBinder.h"
#include "engine/core/log/LogManager.h"

namespace Echo
{
	void register_core_to_lua()
	{
		// core library
		{
			// 1.utils
			String utils = ""\
			"utils = {}\n\n"\
			"function utils.append_table(a, b)\n"\
			"    for k, v in pairs(b) do\n"\
			"        a[k] = v\n"\
			"    end\n"\
			"end\n"\
			"\n"\
			"_Nodes = {}\n"\
			"\n"\
			"function _update_all_nodes()\n"\
			"    for k, v in pairs(_Nodes) do\n"\
			"        v:update()\n"\
			"    end\n"\
			"end\n"\
			"\n"\
			"";
			LuaBinder::instance()->execString(utils.c_str(), true);
		}

		// log
		{
			LuaBinder::instance()->registerClass("LogManager", nullptr);
			//luaEx->register_function<LogManager, const char*>("LogManager", "error", &LogManager::error);
			//luaEx->register_function<LogManager, const char*>("LogManager", "warning", &LogManager::warning);
			//luaEx->register_function<LogManager, const char*>("LogManager", "info", &LogManager::info);

			//luaEx->register_object("LogManager", "Log", LogManager::instance());
		}
	}
}