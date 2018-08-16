#pragma once

#include "scl/assert.h"
#include "scl/type.h"
#include "scl/ptr.h"
#include "scl/string.h"

#include <stdarg.h>
#include <stddef.h>
#include <string.h>

extern "C"
{
#include <thirdparty/lua/lua.h>
#include <thirdparty/lua/lualib.h>
#include <thirdparty/lua/lauxlib.h>
}

#if LUA_VERSION_NUM <= 501
#define LUAEX_51
#endif

struct	lua_State;
typedef int		(*lua_CFunction)	(lua_State *L);
typedef void*	(*lua_Alloc)		(void *ud, void *ptr, size_t osize, size_t nsize);

#define _CHILL_LUA_CPP_PTR_NAME_		"_luaex__cpp_class_ptr__"
#define _CHILL_NIL_NEWINDEX_FUNCTION	"_luaex__nil_newindex_func__"

namespace luaex {

struct TestVector { int x; int y; };

int				luaState_getUpValueIndex(const int i);
void			_stack_pushcclosure		(lua_State*	L, lua_CFunction fn, int n);
void			_stack_pushstring		(lua_State*	L, const char *s);
void			_stack_pushinteger		(lua_State *L, int n);
void			_stack_pushnumber		(lua_State *L, double n);
void			_stack_pushboolean		(lua_State *L, int b);
void			_stack_pushlightuserdata(lua_State *L, void* p);
void*			_stack_touserdata		(lua_State*	L, int index);
int				_stack_tointeger		(lua_State *L, int index);
double			_stack_tonumber			(lua_State *L, int index);
bool			_stack_toboolean		(lua_State *L, int index);
const char*		_stack_tostring			(lua_State *L, int index);
bool			_stack_isstring			(lua_State *L, int index);
bool			_stack_isnumber			(lua_State *L, int index);
//bool			_stack_isinteger		(lua_State *L, int index);
bool			_stack_isfunction		(lua_State *L, int index);
bool			_stack_isuserdata		(lua_State *L, int index);
int				_stack_gettop			(lua_State *L);
void 			_stack_pop				(lua_State*	L, int index);
void			_stack_remove			(lua_State*	L, int index);
void 			_stack_gettable			(lua_State*	L, int index);
void			_stack_getfield			(lua_State *L, int index, const char *k);
void			_stack_setfield			(lua_State *L, int index, const char *k);
bool			_stack_isnil			(lua_State *L, int index);
void			_stack_getglobal		(lua_State*	L, const char* name);
int				_stack_pcall			(lua_State* L, int n, int r, int f);

//overload _stack_push
inline void		_stack_pushvalue		(lua_State* L, int v		)	{ _stack_pushinteger(L, v);						}
inline void		_stack_pushvalue		(lua_State* L, uint v		)	{ _stack_pushnumber(L, v);						}
inline void		_stack_pushvalue		(lua_State* L, float v		)	{ _stack_pushnumber(L, v);						}
inline void		_stack_pushvalue		(lua_State* L, double v		)	{ _stack_pushnumber(L, v);						}
inline void		_stack_pushvalue		(lua_State* L, const char* v)	{ _stack_pushstring(L, v);						}
inline void		_stack_pushvalue		(lua_State* L, bool v		)	{ _stack_pushboolean(L, v);						}
inline void		_stack_pushvalue		(lua_State* L, TestVector& v)	{ _stack_pushlightuserdata(L, (void*)&v);		}

//overload _stack_tovalue
template <typename T> static inline T	_stack_tovalue				(lua_State* L, const int index) { assert(false && "invalid lua function type!"); return T(); }
template<> inline int					_stack_tovalue<int>			(lua_State* L, const int index) { return _stack_tointeger(L, index); }
template<> inline uint					_stack_tovalue<uint>		(lua_State* L, const int index) { return static_cast<unsigned int>(_stack_tonumber(L, index)); }
template<> inline float					_stack_tovalue<float>		(lua_State* L, const int index) { return static_cast<float>(_stack_tonumber(L, index)); }
template<> inline double 				_stack_tovalue<double>		(lua_State* L, const int index) { return _stack_tonumber(L, index); }
template<> inline bool					_stack_tovalue<bool>		(lua_State* L, const int index) { return _stack_toboolean(L, index); }
template<> inline const char*			_stack_tovalue<const char*>	(lua_State* L, const int index) { return _stack_tostring(L, index); }
template<> inline lua_State*			_stack_tovalue<lua_State*>	(lua_State* , const int		)	{ assert(false && "when using standard lua functions, use class Lua's registerLuaFunction instead of Luaex's registerFunction!"); return NULL; }
template<> inline TestVector&			_stack_tovalue<TestVector&>	(lua_State* L, const int index) { return *static_cast<TestVector*>(_stack_touserdata(L, index)); }

//for c++ class object and function
template <typename ClassT	> ClassT*	_get_object_ptr				(lua_State* L);	//	用于在lua回调函数中获得对象指针
template <typename ClassT	> ClassT*	_get_object_ptr_stack		(lua_State* L, const int stackIndex);	//	用于在lua回调函数中获得对象指针
template <typename FuncT	> FuncT		_get_function_ptr			(lua_State* L); //	用于在lua回调函数中获得函数指针
template <typename FuncT	> FuncT		_get_class_function_ptr		(lua_State* L); //	用于在lua回调函数中获得函数指针
template <typename ClassT	> int		_callback_class_lua_standard(lua_State* L); //	int ClassT::FuncT(lua_State* L)

class Lua
{
public:
	Lua();
	virtual ~Lua();

	bool			init							(lua_Alloc f = NULL);
	void			set_alloc						(lua_Alloc f);
	void			release							();

	bool			loadfile						(const char* const file, bool execute = true , const char* const environment = NULL, bool see_all = false);
	bool			loadstring						(const char* const script, bool execute = true, const char* const environment = NULL, bool see_all = false);
	bool			loadbuffer						(const char* const buffer, const int len, const char* const filename = NULL, bool execute = true, const char* const environment = NULL, bool see_all = false);
	void			gc								();

	//regster stuffs to lua
	template <typename ClassT> 
	void			register_class_function			(const char* const class_name, const char* const functionname, int (ClassT::*function)(lua_State* L));
	void			register_object					(const char* const class_name, const char* const objectName, void* pObject);
	void			register_function				(const char* const function_name, lua_CFunction f);
	void*			get_object_ptr					(const char* const name);
	void			delete_table					(const char* const name);
	bool			register_lua_function_from_buffer(const char* const function_name, const char* const buffer, const int bufferSizeInByte);

	//example: 
	//	lua fucntion "function_name(f_a, i_b) return "r = " .. f_a .. i_b end" 
	//	callv("function_name", "fi>s")
	//	return value		: the count of return values
	//	!!! if the result contains [char*] type variables, use callv_begin, then copy the char* out, then use callv_end
	int				callv							(const char *func, const char *sig, ...);
	int				callv_begin						(const char *func, const char *sig, ...);
	void			callv_end						(const int result_count);
	int				call_argv						(const char *func, const char *sig, va_list apg);
	void			callf							(const char* const function);

	template <typename T>
	void			set_table						(const char* const table, const char* const field, const T value); //设置已存在的一个table的某个field为value //即 : table[field] = value 

	template <typename T>
	void			set_table						(const char* const table, const T value); //设置已存在的一个table的为value //即 : table = value 

	void			set_table_to_table				(const char* const table, const char* const field, const char* const table2); //设置已存在的一个table的某个field为value //即 : table[field] = value 
	void			set_readonly					(const char* const name);	//将一个table设置为read only，一般从c++导出枚举到lua中会用到
	bool			is_exists						(const char* const name);
	void			create_table					(const char* const name, bool replace_exists = false);	//在lua中创建新table，支持table.member.func的形式
	//void			create_table_no_replace			(const char* const name);	//在lua中创建新table，支持table.member.func的形式, 如果某个table已经存在，不会替换掉原有table
	void			create_table_single				(const char* const name);	//在lua中创建一个新table
	void			create_environment				(const char* const name, bool see_all = false);
	bool			is_exists_single				(const char* const name);
	int				get_table_unsafe				(const char* const table); //!!!这个函数执行后，堆栈是不平衡的!!!
	
	//get raw lua state pointer
	lua_State*		get_state						()	{ return L; }

protected:
	void			_checkstack						(const int i = 0);
	void			_print_error					(const int pop = 0);
	int				_call_va_args					(const char *func, const char *sig, va_list ap);
	static void		_set_environment				(lua_State* L, const char* const name);	//将名称为name的table设置为当前栈顶function的_ENV upvalue。注意！_setEnvironment总是假设当前栈顶必须有一个function
	void			_let_environment_see_all		(const char* const table);

	//递归获取table，处理像"table.member.func"这样的名字
	int				_get_table_recursive			(const char* const fullName);	
	int				_get_parent_table				(const char* const fullName, string128& single_name);	
	static int		_parse_name						(char* name, char** result, const int result_size, char* last_separator = NULL);
	int				_get_tables						(char** names, const int count);

	//注册函数之前的准备工作和注册函数之后的工作
	//注意_post_registerFunction和_prepare_registerFunction必须成对调用，否则会导致堆栈不平衡
	void			_prepare_register_function		(void* pf);
	void			_post_register_function			(const char* funcName);
	int				_prepare_register_class_function(const char* className, const char* funcName, scl::class_function* pf);
	void			_post_register_class_function	();

protected:
	lua_State*		L;
	bool			m_auto_release;
};

template <typename T>
void Lua::set_table(const char* const tableName, const char* const fieldName, const T value)
{
	_checkstack();

	int count = _get_table_recursive(tableName);	// stack + 1 
	assert(count > 0);
	const int table_index = _stack_gettop(L);

	_stack_pushvalue(L, value);							// stack + 1
	_stack_setfield(L, table_index, fieldName);			// stack - 1

	_stack_pop(L, 1);									// stack - 1

	_checkstack();
}

template <typename T>
void Lua::set_table(const char* const table, const T value)
{
	_checkstack();

	string1024	fullname				= table;
	char*		names[64]				= { 0 };
	int			count					= _parse_name(fullname.c_str(), names, 64);
	int			exist_count				= _get_tables(names, count - 1); // stack += exist_count
	if (exist_count != count - 1)
		assert(false);

	_stack_pushvalue(L, value);		// stack + 1 

	_stack_setfield(L, -2, names[count - 1]); // stack - 1

	_stack_pop(L, exist_count);

	_checkstack();
}

template <typename ClassT>
void Lua::register_class_function(const char* const classname, const char* const functionname, int (ClassT::*func)(lua_State* L) )
{
	scl::class_function cf = reinterpret_cast<scl::class_function>(func);
	int upValueCount = _prepare_register_class_function(classname, functionname, &cf); 
	_stack_pushcclosure(L, &_callback_class_lua_standard<ClassT>, upValueCount); 
	_post_register_class_function();
}


template <typename ClassT>
ClassT* _get_object_ptr( lua_State* L )
{
	return _get_object_ptr_stack<ClassT>(L, 1);
}

template <typename ClassT>
ClassT* _get_object_ptr_stack( lua_State* L, const int stackIndex )
{
	//！注意，当前堆栈的栈底元素就是对象的self指针

	//获得obj对象指针
	_stack_pushstring(L, _CHILL_LUA_CPP_PTR_NAME_);
	_stack_gettable(L, stackIndex); //push self[_CHILL_LUA_CPP_PTR_NAME_] to the current stack
	ClassT* obj = static_cast<ClassT*>(_stack_touserdata(L, -1));
	//如果obj是NULL，很可能是lua代码调用写法出了问题：
	//例如  object:func 写成 object.func, 导致第一个参数不是object本身，所以这里获取不到object表，从而无法得到self[_CHILL_LUA_CPP_PTR_NAME_]
	_stack_pop(L, 1);
	assert(obj);
	return obj;
}

template <typename ClassT>
int _callback_class_lua_standard( lua_State* L )
{
	ClassT* obj = _get_object_ptr<ClassT>(L);

	typedef int (ClassT::*FuncT)(lua_State* L);
	FuncT func = _get_class_function_ptr<FuncT>(L);

	_stack_remove(L, 1);	//remove the this pointer

	//执行调用
	int n = (obj->*func)(L);
	return n;
}


template <typename FuncT>
FuncT _get_function_ptr( lua_State* L )
{
	scl::ptr_t data = _stack_touserdata(L, luaState_getUpValueIndex(1));
	void* pdata = &data;
	FuncT* pf = static_cast<FuncT*>(pdata);
	assert(pf);
	return *pf;
}

template <typename FuncT>
FuncT _get_class_function_ptr( lua_State* L )
{

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	const int diff = sizeof(scl::class_function) - sizeof(void*);
	scl::ptr_t low	= _stack_touserdata(L, luaState_getUpValueIndex(1));
	scl::ptr_t high	= _stack_touserdata(L, luaState_getUpValueIndex(2));
	scl::class_function f;// = reinterpret_cast<scl::class_function>(low);
	memcpy(reinterpret_cast<char*>(&f) + diff, &high, sizeof(high)); 
	memcpy(&f, &low, sizeof(low));
	return reinterpret_cast<FuncT>(f);
#endif

#ifdef SCL_WIN
	return _get_function_ptr<FuncT>(L);
#endif

}

class Table
{
public:
	Table(lua_State* luaState);

	Table(lua_State* luaState, bool table_on_stack);

	void create();

	template<typename T>
	void set_field(const char* const field_name, T value);

	bool has_field(const char* const field_name);

	template <typename T>
	T get_field(const char* const field_name);

	template <typename T>
	T get_field(const int field_index);

	template <typename T>
	T get_field(const char* const field_name, const int field_index);

private:
	lua_State*	L;
	int			m_stack_index;
};


template<typename T>
void Table::set_field(const char* const field_name, T value)
{
	_stack_pushvalue(L, value);
	_stack_setfield(L, m_stack_index, field_name);
}


template <typename T>
T Table::get_field(const int field_index)
{
	_stack_pushinteger(L, field_index);
	_stack_gettable(L, m_stack_index);
	T r = _stack_tovalue<T>(L, -1);
	_stack_pop(L, 1);
	return r;
}

template <typename T>
T Table::get_field(const char* const field_name, const int field_index)
{
	if (has_field(field_name))
		return get_field<T>(field_name);
	else
		return get_field<T>(field_index);
}

template <typename T>
T Table::get_field(const char* const field_name)
{
	_stack_getfield(L, m_stack_index, field_name);
	if (_stack_isnil(L, -1))
	{
		_stack_pop(L, 1);
		throw 1;
	}
	T r = _stack_tovalue<T>(L, -1);
	_stack_pop(L, 1);
	return r;
}


//如果注册到lua的函数有多个类型为T&的参数，简单的在_stack_tovalue中返回一个static变量的引用，会导致这些参数都指向了同一个实例，这会造成bug
//所以使用param_holder来保持多个T&类型参数的独立。由于注册到lua的函数的参数数量被限制到16个，所以最多保存16个T的实例就可以解决这个问题了
template <typename T>
class param_holder
{
public:
	param_holder() : m_index(0) {}
	T& get()
	{
		if (m_index >= 16)
			m_index = m_index % 16;
		return m_array[m_index++];
	}

private:
	T m_array[16];
	int m_index;
};



} //namespace luaex 



