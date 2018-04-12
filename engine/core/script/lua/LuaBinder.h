#pragma once

#include "engine/core/util/StringUtil.h"

extern "C"
{
#include <thirdparty/lua/lua.h>
#include <thirdparty/lua/lualib.h>
#include <thirdparty/lua/lauxlib.h>
}

namespace Echo
{
	class LuaBinder
	{
	public:
		// instance
		static LuaBinder* instance();

		// destroy
		static void destroy();

		// set state
		void init(lua_State* state);

	public:
		// exec script
		bool execString(const String& script, bool execute=true);

	public:
		// get global value
		bool getGlobalVariableBoolean(const String& varName);
		float getGlobalVariableFloat(const String& varName);
		double getGlobalVariableDouble(const String& varName);

	public:
		// output error and pop stack
		void outputError(int pop=0);

	private:
		LuaBinder() {}
		~LuaBinder() {}

	private:
		lua_State*		m_state;		// luaState
	};
}