#pragma once

#include "LuaBase.h"
#include "engine/core/log/Log.h"
#include "engine/core/base/object.h"
#include "engine/core/scene/node.h"

namespace Echo
{
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
		itoa(obj->getId(), buffer, 10);
	}

	void lua_get_node_name(Node* obj, char* buffer, int len)
	{
		itoa(obj->getId(), buffer, 10);
	}

	// get upper layer table
	int lua_get_upper_tables(lua_State* luaState, const String& objectName, String& currentLayerName)
	{
		StringArray names = StringUtil::Split(objectName, ".");
		if (names.size() > 1)
		{
			currentLayerName = names.back();
			return lua_get_tables(luaState, names, names.size() - 1);
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
			}
			else
			{
				lua_getfield(luaState, -1, objectNames[i].c_str());
			}

			tableCount++;
			if (lua_isnil(luaState, -1))
			{
				lua_pop(luaState, tableCount);
				return 0;
			}
		}

		return tableCount;
	}
}