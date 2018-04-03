#include "luaex.h"

namespace luaex
{
	LuaEx* LuaEx::instance()
	{
		static LuaEx* inst = new LuaEx(true);
		return inst;
	}

	void LuaEx::destroy()
	{

	}
}