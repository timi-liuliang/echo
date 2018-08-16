#include "LuaBinder.h"
#include "engine/core/base/MethodBind.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/log/LogManager.h"
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

	static int cb( lua_State* L)
	{
		MethodBind* method = nullptr;

		return method ? method->call( L) : 0;
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
	void LuaBinder::init()
	{
		m_luaState = luaL_newstate();
		luaL_openlibs(m_luaState);

		addLoader(luaLoaderEcho);
		setSearchPath("User://");
	}

	// set search path
	void LuaBinder::setSearchPath(const String& path)
	{
		String x = getGlobalVariableStr("package.path");
		setGlobalVariableStr("package.path", StringUtil::Format("%s?.lua", path.c_str()).c_str());
	}

	bool LuaBinder::registerClass(const String& className, const char* parentClassName)
	{
		//create metatable for class
		luaL_newmetatable(m_luaState, className.c_str());
		const int metatable = lua_gettop(m_luaState);

		//change the metatable's __index to metatable itself;
		lua_pushliteral(m_luaState, "__index");
		lua_pushvalue(m_luaState, metatable);
		lua_settable(m_luaState, metatable);

		// inherits from parent class
		if (parentClassName && strlen(parentClassName))
		{
			// lookup metatable in Lua registry
			luaL_getmetatable(m_luaState, parentClassName);
			lua_setmetatable(m_luaState, metatable);
		}

		lua_pop(m_luaState, 1);

		return true;
	}

	bool LuaBinder::registerMethod(const String& className, const String& methodName, MethodBind* method)
	{
		luaL_getmetatable(m_luaState, className.c_str());
		lua_pushstring(m_luaState, methodName.c_str());     // table.methodName = method
		lua_pushlightuserdata(m_luaState, method);
		lua_pushcclosure(m_luaState, cb, 1);

		lua_settable(m_luaState, 1);
		lua_pop(m_luaState, 1);

		return false;
	}

	bool LuaBinder::registerObject(const String& className, const String& objectName, void* obj)
	{
		if ( !className.empty() && !objectName.empty() && obj)
		{
			lua_newtable(m_luaState);
			int objTableStatckIdx = lua_gettop(m_luaState);

			const char* objectPtrName = "c_ptr";
			lua_pushstring(m_luaState, objectPtrName);
			lua_pushlightuserdata(m_luaState, obj);
			lua_settable(m_luaState, objTableStatckIdx);

			luaL_getmetatable(m_luaState, className.c_str());
			lua_setmetatable(m_luaState, objTableStatckIdx);

			lua_setglobal(m_luaState, objectName.c_str());
		}

		return false;
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
			lua_getglobal(m_luaState, "package");
			lua_getfield(m_luaState, -1, "searchers");

			int b = lua_gettop(m_luaState);

			// << loader func
			for (int i = lua_rawlen(m_luaState, -1); i >= 1; --i)
			{
				lua_rawgeti(m_luaState, -1, i);
				lua_rawseti(m_luaState, -2, i+1);
			}

			lua_pushcfunction(m_luaState, func);
			lua_rawseti(m_luaState, -2, 1);
			lua_settop(m_luaState, 0);
		}
	}

	// exec string
	bool LuaBinder::execString(const String& script, bool execute)
	{
		if (!luaL_loadstring(m_luaState, script.c_str()))
		{
			if (execute)
			{
				if (!lua_pcall(m_luaState, 0, LUA_MULTRET, 0))
					return true;
			}
			else
			{
				lua_pop(m_luaState, 1);
				return true;
			}
		}

		outputError();
		return false;
	}

	// exec fun
	void LuaBinder::execFunc(const String& funName)
	{

	}

	// get global value(boolean)
	bool LuaBinder::getGlobalVariableBoolean(const String& varName)
	{
		lua_getglobal(m_luaState, varName.c_str());

	#ifdef ECHO_EDITOR_MODE
		if (lua_isnil(m_luaState, 1))	EchoLogError("Lua global variable [%s == nil]", varName.c_str());
	#endif

		bool result = lua_toboolean(m_luaState, 1)!=0;

		// clear stack
		lua_settop(m_luaState, 0);

		return result;
	}

	float LuaBinder::getGlobalVariableFloat(const String& varName)
	{
		return static_cast<float>(getGlobalVariableDouble(varName));
	}

	// get global value (float)
	double LuaBinder::getGlobalVariableDouble(const String& varName)
	{
		lua_getglobal(m_luaState, varName.c_str());

	#ifdef ECHO_EDITOR_MODE
		if (lua_isnil(m_luaState, 1))	EchoLogError("lua global variable [%s == nil]", varName.c_str());
	#endif

		double result = lua_tonumber(m_luaState, 1);

		// clear stack
		lua_settop(m_luaState, 0);

		return result;
	}

	String LuaBinder::getGlobalVariableStr(const String& varName)
	{
		StringArray vars = StringUtil::Split(varName, ".");

		lua_getglobal(m_luaState, vars[0].c_str());
		for (size_t idx = 1; idx < vars.size(); idx++)
			lua_getfield(m_luaState, -1, vars[idx].c_str());

#ifdef ECHO_EDITOR_MODE
		if (lua_isnil(m_luaState, vars.size())) EchoLogError("lua global variable [%s == nil]", varName.c_str());
#endif
		int a = lua_gettop(m_luaState);
		String result = lua_tostring(m_luaState, vars.size());
		int b = lua_gettop(m_luaState);

		// clear stack
		lua_settop(m_luaState, 0);

		return result;
	}

	// set global value
	void LuaBinder::setGlobalVariableStr(const String& varName, const String& value)
	{
		StringArray vars = StringUtil::Split(varName, ".");

		lua_getglobal(m_luaState, vars[0].c_str());
		for (size_t idx = 1; idx < vars.size()-1; idx++)
			lua_getfield(m_luaState, -1, vars[idx].c_str());

		lua_pushstring(m_luaState, value.c_str());

		lua_setfield(m_luaState, -2, vars.back().c_str());

		lua_settop(m_luaState, 0);
	}

	void LuaBinder::setTableKeyValue(const String& tableName, int key, lua_CFunction value)
	{
		StringArray vars = StringUtil::Split(tableName, ".");

		lua_getglobal(m_luaState, vars[0].c_str());
		for (size_t idx = 1; idx < vars.size(); idx++)
			lua_getfield(m_luaState, -1, vars[idx].c_str());

		int  a = lua_gettop(m_luaState);

		lua_pushcfunction(m_luaState, value);

		int  b = lua_gettop(m_luaState);
		lua_rawseti(m_luaState, -2, key);

		lua_settop(m_luaState, 0);
	}

	void LuaBinder::outputError(int pop)
	{
		//´òÓ¡´íÎó½á¹û 
		LogManager::instance()->error(lua_tostring(m_luaState, -1));
		lua_pop(m_luaState, 1);

		if (pop > 0)
			lua_pop(m_luaState, pop);
	}
}