#pragma once

#include "lua_base.h"

namespace Echo
{
	class MethodBind;
	class ClassMethodBind;
	class LuaBinder
	{
	public:
		~LuaBinder() {}

		// instance
		static LuaBinder* instance();

		// set state
		void init();
		
		// register
		bool registerMethod(const String& methodName, MethodBind* method);

		// register
		bool registerClass(const char* className, const char* parentClassName);
		bool registerClassMethod(const String& className, const String& methodName, ClassMethodBind* method);
		bool registerObject(const String& className, const String& objectName, void* obj);

		// get class infos
		void getClassMethods(const String& className, StringArray& methods);

		// def search path
		void setSearchPath(const String& path);
		void addSearchPath(const String& path);

		// add lua loader
		void addLoader(lua_CFunction func);

    public:
		// exec script directly
		bool execString(const String& script, bool execute=true);

		// call lua function with 0-10 parameters
		template<typename ReturnT> ReturnT call(const char* const functionName);

		// call lua function with 1 parameter
		template<typename ReturnT, typename Param1T> ReturnT call(const char* const functionName, Param1T p1);

		// call lua function with 2 parameter
		template<typename ReturnT, typename Param1T, typename Param2T> ReturnT call(const char* const functionName, Param1T p1, Param2T p2);

		// call lua function with 3 parameter
		template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T> ReturnT call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3);

		// call lua function with 4 parameter
		template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T> ReturnT call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4);

		// call lua function with 5 parameter
		template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T> ReturnT call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5);

		// call lua function with 6 parameter
		template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T> ReturnT call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6);

		// call lua function with 7 parameter
		template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T, typename Param7T> ReturnT call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6, Param7T p7);

		// call lua function with 8 parameter
		template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T, typename Param7T, typename Param8T> ReturnT call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6, Param7T p7, Param8T p8);

		// call lua function with 9 parameter
		template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T, typename Param7T, typename Param8T, typename Param9T> ReturnT call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6, Param7T p7, Param8T p8, Param9T p9);

		// call lua function with 10 parameter
		template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T, typename Param7T, typename Param8T, typename Param9T, typename Param10T> ReturnT call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6, Param7T p7, Param8T p8, Param9T p9, Param10T p10);

	public:
		// get global value
		bool getGlobalVariableBoolean(const String& varName);
		float getGlobalVariableFloat(const String& varName);
		double getGlobalVariableDouble(const String& varName);
		String getGlobalVariableStr(const String& varName);

		// set global value
		void setGlobalVariableStr(const String& varName, const String& value);

	private:
		// output error and pop stack
		void outputError(int pop=0);

		// parse name
		int parseName(char* name, char** result, const int resultSize, char* lastSeparator);

	private:
		LuaBinder() {}

	private:
		lua_State*		m_luaState;		// luaState
	};

	// call lua function with no parameter
	template<typename ReturnT> ReturnT LuaBinder::call(const char* const functionName)
	{
		LUA_STACK_CHECK(m_luaState);

		String currentLayerName;
		int upperTableCount = lua_get_upper_tables(m_luaState, functionName, currentLayerName);
		int parentIdx = lua_gettop(m_luaState);
		int narg = 0;

		if (upperTableCount > 0)
		{
			lua_getfield(m_luaState, parentIdx, currentLayerName.c_str());
		}
		else
		{
			lua_getglobal(m_luaState, currentLayerName.c_str());
		}

		// push self
		if (/*lastSeparator == ':'*/true)
		{
			lua_pushvalue(m_luaState, -2);
			narg++;
		}

		if (lua_pcall(m_luaState, narg, 0, 0) != 0)
		{
			outputError();
		}

		// clear stack
		lua_settop(m_luaState, 0);
	}

	// call lua function with one parameter
	template<typename ReturnT, typename Param1T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1)
	{

	}

	// call lua function with 2 parameter
	template<typename ReturnT, typename Param1T, typename Param2T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1, Param2T p2)
	{

	}

	// call lua function with 3 parameter
	template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3)
	{

	}

	// call lua function with 4 parameter
	template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4)
	{

	}

	// call lua function with 5 parameter
	template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5)
	{

	}

	// call lua function with 6 parameter
	template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6)
	{

	}

	// call lua function with 7 parameter
	template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T, typename Param7T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6, Param7T p7)
	{

	}

	// call lua function with 8 parameter
	template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T, typename Param7T, typename Param8T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6, Param7T p7, Param8T p8)
	{

	}

	// call lua function with 9 parameter
	template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T, typename Param7T, typename Param8T, typename Param9T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6, Param7T p7, Param8T p8, Param9T p9)
	{

	}

	// call lua function with 10 parameter
	template<typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T, typename Param7T, typename Param8T, typename Param9T, typename Param10T> ReturnT LuaBinder::call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6, Param7T p7, Param8T p8, Param9T p9, Param10T p10)
	{

	}
}
