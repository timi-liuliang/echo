#include "engine/core/util/AssertX.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/io/DataStream.h"
#include "LuaBinder.h"

namespace Echo
{
	static LuaBinder* g_lua = nullptr;

	LuaBinder::LuaBinder()
	{
		m_state = luaL_newstate();

		// Make standard libraries available in the lua object
		luaL_openlibs(m_state);
	}

	LuaBinder::~LuaBinder()
	{
		if(m_state)
			lua_close(m_state);

		g_lua = nullptr;
	}

	// get instance
	LuaBinder* LuaBinder::instance()
	{
		if (!g_lua)
			g_lua = EchoNew(LuaBinder);

		return g_lua;
	}

	// destory
	void LuaBinder::destroy()
	{
		EchoSafeDelete(g_lua, LuaBinder);
	}

	// load file
	bool LuaBinder::loadFile(const String& file)
	{
		MemoryReader memReader( file);
		if (LUA_OK == luaL_loadbuffer(m_state, memReader.getData<const char*>(), memReader.getSize(), "name"))
		{
			if (true)
			{
				if (LUA_OK != lua_pcall(m_state, 0, LUA_MULTRET, 0))
				{
					outputError(m_state);
					return false;
				}
			}
		}
		else
		{
			outputError(m_state);
			return false;
		}

		return true;
	}

	// register class
	void LuaBinder::registerClass(const String& className, const String& parentClassName)
	{
		checkStack();

		// create metatable for class
		luaL_newmetatable(m_state, className.c_str());
		const int classMetatable = lua_gettop(m_state);

		// change the metatable's __index to metatable itself
		lua_pushliteral(m_state, "__index");
		lua_pushvalue(m_state, classMetatable);
		lua_settable(m_state, classMetatable);

		lua_pop(m_state, 1);

		checkStack();
	}

	// prepare register class function
	int LuaBinder::prepareRegisterClassFunction(const String& className, const String& funcName, class_function* cf)
	{
		checkStack();

		luaL_getmetatable(m_state, className.c_str());
		lua_pushstring(m_state, funcName.c_str());

		void* ptr = *reinterpret_cast<void**>(cf);
		lua_pushlightuserdata(m_state, ptr);

		return 1;
	}

	// post register class function
	void LuaBinder::postRegisterClassFunction()
	{
		lua_settable(m_state, 1);
		lua_pop(m_state, 1);

		checkStack();
	}

	// check stack
	bool LuaBinder::checkStack()
	{
		const int topIdx = lua_gettop(m_state);
		return topIdx == 0;
	}

	// out error
	void LuaBinder::outputError(lua_State* state)
	{
		const char* message = lua_tostring(state, -1);
		EchoLogError("lua : %s", message);
		lua_pop(state, 1);
	}
}