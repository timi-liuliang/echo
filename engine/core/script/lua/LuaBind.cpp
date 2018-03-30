#include "engine/core/util/AssertX.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/io/DataStream.h"
#include "LuaBind.h"

extern "C"
{
#include <thirdparty/lua/lua.h>
#include <thirdparty/lua/lualib.h>
#include <thirdparty/lua/lauxlib.h>
}

int howdy(lua_State* state)
{
	int numArgs = lua_gettop(state);
	for (int n = 1; n <= numArgs; n++)
	{
		Echo::String a = lua_tostring(state, n);
		int b = 10;
	}

	//lua_CFunction()

	lua_pushnumber(state, 123);
	lua_pushnumber(state, 456);

	// number of results 
	return 2;
}

namespace Echo
{
	static LuaBind* g_lua = nullptr;

	LuaBind::LuaBind()
	{
		lua_State* state = luaL_newstate();

		// Make standard libraries available in the lua object
		luaL_openlibs(state);

		lua_register(state, "hao123", howdy);

		MemoryReader memReader("Res://lua/hello_world.lua");

		if (LUA_OK == luaL_loadbuffer(state, memReader.getData<const char*>(), memReader.getSize(), "name"))
		{
			if (LUA_OK != lua_pcall(state, 0, LUA_MULTRET, 0))
			{
				outputError(state);
			}
		}
		else
		{
			outputError(state);
		}

		lua_close(state);

		int a = 10;
	}

	LuaBind::~LuaBind()
	{
		g_lua = nullptr;
	}

	// get instance
	LuaBind* LuaBind::instance()
	{
		if (!g_lua)
			g_lua = EchoNew(LuaBind);

		return g_lua;
	}

	// destory
	void LuaBind::destroy()
	{
		EchoSafeDelete(g_lua, LuaBind);
	}

	// out error
	void LuaBind::outputError(lua_State* state)
	{
		const char* message = lua_tostring(state, -1);
		EchoLogError("lua : %s", message);
		lua_pop(state, 1);
	}
}