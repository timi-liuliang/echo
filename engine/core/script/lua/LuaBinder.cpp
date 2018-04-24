#include "LuaBinder.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/DataStream.h"

namespace Echo
{
	int luaLoaderEcho(lua_State* L)
	{
		String fileName(luaL_checkstring(L, 1));
		fileName = StringUtil::Replace(fileName, ".", "/");
		fileName = "Res://" + fileName + ".lua";

		MemoryReader memReader(fileName);
		if (memReader.getSize())
		{
			int loader = luaL_loadbuffer(L, memReader.getData<char*>(), memReader.getSize(), fileName.c_str());
			if (loader)
			{
				int top = lua_gettop(L);
				EchoLogError("lua load [%s] failed", lua_tostring(L, -1));
				lua_settop(L, top);
				return loader;
			}
		}

		return 1;
	}

	// instance
	LuaBinder* LuaBinder::instance()
	{
		static LuaBinder* binder = EchoNew(LuaBinder);

		return binder;
	}

	// destroy
	void LuaBinder::destroy()
	{
		LuaBinder* inst = instance();
		EchoSafeDelete(inst, LuaBinder);
	}

	// set state
	void LuaBinder::init(lua_State* state)
	{
		m_state = state;

		addLoader(luaLoaderEcho);
		setSearchPath("User://");
	}

	// set search path
	void LuaBinder::setSearchPath(const String& path)
	{
		String x = getGlobalVariableStr("package.path");
		setGlobalVariableStr("package.path", StringUtil::Format("%s?.lua", path.c_str()).c_str());
	}

	// add search path
	void LuaBinder::addSearchPath(const String& path)
	{
		String curPath = getGlobalVariableStr("package.path");
		setGlobalVariableStr("package.path", StringUtil::Format("%s?.lua;%s", path.c_str(), curPath.c_str()));
	}

	// add lua loader
	void LuaBinder::addLoader(lua_CFunction func)
	{
		if (func)
		{
			lua_getglobal(m_state, "package");
			lua_getfield(m_state, -1, "searchers");

			int b = lua_gettop(m_state);

			// << loader func
			for (int i = lua_rawlen(m_state, -1); i >= 1; --i)
			{
				lua_rawgeti(m_state, -1, i);
				lua_rawseti(m_state, -2, i+1);
			}

			lua_pushcfunction(m_state, func);
			lua_rawseti(m_state, -2, 1);
			lua_settop(m_state, 0);
		}
	}

	// exec string
	bool LuaBinder::execString(const String& script, bool execute)
	{
		if (!luaL_loadstring(m_state, script.c_str()))
		{
			if (execute)
			{
				if (!lua_pcall(m_state, 0, LUA_MULTRET, 0))
					return true;
			}
			else
			{
				lua_pop(m_state, 1);
				return true;
			}
		}

		outputError();
		return false;
	}

	// get global value(boolean)
	bool LuaBinder::getGlobalVariableBoolean(const String& varName)
	{
		lua_getglobal(m_state, varName.c_str());

	#ifdef ECHO_EDITOR_MODE
		if (lua_isnil(m_state, 1))	EchoLogError("Lua global variable [%s == nil]", varName.c_str());
	#endif

		bool result = (bool)(lua_toboolean(m_state, 1));

		// clear stack
		lua_settop(m_state, 0);

		return result;
	}

	float LuaBinder::getGlobalVariableFloat(const String& varName)
	{
		return static_cast<float>(getGlobalVariableDouble(varName));
	}

	// get global value (float)
	double LuaBinder::getGlobalVariableDouble(const String& varName)
	{
		lua_getglobal(m_state, varName.c_str());

	#ifdef ECHO_EDITOR_MODE
		if (lua_isnil(m_state, 1))	EchoLogError("lua global variable [%s == nil]", varName.c_str());
	#endif

		double result = lua_tonumber(m_state, 1);

		// clear stack
		lua_settop(m_state, 0);

		return result;
	}

	String LuaBinder::getGlobalVariableStr(const String& varName)
	{
		StringArray vars = StringUtil::Split(varName, ".");

		lua_getglobal(m_state, vars[0].c_str());
		for (size_t idx = 1; idx < vars.size(); idx++)
			lua_getfield(m_state, -1, vars[idx].c_str());

#ifdef ECHO_EDITOR_MODE
		if (lua_isnil(m_state, vars.size())) EchoLogError("lua global variable [%s == nil]", varName.c_str());
#endif
		int a = lua_gettop(m_state);
		String result = lua_tostring(m_state, vars.size());
		int b = lua_gettop(m_state);

		// clear stack
		lua_settop(m_state, 0);

		return result;
	}

	// set global value
	void LuaBinder::setGlobalVariableStr(const String& varName, const String& value)
	{
		StringArray vars = StringUtil::Split(varName, ".");

		lua_getglobal(m_state, vars[0].c_str());
		for (size_t idx = 1; idx < vars.size()-1; idx++)
			lua_getfield(m_state, -1, vars[idx].c_str());

		lua_pushstring(m_state, value.c_str());

		lua_setfield(m_state, -2, vars.back().c_str());

		lua_settop(m_state, 0);
	}

	void LuaBinder::setTableKeyValue(const String& tableName, int key, lua_CFunction value)
	{
		StringArray vars = StringUtil::Split(tableName, ".");

		lua_getglobal(m_state, vars[0].c_str());
		for (size_t idx = 1; idx < vars.size(); idx++)
			lua_getfield(m_state, -1, vars[idx].c_str());

		int  a = lua_gettop(m_state);

		lua_pushcfunction(m_state, value);

		int  b = lua_gettop(m_state);
		lua_rawseti(m_state, -2, key);

		lua_settop(m_state, 0);
	}

	void LuaBinder::outputError(int pop)
	{
		//´òÓ¡´íÎó½á¹û 
		LogManager::instance()->error(lua_tostring(m_state, -1));
		lua_pop(m_state, 1);

		if (pop > 0)
			lua_pop(m_state, pop);
	}

}