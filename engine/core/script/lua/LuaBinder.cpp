#include "LuaBinder.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/util/PathUtil.h"

namespace Echo
{
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
	}

	// set search path
	void LuaBinder::setSearchPath(const String& path)
	{
		setGlobalVariableStr("package.path", path);

		execString("log:error(package.path)");
	}

	// add search path
	void LuaBinder::addSearchPath(const String& path)
	{
		String formatPath = path;
		PathUtil::FormatPath(formatPath, false);

		String curPath = getGlobalVariableStr("package.path");
		setGlobalVariableStr("package.path", StringUtil::Format("%s;%s?.lua", formatPath.c_str(), curPath.c_str()));

		execString("log:error(package.path)");
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

		String result = lua_tostring(m_state, vars.size());

		// clear stack
		lua_settop(m_state, 0);

		return result;
	}

	// set global value
	void LuaBinder::setGlobalVariableStr(const String& varName, const String& value)
	{
		StringArray vars = StringUtil::Split(varName, ".");

		lua_getglobal(m_state, vars[0].c_str());
		for (size_t idx = 1; idx < vars.size(); idx++)
			lua_getfield(m_state, -1, vars[idx].c_str());


	}

	void LuaBinder::outputError(int pop)
	{
		//´òÓ¡´íÎó½á¹û 
		const char* error = lua_tostring(m_state, -1);
		LogManager::instance()->error(error);

		lua_pop(m_state, 1);
		if (pop > 0)
			lua_pop(m_state, pop);
	}

}