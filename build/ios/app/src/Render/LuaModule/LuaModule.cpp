#include "LuaModule.h"

#include <array>
#include "Frame/Helper.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

extern int tolua_Lord_open(lua_State* tolua_S);

int Lua_Loader(lua_State* L)
{
	LORD::String filename(luaL_checkstring(L, 1));
	size_t pos = filename.rfind(".lua");
	if (pos != LORD::String::npos)
	{
		filename = filename.substr(0, pos);
	}

	pos = filename.find_first_of(".");
	while (pos != LORD::String::npos)
	{
		filename.replace(pos, 1, "/");
		pos = filename.find_first_of(".");
	}
	filename.append(".lua");

	size_t codeBufferSize = 0;
	LORD::DataStream* pDataStream = LordResourceManager->openResource(filename);
	if (!pDataStream)
	{
		LordLogError("open resource %s failed", filename.c_str());
		return 0;
	}

	codeBufferSize = pDataStream->size();

	char* codeBuffer = (char*)LordMalloc(codeBufferSize + 1);

	pDataStream->read(codeBuffer, codeBufferSize);

	codeBuffer[codeBufferSize] = 0;

	LordSafeDelete(pDataStream, DataStream);

	LordLogInfo("require lua file %s!", filename.c_str());

	if (codeBuffer)
	{
		if (luaL_loadbuffer(L, (char*)codeBuffer, codeBufferSize, filename.c_str()) != 0)
		{
			LordLogError("error loading module %s from file %s :\n\t%s",
				lua_tostring(L, 1), filename.c_str(), lua_tostring(L, -1));
			assert(0);
		}

		LordSafeFree(codeBuffer);
	}
	else
	{
		LordLogError("can not get file data of %s", filename.c_str());
	}

	return 1;
}

int Lua_Error(lua_State* L)
{
	LORD::String sErrorDesc = lua_tostring(L, -1);
	LordLogError("%s\n", sErrorDesc.c_str());
	lua_pop(L, 1);

	LordLogError("===========================================");
	LordLogError("==============LuaTraceBack:================");

	if (luaL_dostring(L, "debug.traceback()"))
	{
		LORD::String szInfomation = lua_tostring(L, -1);
		LordLogError("%s\n", szInfomation.c_str());
		lua_pop(L, 1);
	}

	LordLogError("===========================================");

	return 1;
}

namespace Examples
{
	LuaModule::LuaModule()
		: m_pState(nullptr)
	{

	}

	LuaModule::~LuaModule()
	{

	}

	const LORD::String LuaModule::getHelp()
	{
		return LORD::String("LORD::LuaModule"); 
	}

	bool LuaModule::initialize()
	{
		assert(!m_pState);

		m_pState = lua_open();
		luaL_openlibs(m_pState);
		tolua_Lord_open(m_pState);
		//		tolua_EngineEx_open(m_pState);

		addLuaLoader(m_pState, Lua_Loader);

		doLua(m_pState, "lord_main.lua");

		return true; 
	}

	void LuaModule::destroy()
	{
		lua_close(m_pState);
		m_pState = nullptr; 
	}

	void LuaModule::tick(LORD::ui32 elapsedTime)
	{

	}

	void LuaModule::keyboardProc(LORD::ui32 keyChar, bool isKeyDown)
	{
	
	}

	void LuaModule::mouseLBProc(LORD::i32 xpos, LORD::i32 ypos)
	{

	}

	void LuaModule::mouseMoveProc(LORD::i32 xpos, LORD::i32 ypos)
	{

	}

	void LuaModule::mouseWhellProc(LORD::i32 param)
	{
		// m_cameraLookAt -= m_cameraForward * param;
	}

	void LuaModule::addLuaLoader(lua_State* L, lua_CFunction func)
	{
		if (!func) return;

		// stack content after the invoking of the function
		// get loader table
		lua_getglobal(L, "package");                                  /* L: package */
		lua_getfield(L, -1, "loaders");                               /* L: package, loaders */

		// insert loader into index 2
		lua_pushcfunction(L, func);                                   /* L: package, loaders, func */
		for (int i = lua_objlen(L, -2) + 1; i > 2; --i)
		{
			lua_rawgeti(L, -2, i - 1);                                /* L: package, loaders, func, function */
			// we call lua_rawgeti, so the loader table now is at -3
			lua_rawseti(L, -3, i);                                    /* L: package, loaders, func */
		}
		lua_rawseti(L, -2, 2);                                        /* L: package, loaders */

		// set loaders into package
		lua_setfield(L, -2, "loaders");                               /* L: package */

		lua_pop(L, 1);
	}

	bool LuaModule::doLua(lua_State* L, const LORD::String& fileName)
	{
		int nBase = lua_gettop(L);
		lua_pushcfunction(L, (lua_CFunction)&Lua_Error);
		LORD::i32 nErrFunIndex = lua_gettop(L);

		size_t codeBufferSize = 0;
		LORD::DataStream* pDataStream = LordResourceManager->openResource(fileName);
		if (!pDataStream)
		{
			LordLogError("open resource %s failed", fileName.c_str());
			return 0;
		}

		codeBufferSize = pDataStream->size();

		char* codeBuffer = (char*)LordMalloc(codeBufferSize + 1);

		pDataStream->read(codeBuffer, codeBufferSize);

		codeBuffer[codeBufferSize] = 0;

		if (codeBuffer)
		{
			if (luaL_loadbuffer(L, codeBuffer, codeBufferSize, fileName.c_str()) != 0)
			{
				LordLogError("error loading module %s from file [%s] :[%s]",
					lua_tostring(L, 1), fileName.c_str(), lua_tostring(L, -1));
				assert(0);
			}
		}
		else
		{
			LordLogError("can not get file data of %s", fileName.c_str());

			return false;
		}

		int nRetError = lua_pcall(L, 0, -1, nErrFunIndex);
		if (0 != nRetError)
		{
			assert(0 && "lua_pcall error!");
			lua_close(L);
			return 1;
		}
		lua_settop(L, nBase);

		LordSafeFree(codeBuffer);

		return true;
	}
}