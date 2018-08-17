#pragma once

#include "LuaBase.h"

namespace Echo
{
	class MethodBind;
	class LuaBinder
	{
	public:
		// instance
		static LuaBinder* instance();

		// destroy
		static void destroy();

		// set state
		void init();

		// register
		bool registerClass(const String& className, const char* parentClassName);
		bool registerMethod(const String& className, const String& methodName, MethodBind* method);
		bool registerObject(const String& className, const String& objectName, void* obj);

		// def search path
		void setSearchPath(const String& path);
		void addSearchPath(const String& path);

		// add lua loader
		void addLoader(lua_CFunction func);

		// exec script
		bool execString(const String& script, bool execute=true);

		// exec fun
		void execFunc(const String& funName);

	public:
		// get global value
		bool getGlobalVariableBoolean(const String& varName);
		float getGlobalVariableFloat(const String& varName);
		double getGlobalVariableDouble(const String& varName);
		String getGlobalVariableStr(const String& varName);

		// set global value
		void setGlobalVariableStr(const String& varName, const String& value);
		void setTableKeyValue(const String& tableName, int key, lua_CFunction value);

	private:
		// output error and pop stack
		void outputError(int pop=0);

	private:
		LuaBinder() {}
		~LuaBinder() {}

	private:
		lua_State*		m_luaState;		// luaState
	};
}