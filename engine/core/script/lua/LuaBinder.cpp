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

	bool LuaBinder::registerClass(const String& className, const char* parentClassName)
	{
		//create metatable for class
		luaL_newmetatable( m_state, className.c_str());
		const int metatable = lua_gettop(m_state);

		//change the metatable's __index to metatable itself;
		lua_pushliteral(m_state, "__index");
		lua_pushvalue(m_state, metatable);
		lua_settable(m_state, metatable);

		// inherits from parent class
		if (parentClassName && strlen(parentClassName))
		{
			// lookup metatable in Lua registry
			luaL_getmetatable(m_state, parentClassName);
			lua_setmetatable(m_state, metatable);
		}

		lua_pop(m_state, 1);

		return true;
	}

	bool LuaBinder::registerMethod(const String& className, const String& methodName, MethodBind* method)
	{
		int a = 10;

		return false;
	}

	bool LuaBinder::registerObject(const String& className, const String& objectName, void* obj)
	{
		int a = 10;
		//if ( className && strlen(className) && objectName && strlen(objectName) && obj)
		//{
		//	LUAEX_CHECK_BEGIN;

		//	string128 single_name;
		//	int parent_stack = _get_parent_table(object_name, single_name); //stack + 1

		//																	//解析object_name
		//	bool has_parent = parent_stack != 0;

		//	//新建object在lua中对应的table，新table创建后被自动放在stacktop，
		//	//然后保存新表格对象的stackindex，方便后面使用
		//	lua_newtable(L);											// stack + 1
		//	int si_newClassTable = lua_gettop(L);

		//	//设置对象指针
		//	lua_pushstring(L, _CHILL_LUA_CPP_PTR_NAME_);				// stack + 1
		//	lua_pushlightuserdata(L, pobj);								// stack + 1
		//	lua_settable(L, si_newClassTable);							// stack - 2

		//																//设置对象的metatable
		//	luaL_getmetatable(L, class_name);  // lookup metatable in Lua registry // stack + 1
		//	lua_setmetatable(L, si_newClassTable);						// stack - 1

		//																//将对象抛出给脚本
		//	if (has_parent)
		//	{
		//		lua_setfield(L, parent_stack, single_name.c_str());		//stack - 1
		//		lua_pop(L, 1); // pop the parent;						//stack - 1
		//	}
		//	else
		//		lua_setglobal(L, single_name.c_str());

		//	LUAEX_CHECK_END;
		//}

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

	// exec fun
	void LuaBinder::execFunc(const String& funName)
	{

	}

	// get global value(boolean)
	bool LuaBinder::getGlobalVariableBoolean(const String& varName)
	{
		lua_getglobal(m_state, varName.c_str());

	#ifdef ECHO_EDITOR_MODE
		if (lua_isnil(m_state, 1))	EchoLogError("Lua global variable [%s == nil]", varName.c_str());
	#endif

		bool result = lua_toboolean(m_state, 1)!=0;

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
		//打印错误结果 
		LogManager::instance()->error(lua_tostring(m_state, -1));
		lua_pop(m_state, 1);

		if (pop > 0)
			lua_pop(m_state, pop);
	}

}