#include "engine/core/util/AssertX.h"
#include "LuaBind.h"

extern "C"
{
#include <thirdparty/lua/lua.h>
#include <thirdparty/lua/lualib.h>
#include <thirdparty/lua/lauxlib.h>
}

namespace Echo
{
	static LuaBind* g_lua = nullptr;

	LuaBind::LuaBind()
	{
		lua_State* state = luaL_newstate();

		// Make standard libraries available in the lua object
		luaL_openlibs(state);

		//int result = luaL_loadbuffer();

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
}