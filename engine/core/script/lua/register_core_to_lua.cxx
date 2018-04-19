#include "luaex.h"
#include "LuaBinder.h"
#include "engine/core/util/LogManager.h"

namespace Echo
{
	void register_core_to_lua()
	{
		luaex::LuaEx* luaEx = luaex::LuaEx::instance();
		if (!luaEx)
			return;

		// core library
		{
			// 1.utils
			String utils = ""\
			"utils = {}\n\n"\
			"function utils.append_table(a, b)\n"\
			"    for k, v in pairs(b) do\n"\
			"        a[k] = v\n"\
			"    end\n"\
			"end\n";
			LuaBinder::instance()->execString(utils.c_str(), true);
		}

		// log
		{
			luaEx->register_class("LogManager");
			luaEx->register_function<LogManager, const char*>("LogManager", "error", &LogManager::error);
			luaEx->register_function<LogManager, const char*>("LogManager", "warning", &LogManager::warning);
			luaEx->register_function<LogManager, const char*>("LogManager", "info", &LogManager::info);

			luaEx->register_object("LogManager", "log", LogManager::instance());
		}
	}
}