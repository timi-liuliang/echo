#include "luaex.h"
#include "engine/core/util/LogManager.h"

namespace Echo
{
	void register_core_to_lua()
	{
		luaex::LuaEx* luaEx = luaex::LuaEx::instance();
		if (!luaEx)
			return;

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