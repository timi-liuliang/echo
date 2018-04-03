#pragma once

extern "C"
{
#include <thirdparty/lua/lua.h>
#include <thirdparty/lua/lualib.h>
#include <thirdparty/lua/lauxlib.h>
}

#define SCRIPT_CHECK(fun) if(fun) { outputError(m_state); }

#define _CHILL_LUA_CPP_PTR_NAME		"_lua_cpp_class_ptr__"

namespace Echo
{
	class _None_TagClass { public:  int _i; void _none_tagF(); };
	typedef void(_None_TagClass::*class_function)();

	template<typename ClassT>
	ClassT* _get_object_ptr_stack(lua_State* L, const int stackIndex)
	{
		lua_pushstring(L, _CHILL_LUA_CPP_PTR_NAME);
		lua_gettable(L, stackIndex);

		ClassT* obj = static_cast<ClassT*>(lua_touserdata(L, -1));

		lua_pop(L, 1);
		return obj;
	}

	template<typename ClassT>
	ClassT* _get_object_ptr(lua_State* L)
	{
		return _get_object_ptr_stack<ClassT>( L, 1);
	}

	template <typename ClassT>
	static int _callback_class_param0_returnVoid(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef void(ClassT::*FuncT)();
		FuncT func;// _get_class_function_ptr<FuncT>(L);

		// exec
		(obj->*func)();

		return 0;
	}

	class LuaBinder
	{
	public:
		LuaBinder();
		~LuaBinder();

		// get instance
		static LuaBinder* instance();

		// destory
		static void destroy();

	public:
		// load file
		bool loadFile(const String& file);

	public:
		// register class
		void registerClass(const String& className, const String& parentClassName);

		// register function
		template<typename ClassT>
		void registerFunction(const String& className, const String& funcName, void(ClassT::*func)())
		{
			int n = prepareRegisterClassFunction(className, funcName, reinterpret_cast<class_function*>(&func));
			lua_pushcclosure(m_state, &_callback_class_param0_returnVoid<ClassT>, n);
			postRegisterClassFunction();
		}

	public:
		// call without params
		void call(const String& funName)
		{
			if (checkStack())
			{
				lua_getglobal(m_state, funName.c_str());
				SCRIPT_CHECK(lua_pcall(m_state, 0, 0, 0));
			}
		}

	private:
		// out error
		void outputError(lua_State* state);

		// check stack
		bool checkStack();

		// prepare register class function
		int prepareRegisterClassFunction(const String& className, const String& funcName, class_function* cf);

		// post register class function
		void postRegisterClassFunction();

	private:
		lua_State*		m_state;		// lua state
	};
}