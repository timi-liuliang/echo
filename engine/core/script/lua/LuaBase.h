#pragma once

#include "engine/core/math/Vector4.h"

extern "C"
{
#include <thirdparty/lua/lua.h>
#include <thirdparty/lua/lualib.h>
#include <thirdparty/lua/lauxlib.h>
}

namespace Echo
{
	// error message
	void lua_binder_error(const char* msg);

	// lua stack to value
	template<typename T> INLINE T lua_stack_to_value(lua_State* L, const int index)			{ lua_binder_error("lua stack to value error, unknow c type"); static T st = Variant(); return st; }
	template<> INLINE const char* lua_stack_to_value<const char*>(lua_State* state, int idx){ return lua_tostring(state, idx); }

	// lua operate
	int lua_get_tables(lua_State* luaState, const StringArray& objectNames, const int count);
	int lua_get_upper_tables(lua_State* luaState, const String& objectName, String& currentLayerName);
}