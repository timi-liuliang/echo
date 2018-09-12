#pragma once

#include "lua_base.h"

namespace Echo
{
	class MethodBind;
	class ClassMethodBind;
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
		bool registerMethod(const String& methodName, MethodBind* method);

		// register
		bool registerClass(const String& className, const char* parentClassName);
		bool registerClassMethod(const String& className, const String& methodName, ClassMethodBind* method);
		bool registerObject(const String& className, const String& objectName, void* obj);

		// get class infos
		void getClassMethods(const String& className, StringArray& methods);

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