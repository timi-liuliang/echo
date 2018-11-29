#include "lua_base.h"
#include "engine/core/log/Log.h"
#include "engine/core/base/object.h"
#include "engine/core/scene/node.h"

namespace Echo
{
	ObjectPool<Vector3>		LuaVec3Pool = ObjectPool<Vector3>(32);
	ObjectPool<String>		LuaStrPool  = ObjectPool<String>(32);
	ObjectPool<RealVector>	LuaRealVectorPool = ObjectPool<RealVector>(32);
	ObjectPool<Matrix>		LuaMatrixPool = ObjectPool<Matrix>(32);

	void lua_binder_warning(const char* msg)
	{
		Log::instance()->warning(msg);
	}

	void lua_binder_error(const char* msg)
	{
		Log::instance()->error(msg);
	}

	void lua_get_obj_name(Object* obj, char* buffer, int len)
	{
        sprintf(buffer, "%d", obj->getId());
	}

	void lua_get_node_name(Node* obj, char* buffer, int len)
	{
        sprintf(buffer, "%d", obj->getId());
	}

	int lua_get_upper_tables(lua_State* luaState, const String& objectName, String& currentLayerName)
	{
		StringArray names = StringUtil::Split(objectName, ".");
		if (names.size() > 1)
		{
			currentLayerName = names.back();
			return lua_get_tables(luaState, names, static_cast<int>(names.size()) - 1);
		}
		else
		{
			return 0;
		}
	}

	int lua_get_tables(lua_State* luaState, const StringArray& objectNames, const int count)
	{
		int tableCount = 0;
		for (int i = 0; i < count; i++)
		{
			if (i == 0)
			{
				lua_getglobal(luaState, objectNames[i].c_str());
				if (lua_isnil(luaState, -1))
				{
					LUA_STACK_CHECK( luaState);
					lua_pop( luaState, 1);
					lua_newtable(luaState);
					lua_setglobal(luaState, objectNames[i].c_str());
					lua_getglobal(luaState, objectNames[i].c_str());
				}
			}
			else
			{
				lua_getfield(luaState, -1, objectNames[i].c_str());
				if (lua_isnil(luaState, -1))
				{
					LUA_STACK_CHECK(luaState);
					lua_pop(luaState, 1);
					lua_newtable(luaState);
					lua_setfield(luaState, -2, objectNames[i].c_str());
					lua_getfield(luaState, -1, objectNames[i].c_str());
				}
			}

			tableCount++;
		}

		return tableCount;
	}
}
