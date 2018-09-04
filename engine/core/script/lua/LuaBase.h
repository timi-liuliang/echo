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
	class Object;
	class Node;

	struct LuaStackCheck
	{
		int			m_elementNum;
		lua_State*	m_luaState;

		LuaStackCheck(lua_State* state)
		{
			m_luaState = state;
			m_elementNum = lua_gettop(state);
		}

		~LuaStackCheck()
		{
			EchoAssert(m_elementNum == lua_gettop(m_luaState));
		}
	};

#ifdef ECHO_EDITOR_MODE
#define LUA_STACK_CHECK(state) LuaStackCheck stackCheck(state)
#else
#define LUA_STACK_CHECK(state)
#endif 

	// log messages
	void lua_binder_warning(const char* msg);
	void lua_binder_error(const char* msg);
	void lua_get_obj_name(Object* obj, char* buffer, int len);
	void lua_get_node_name(Node* obj, char* buffer, int len);

	// lua stack to value
	template<typename T> INLINE T lua_getvalue(lua_State* L, int index)			
	{ 
		lua_binder_error("lua stack to value error, unknow c type"); 
		static T st = variant_cast<T>(Variant()); 
		return st; 
	}

	template<> INLINE const char* lua_getvalue<const char*>(lua_State* state, int idx)
	{ 
		return lua_tostring(state, idx); 
	}

	template<> INLINE i32 lua_getvalue<i32>(lua_State* state, int idx) 
	{ 
		return (i32)lua_tonumber(state, idx); 
	}

	template<> INLINE ui32 lua_getvalue<ui32>(lua_State* state, int idx) 
	{ 
		return (ui32)lua_tonumber(state, idx); 
	}
	
	template<> INLINE const Vector3& lua_getvalue<const Vector3&>(lua_State* state, int idx) 
	{
		static Vector3 result;
		lua_getfield(state, idx, "x");
		lua_getfield(state, idx, "y");
		lua_getfield(state, idx, "z");
		result.x = (float)lua_tonumber(state, idx+1);
		result.y = (float)lua_tonumber(state, idx+2);
		result.z = (float)lua_tonumber(state, idx+3);
		lua_pop(state, 3);
		return result; 
	}

	template<> INLINE Node* lua_getvalue<Node*>(lua_State* state, int idx)
	{
		LUA_STACK_CHECK(state);

		lua_getfield(state, idx, "this");
		Node* nodeptr = static_cast<Node*>(lua_touserdata(state, -1));
		lua_pop(state, 1);

		return nodeptr;
	}

	// lua operate
	int lua_get_tables(lua_State* luaState, const StringArray& objectNames, const int count);
	int lua_get_upper_tables(lua_State* luaState, const String& objectName, String& currentLayerName);

	// lua push vlaue to stack
	template<typename T> INLINE void lua_pushvalue(lua_State* L, T value) 
	{ 
		lua_binder_error("lua stack to value error, unknow c type"); 
	}

	template<> INLINE void lua_pushvalue<const char*>(lua_State* state, const char* value) 
	{ 
		lua_pushfstring(state, value); 
	}

	template<> INLINE void lua_pushvalue<bool>(lua_State* state, bool value) 
	{ 
		lua_pushboolean(state, value); 
	}

	template<> INLINE void lua_pushvalue<i32>(lua_State* state, i32 value) 
	{ 
		lua_pushinteger(state, value); 
	}

	template<> INLINE void lua_pushvalue<ui32>(lua_State* state, ui32 value) 
	{ 
		lua_pushinteger(state, value); 
	}

	template<> INLINE void lua_pushvalue<Object*>(lua_State* state, Object* value) 
	{
		if (value)
		{
			char obj_name[128] = "_";
			lua_get_obj_name(value, obj_name + 1, 127);

			lua_getglobal(state, "objs");
			lua_getfield(state, -1, obj_name);
			lua_remove(state, -2);
		}
		else
		{
			lua_pushnil(state);
		}
	}

	template<> INLINE void lua_pushvalue<Node*>(lua_State* state, Node* value)
	{
		if (value)
		{
			char obj_name[128] = "_";
			lua_get_node_name(value, obj_name + 1, 127);

			lua_getglobal(state, "objs");
			lua_getfield(state, -1, obj_name);
			lua_remove(state, -2);
		}
		else
		{
			lua_pushnil(state);
		}
	}
}