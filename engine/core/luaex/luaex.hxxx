
#pragma once

#include "lua_wrapper.h"
#include "scl/assert.h"

//#include "lua/lua.hpp"

namespace luaex {

////////////////////////////////////
//	LuaEx:
//		封装了3个函数
//		call				//用于C++调用没有返回值的脚本函数
//		callreturn			//用于C++调用有返回值的脚本函数
//		registerFunction	//用于注册任意类型的C++函数到脚本中
////////////////////////////////////
class LuaEx : public Lua
{
public:
	LuaEx() {}
	LuaEx(bool auto_init) { if (auto_init) init(); }
	LuaEx(lua_State* state) { L = state; m_auto_release = false; }
	virtual ~LuaEx() {}


public:
	void register_function(const char* const functionname, lua_CFunction f) { return Lua::register_function(functionname, f); }

public:
	//call with param 0 returnT
	template <typename ReturnT>
	ReturnT call_return(const char* const functionName)
	{
		_checkstack();

		_stack_getglobal(L, functionName); // function to be called

		int err = _stack_pcall(L, 0, 1, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		ReturnT r = _stack_tovalue<ReturnT>(L, -1);
		assert(_stack_gettop(L) >= 1 && "the lua function doesn't return any value!");
		_stack_pop(L, 1);

		_checkstack();
		return r;
	}
	//call with param 1 returnT
	template <typename ReturnT, typename Param1T>
	ReturnT call_return(const char* const functionName, Param1T p1)
	{
		_checkstack();

		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);

		int err = _stack_pcall(L, 1, 1, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		ReturnT r = _stack_tovalue<ReturnT>(L, -1);
		assert(_stack_gettop(L) >= 1 && "the lua function doesn't return any value!");
		_stack_pop(L, 1);

		_checkstack();
		return r;
	}
	//call with param 1 return void
	template <typename Param1T>
	void call(const char* const functionName, Param1T p1)
	{
		_checkstack();
		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);

		int err = _stack_pcall(L, 1, 0, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		_checkstack();
	}
	//call with param 2 returnT
	template <typename ReturnT, typename Param1T, typename Param2T>
	ReturnT call_return(const char* const functionName, Param1T p1, Param2T p2)
	{
		_checkstack();

		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);

		int err = _stack_pcall(L, 2, 1, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		ReturnT r = _stack_tovalue<ReturnT>(L, -1);
		assert(_stack_gettop(L) >= 1 && "the lua function doesn't return any value!");
		_stack_pop(L, 1);

		_checkstack();
		return r;
	}
	//call with param 2 return void
	template <typename Param1T, typename Param2T>
	void call(const char* const functionName, Param1T p1, Param2T p2)
	{
		_checkstack();
		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);

		int err = _stack_pcall(L, 2, 0, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		_checkstack();
	}
	//call with param 3 returnT
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T>
	ReturnT call_return(const char* const functionName, Param1T p1, Param2T p2, Param3T p3)
	{
		_checkstack();

		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);
		_stack_pushvalue(L, p3);

		int err = _stack_pcall(L, 3, 1, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		ReturnT r = _stack_tovalue<ReturnT>(L, -1);
		assert(_stack_gettop(L) >= 1 && "the lua function doesn't return any value!");
		_stack_pop(L, 1);

		_checkstack();
		return r;
	}
	//call with param 3 return void
	template <typename Param1T, typename Param2T, typename Param3T>
	void call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3)
	{
		_checkstack();
		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);
		_stack_pushvalue(L, p3);

		int err = _stack_pcall(L, 3, 0, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		_checkstack();
	}
	//call with param 4 returnT
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	ReturnT call_return(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4)
	{
		_checkstack();

		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);
		_stack_pushvalue(L, p3);
		_stack_pushvalue(L, p4);

		int err = _stack_pcall(L, 4, 1, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		ReturnT r = _stack_tovalue<ReturnT>(L, -1);
		assert(_stack_gettop(L) >= 1 && "the lua function doesn't return any value!");
		_stack_pop(L, 1);

		_checkstack();
		return r;
	}
	//call with param 4 return void
	template <typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	void call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4)
	{
		_checkstack();
		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);
		_stack_pushvalue(L, p3);
		_stack_pushvalue(L, p4);

		int err = _stack_pcall(L, 4, 0, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		_checkstack();
	}
	//call with param 5 returnT
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	ReturnT call_return(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5)
	{
		_checkstack();

		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);
		_stack_pushvalue(L, p3);
		_stack_pushvalue(L, p4);
		_stack_pushvalue(L, p5);

		int err = _stack_pcall(L, 5, 1, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		ReturnT r = _stack_tovalue<ReturnT>(L, -1);
		assert(_stack_gettop(L) >= 1 && "the lua function doesn't return any value!");
		_stack_pop(L, 1);

		_checkstack();
		return r;
	}
	//call with param 5 return void
	template <typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	void call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5)
	{
		_checkstack();
		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);
		_stack_pushvalue(L, p3);
		_stack_pushvalue(L, p4);
		_stack_pushvalue(L, p5);

		int err = _stack_pcall(L, 5, 0, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		_checkstack();
	}
	//call with param 6 returnT
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	ReturnT call_return(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6)
	{
		_checkstack();

		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);
		_stack_pushvalue(L, p3);
		_stack_pushvalue(L, p4);
		_stack_pushvalue(L, p5);
		_stack_pushvalue(L, p6);

		int err = _stack_pcall(L, 6, 1, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		ReturnT r = _stack_tovalue<ReturnT>(L, -1);
		assert(_stack_gettop(L) >= 1 && "the lua function doesn't return any value!");
		_stack_pop(L, 1);

		_checkstack();
		return r;
	}
	//call with param 6 return void
	template <typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	void call(const char* const functionName, Param1T p1, Param2T p2, Param3T p3, Param4T p4, Param5T p5, Param6T p6)
	{
		_checkstack();
		_stack_getglobal(L, functionName); // function to be called
		_stack_pushvalue(L, p1);
		_stack_pushvalue(L, p2);
		_stack_pushvalue(L, p3);
		_stack_pushvalue(L, p4);
		_stack_pushvalue(L, p5);
		_stack_pushvalue(L, p6);

		int err = _stack_pcall(L, 6, 0, 0);
		if (err)
		{
			const char* error = _stack_tostring(L, -1);
			//打印错误结果
			assertf(false, "%s", error);
			_stack_pop(L, 1);
		}

		_checkstack();
	}
public:
	////////////////////////////////////////////////////////////////////////
	//	param count 0
	////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT()
	template <typename ClassT, typename ReturnT>
	void register_function(const char* const className, const char* const funcName,  ReturnT (ClassT::*func)())
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param0_returnT<ClassT, ReturnT>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)()
	template <typename ClassT>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)())
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param0_returnVoid<ClassT>, n);
		_post_register_class_function();
	}

	//	ReturnT ClassT::FuncT() const
	template <typename ClassT, typename ReturnT>
	void register_function(const char* const className, const char* const funcName, ReturnT (ClassT::*func)() const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param0_returnT<ClassT, ReturnT>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)() const
	template <typename ClassT>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)() const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param0_returnVoid<ClassT>, n);
		_post_register_class_function();
	}

	//	ReturnT FuncT()
	template <typename ReturnT>
	void register_function(const char* const funcName, ReturnT (*func)())
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param0_returnT<ReturnT>, 1);
		_post_register_function(funcName);
	}

	//	void (*func)()
	void register_function(const char* const funcName, void (*func)())
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param0_returnVoid, 1);
		_post_register_function(funcName);
	}

	////////////////////////////////////////////////////////////////////////
	//	param count 1
	////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T)
	template <typename ClassT, typename ReturnT, typename Param1T>
	void register_function(const char* const className, const char* const funcName,  ReturnT (ClassT::*func)(Param1T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param1_returnT<ClassT, ReturnT, Param1T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T)
	template <typename ClassT, typename Param1T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param1_returnVoid<ClassT, Param1T>, n);
		_post_register_class_function();
	}

	//	ReturnT ClassT::FuncT(Param1T) const
	template <typename ClassT, typename ReturnT, typename Param1T>
	void register_function(const char* const className, const char* const funcName, ReturnT (ClassT::*func)(Param1T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param1_returnT<ClassT, ReturnT, Param1T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T) const
	template <typename ClassT, typename Param1T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param1_returnVoid<ClassT, Param1T>, n);
		_post_register_class_function();
	}

	//	ReturnT FuncT(Param1T)
	template <typename ReturnT, typename Param1T>
	void register_function(const char* const funcName, ReturnT (*func)(Param1T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param1_returnT<ReturnT, Param1T>, 1);
		_post_register_function(funcName);
	}

	//	void (*func)(Param1T)
	template <typename Param1T>
	void register_function(const char* const funcName, void (*func)(Param1T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param1_returnVoid<Param1T>, 1);
		_post_register_function(funcName);
	}

	////////////////////////////////////////////////////////////////////////
	//	param count 2
	////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T>
	void register_function(const char* const className, const char* const funcName,  ReturnT (ClassT::*func)(Param1T, Param2T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param2_returnT<ClassT, ReturnT, Param1T, Param2T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T)
	template <typename ClassT, typename Param1T, typename Param2T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param2_returnVoid<ClassT, Param1T, Param2T>, n);
		_post_register_class_function();
	}

	//	ReturnT ClassT::FuncT(Param1T, Param2T) const
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T>
	void register_function(const char* const className, const char* const funcName, ReturnT (ClassT::*func)(Param1T, Param2T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param2_returnT<ClassT, ReturnT, Param1T, Param2T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T) const
	template <typename ClassT, typename Param1T, typename Param2T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param2_returnVoid<ClassT, Param1T, Param2T>, n);
		_post_register_class_function();
	}

	//	ReturnT FuncT(Param1T, Param2T)
	template <typename ReturnT, typename Param1T, typename Param2T>
	void register_function(const char* const funcName, ReturnT (*func)(Param1T, Param2T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param2_returnT<ReturnT, Param1T, Param2T>, 1);
		_post_register_function(funcName);
	}

	//	void (*func)(Param1T, Param2T)
	template <typename Param1T, typename Param2T>
	void register_function(const char* const funcName, void (*func)(Param1T, Param2T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param2_returnVoid<Param1T, Param2T>, 1);
		_post_register_function(funcName);
	}

	////////////////////////////////////////////////////////////////////////
	//	param count 3
	////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T>
	void register_function(const char* const className, const char* const funcName,  ReturnT (ClassT::*func)(Param1T, Param2T, Param3T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param3_returnT<ClassT, ReturnT, Param1T, Param2T, Param3T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T, Param3T)
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T, Param3T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param3_returnVoid<ClassT, Param1T, Param2T, Param3T>, n);
		_post_register_class_function();
	}

	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T) const
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T>
	void register_function(const char* const className, const char* const funcName, ReturnT (ClassT::*func)(Param1T, Param2T, Param3T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param3_returnT<ClassT, ReturnT, Param1T, Param2T, Param3T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T, Param3T) const
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T, Param3T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param3_returnVoid<ClassT, Param1T, Param2T, Param3T>, n);
		_post_register_class_function();
	}

	//	ReturnT FuncT(Param1T, Param2T, Param3T)
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T>
	void register_function(const char* const funcName, ReturnT (*func)(Param1T, Param2T, Param3T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param3_returnT<ReturnT, Param1T, Param2T, Param3T>, 1);
		_post_register_function(funcName);
	}

	//	void (*func)(Param1T, Param2T, Param3T)
	template <typename Param1T, typename Param2T, typename Param3T>
	void register_function(const char* const funcName, void (*func)(Param1T, Param2T, Param3T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param3_returnVoid<Param1T, Param2T, Param3T>, 1);
		_post_register_function(funcName);
	}

	////////////////////////////////////////////////////////////////////////
	//	param count 4
	////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T, Param4T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	void register_function(const char* const className, const char* const funcName,  ReturnT (ClassT::*func)(Param1T, Param2T, Param3T, Param4T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param4_returnT<ClassT, ReturnT, Param1T, Param2T, Param3T, Param4T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T, Param3T, Param4T)
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T, Param3T, Param4T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param4_returnVoid<ClassT, Param1T, Param2T, Param3T, Param4T>, n);
		_post_register_class_function();
	}

	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T, Param4T) const
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	void register_function(const char* const className, const char* const funcName, ReturnT (ClassT::*func)(Param1T, Param2T, Param3T, Param4T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param4_returnT<ClassT, ReturnT, Param1T, Param2T, Param3T, Param4T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T, Param3T, Param4T) const
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T, Param3T, Param4T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param4_returnVoid<ClassT, Param1T, Param2T, Param3T, Param4T>, n);
		_post_register_class_function();
	}

	//	ReturnT FuncT(Param1T, Param2T, Param3T, Param4T)
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	void register_function(const char* const funcName, ReturnT (*func)(Param1T, Param2T, Param3T, Param4T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param4_returnT<ReturnT, Param1T, Param2T, Param3T, Param4T>, 1);
		_post_register_function(funcName);
	}

	//	void (*func)(Param1T, Param2T, Param3T, Param4T)
	template <typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	void register_function(const char* const funcName, void (*func)(Param1T, Param2T, Param3T, Param4T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param4_returnVoid<Param1T, Param2T, Param3T, Param4T>, 1);
		_post_register_function(funcName);
	}

	////////////////////////////////////////////////////////////////////////
	//	param count 5
	////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T, Param4T, Param5T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	void register_function(const char* const className, const char* const funcName,  ReturnT (ClassT::*func)(Param1T, Param2T, Param3T, Param4T, Param5T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param5_returnT<ClassT, ReturnT, Param1T, Param2T, Param3T, Param4T, Param5T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T)
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T, Param3T, Param4T, Param5T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param5_returnVoid<ClassT, Param1T, Param2T, Param3T, Param4T, Param5T>, n);
		_post_register_class_function();
	}

	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T, Param4T, Param5T) const
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	void register_function(const char* const className, const char* const funcName, ReturnT (ClassT::*func)(Param1T, Param2T, Param3T, Param4T, Param5T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param5_returnT<ClassT, ReturnT, Param1T, Param2T, Param3T, Param4T, Param5T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T) const
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T, Param3T, Param4T, Param5T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param5_returnVoid<ClassT, Param1T, Param2T, Param3T, Param4T, Param5T>, n);
		_post_register_class_function();
	}

	//	ReturnT FuncT(Param1T, Param2T, Param3T, Param4T, Param5T)
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	void register_function(const char* const funcName, ReturnT (*func)(Param1T, Param2T, Param3T, Param4T, Param5T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param5_returnT<ReturnT, Param1T, Param2T, Param3T, Param4T, Param5T>, 1);
		_post_register_function(funcName);
	}

	//	void (*func)(Param1T, Param2T, Param3T, Param4T, Param5T)
	template <typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	void register_function(const char* const funcName, void (*func)(Param1T, Param2T, Param3T, Param4T, Param5T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param5_returnVoid<Param1T, Param2T, Param3T, Param4T, Param5T>, 1);
		_post_register_function(funcName);
	}

	////////////////////////////////////////////////////////////////////////
	//	param count 6
	////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	void register_function(const char* const className, const char* const funcName,  ReturnT (ClassT::*func)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param6_returnT<ClassT, ReturnT, Param1T, Param2T, Param3T, Param4T, Param5T, Param6T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T)
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T))
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param6_returnVoid<ClassT, Param1T, Param2T, Param3T, Param4T, Param5T, Param6T>, n);
		_post_register_class_function();
	}

	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T) const
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	void register_function(const char* const className, const char* const funcName, ReturnT (ClassT::*func)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param6_returnT<ClassT, ReturnT, Param1T, Param2T, Param3T, Param4T, Param5T, Param6T>, n);
		_post_register_class_function();
	}

	//	void ClassT::FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T) const
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	void register_function(const char* const className, const char* const funcName, void (ClassT::*func)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T) const)
	{
		int n = _prepare_register_class_function(className, funcName, reinterpret_cast<scl::class_function*>(&func));
		_stack_pushcclosure(L, &_callback_class_param6_returnVoid<ClassT, Param1T, Param2T, Param3T, Param4T, Param5T, Param6T>, n);
		_post_register_class_function();
	}

	//	ReturnT FuncT(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T)
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	void register_function(const char* const funcName, ReturnT (*func)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param6_returnT<ReturnT, Param1T, Param2T, Param3T, Param4T, Param5T, Param6T>, 1);
		_post_register_function(funcName);
	}

	//	void (*func)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T)
	template <typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	void register_function(const char* const funcName, void (*func)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T))
	{
		_prepare_register_function(&func);
		_stack_pushcclosure(L, &_callback_param6_returnVoid<Param1T, Param2T, Param3T, Param4T, Param5T, Param6T>, 1);
		_post_register_function(funcName);
	}

private:


////////////////////////////////////////////////////////////////////////
//	param count 0
////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT()
	template <typename ClassT, typename ReturnT>
	static int _callback_class_param0_returnT(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef ReturnT (ClassT::*FuncT)();
		FuncT func = _get_class_function_ptr<FuncT>(L);
		
		//执行调用
		ReturnT result = (obj->*func)();
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void ClassT::FuncT()
	template <typename ClassT>
	static int _callback_class_param0_returnVoid(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef void (ClassT::*FuncT)();
		FuncT func = _get_class_function_ptr<FuncT>(L);
		
		//执行调用
		(obj->*func)();
		return 0;
	}
	//	ReturnT FuncT()
	template <typename ReturnT>
	static int _callback_param0_returnT(lua_State* L)
	{
		typedef ReturnT (*FuncT)();
		FuncT func = _get_function_ptr<FuncT>(L);
		
		//执行调用
		ReturnT result = (*func)();
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void FuncT()
		static int _callback_param0_returnVoid(lua_State* L)
	{
		typedef void (*FuncT)();
		FuncT func = _get_function_ptr<FuncT>(L);
		
		//执行调用
		(*func)();
		return 0;
	}


////////////////////////////////////////////////////////////////////////
//	param count 1
////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T)
	template <typename ClassT, typename ReturnT, typename Param1T>
	static int _callback_class_param1_returnT(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef ReturnT (ClassT::*FuncT)(Param1T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);

		//执行调用
		ReturnT result = (obj->*func)(param1);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void ClassT::FuncT(Param1T)
	template <typename ClassT, typename Param1T>
	static int _callback_class_param1_returnVoid(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef void (ClassT::*FuncT)(Param1T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);

		//执行调用
		(obj->*func)(param1);
		return 0;
	}
	//	ReturnT FuncT(Param1T)
	template <typename ReturnT, typename Param1T>
	static int _callback_param1_returnT(lua_State* L)
	{
		typedef ReturnT (*FuncT)(Param1T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);

		//执行调用
		ReturnT result = (*func)(param1);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void FuncT(Param1T)
	template <typename Param1T>
	static int _callback_param1_returnVoid(lua_State* L)
	{
		typedef void (*FuncT)(Param1T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);

		//执行调用
		(*func)(param1);
		return 0;
	}


////////////////////////////////////////////////////////////////////////
//	param count 2
////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T>
	static int _callback_class_param2_returnT(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef ReturnT (ClassT::*FuncT)(Param1T, Param2T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);

		//执行调用
		ReturnT result = (obj->*func)(param1, param2);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void ClassT::FuncT(Param1T, Param2T)
	template <typename ClassT, typename Param1T, typename Param2T>
	static int _callback_class_param2_returnVoid(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef void (ClassT::*FuncT)(Param1T, Param2T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);

		//执行调用
		(obj->*func)(param1, param2);
		return 0;
	}
	//	ReturnT FuncT(Param1T, Param2T)
	template <typename ReturnT, typename Param1T, typename Param2T>
	static int _callback_param2_returnT(lua_State* L)
	{
		typedef ReturnT (*FuncT)(Param1T, Param2T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);

		//执行调用
		ReturnT result = (*func)(param1, param2);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void FuncT(Param1T, Param2T)
	template <typename Param1T, typename Param2T>
	static int _callback_param2_returnVoid(lua_State* L)
	{
		typedef void (*FuncT)(Param1T, Param2T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);

		//执行调用
		(*func)(param1, param2);
		return 0;
	}


////////////////////////////////////////////////////////////////////////
//	param count 3
////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T>
	static int _callback_class_param3_returnT(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef ReturnT (ClassT::*FuncT)(Param1T, Param2T, Param3T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);
		Param3T param3 = _stack_tovalue<Param3T>(L, 4);

		//执行调用
		ReturnT result = (obj->*func)(param1, param2, param3);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void ClassT::FuncT(Param1T, Param2T, Param3T)
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T>
	static int _callback_class_param3_returnVoid(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef void (ClassT::*FuncT)(Param1T, Param2T, Param3T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);
		Param3T param3 = _stack_tovalue<Param3T>(L, 4);

		//执行调用
		(obj->*func)(param1, param2, param3);
		return 0;
	}
	//	ReturnT FuncT(Param1T, Param2T, Param3T)
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T>
	static int _callback_param3_returnT(lua_State* L)
	{
		typedef ReturnT (*FuncT)(Param1T, Param2T, Param3T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);
		Param3T param3 = _stack_tovalue<Param3T>(L, 3);

		//执行调用
		ReturnT result = (*func)(param1, param2, param3);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void FuncT(Param1T, Param2T, Param3T)
	template <typename Param1T, typename Param2T, typename Param3T>
	static int _callback_param3_returnVoid(lua_State* L)
	{
		typedef void (*FuncT)(Param1T, Param2T, Param3T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);
		Param3T param3 = _stack_tovalue<Param3T>(L, 3);

		//执行调用
		(*func)(param1, param2, param3);
		return 0;
	}


////////////////////////////////////////////////////////////////////////
//	param count 4
////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T, Param4T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	static int _callback_class_param4_returnT(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef ReturnT (ClassT::*FuncT)(Param1T, Param2T, Param3T, Param4T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);
		Param3T param3 = _stack_tovalue<Param3T>(L, 4);
		Param4T param4 = _stack_tovalue<Param4T>(L, 5);

		//执行调用
		ReturnT result = (obj->*func)(param1, param2, param3, param4);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void ClassT::FuncT(Param1T, Param2T, Param3T, Param4T)
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	static int _callback_class_param4_returnVoid(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef void (ClassT::*FuncT)(Param1T, Param2T, Param3T, Param4T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);
		Param3T param3 = _stack_tovalue<Param3T>(L, 4);
		Param4T param4 = _stack_tovalue<Param4T>(L, 5);

		//执行调用
		(obj->*func)(param1, param2, param3, param4);
		return 0;
	}
	//	ReturnT FuncT(Param1T, Param2T, Param3T, Param4T)
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	static int _callback_param4_returnT(lua_State* L)
	{
		typedef ReturnT (*FuncT)(Param1T, Param2T, Param3T, Param4T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);
		Param3T param3 = _stack_tovalue<Param3T>(L, 3);
		Param4T param4 = _stack_tovalue<Param4T>(L, 4);

		//执行调用
		ReturnT result = (*func)(param1, param2, param3, param4);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void FuncT(Param1T, Param2T, Param3T, Param4T)
	template <typename Param1T, typename Param2T, typename Param3T, typename Param4T>
	static int _callback_param4_returnVoid(lua_State* L)
	{
		typedef void (*FuncT)(Param1T, Param2T, Param3T, Param4T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);
		Param3T param3 = _stack_tovalue<Param3T>(L, 3);
		Param4T param4 = _stack_tovalue<Param4T>(L, 4);

		//执行调用
		(*func)(param1, param2, param3, param4);
		return 0;
	}


////////////////////////////////////////////////////////////////////////
//	param count 5
////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T, Param4T, Param5T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	static int _callback_class_param5_returnT(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef ReturnT (ClassT::*FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);
		Param3T param3 = _stack_tovalue<Param3T>(L, 4);
		Param4T param4 = _stack_tovalue<Param4T>(L, 5);
		Param5T param5 = _stack_tovalue<Param5T>(L, 6);

		//执行调用
		ReturnT result = (obj->*func)(param1, param2, param3, param4, param5);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void ClassT::FuncT(Param1T, Param2T, Param3T, Param4T, Param5T)
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	static int _callback_class_param5_returnVoid(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef void (ClassT::*FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);
		Param3T param3 = _stack_tovalue<Param3T>(L, 4);
		Param4T param4 = _stack_tovalue<Param4T>(L, 5);
		Param5T param5 = _stack_tovalue<Param5T>(L, 6);

		//执行调用
		(obj->*func)(param1, param2, param3, param4, param5);
		return 0;
	}
	//	ReturnT FuncT(Param1T, Param2T, Param3T, Param4T, Param5T)
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	static int _callback_param5_returnT(lua_State* L)
	{
		typedef ReturnT (*FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);
		Param3T param3 = _stack_tovalue<Param3T>(L, 3);
		Param4T param4 = _stack_tovalue<Param4T>(L, 4);
		Param5T param5 = _stack_tovalue<Param5T>(L, 5);

		//执行调用
		ReturnT result = (*func)(param1, param2, param3, param4, param5);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void FuncT(Param1T, Param2T, Param3T, Param4T, Param5T)
	template <typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
	static int _callback_param5_returnVoid(lua_State* L)
	{
		typedef void (*FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);
		Param3T param3 = _stack_tovalue<Param3T>(L, 3);
		Param4T param4 = _stack_tovalue<Param4T>(L, 4);
		Param5T param5 = _stack_tovalue<Param5T>(L, 5);

		//执行调用
		(*func)(param1, param2, param3, param4, param5);
		return 0;
	}


////////////////////////////////////////////////////////////////////////
//	param count 6
////////////////////////////////////////////////////////////////////////
	//	ReturnT ClassT::FuncT(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T)
	template <typename ClassT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	static int _callback_class_param6_returnT(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef ReturnT (ClassT::*FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);
		Param3T param3 = _stack_tovalue<Param3T>(L, 4);
		Param4T param4 = _stack_tovalue<Param4T>(L, 5);
		Param5T param5 = _stack_tovalue<Param5T>(L, 6);
		Param6T param6 = _stack_tovalue<Param6T>(L, 7);

		//执行调用
		ReturnT result = (obj->*func)(param1, param2, param3, param4, param5, param6);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void ClassT::FuncT(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T)
	template <typename ClassT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	static int _callback_class_param6_returnVoid(lua_State* L)
	{
		ClassT* obj = _get_object_ptr<ClassT>(L);

		typedef void (ClassT::*FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T);
		FuncT func = _get_class_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 2);
		Param2T param2 = _stack_tovalue<Param2T>(L, 3);
		Param3T param3 = _stack_tovalue<Param3T>(L, 4);
		Param4T param4 = _stack_tovalue<Param4T>(L, 5);
		Param5T param5 = _stack_tovalue<Param5T>(L, 6);
		Param6T param6 = _stack_tovalue<Param6T>(L, 7);

		//执行调用
		(obj->*func)(param1, param2, param3, param4, param5, param6);
		return 0;
	}
	//	ReturnT FuncT(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T)
	template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	static int _callback_param6_returnT(lua_State* L)
	{
		typedef ReturnT (*FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);
		Param3T param3 = _stack_tovalue<Param3T>(L, 3);
		Param4T param4 = _stack_tovalue<Param4T>(L, 4);
		Param5T param5 = _stack_tovalue<Param5T>(L, 5);
		Param6T param6 = _stack_tovalue<Param6T>(L, 6);

		//执行调用
		ReturnT result = (*func)(param1, param2, param3, param4, param5, param6);
		_stack_pushvalue(L, result);
		return 1;
	}
	//	void FuncT(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T)
	template <typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
	static int _callback_param6_returnVoid(lua_State* L)
	{
		typedef void (*FuncT)(Param1T, Param2T, Param3T, Param4T, Param5T, Param6T);
		FuncT func = _get_function_ptr<FuncT>(L);
		//获取参数
		Param1T param1 = _stack_tovalue<Param1T>(L, 1);
		Param2T param2 = _stack_tovalue<Param2T>(L, 2);
		Param3T param3 = _stack_tovalue<Param3T>(L, 3);
		Param4T param4 = _stack_tovalue<Param4T>(L, 4);
		Param5T param5 = _stack_tovalue<Param5T>(L, 5);
		Param6T param6 = _stack_tovalue<Param6T>(L, 6);

		//执行调用
		(*func)(param1, param2, param3, param4, param5, param6);
		return 0;
	}

};

} // namespace luaex


