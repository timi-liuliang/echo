#include "LuaBinder.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/util/LogManager.h"

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

		bool result = lua_toboolean(m_state, 1);

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