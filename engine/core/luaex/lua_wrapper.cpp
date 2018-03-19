//#include "lua_wrapper.h"
//
//#define LUAEX_CHECK_BEGIN const int __check_n = lua_gettop(L)
//#define LUAEX_CHECK_END assert(__check_n == lua_gettop(L)) 
//
//namespace luaex {
//
//static const int MAX_NAME_COUNT = 64;
//
//#ifdef LUAEX_51
//void lua_copy(lua_State* L, int fromidx, int toidx)
//{
//	int top = lua_gettop(L);
//	if (fromidx < 0)
//		fromidx = top + fromidx + 1;
//	if (toidx < 0)
//		toidx = top + toidx + 1;
//	lua_pushvalue(L, fromidx);
//	lua_replace(L, toidx);
//}
//#endif
//
//Lua::Lua() : L(NULL), m_auto_release(true)
//{
//
//}
//
//bool Lua::init(lua_Alloc f)
//{
//	if (NULL != L)
//		return false;	//不要重复初始化
//
//	L = luaL_newstate();
//	if (NULL == L) 
//		return false;
//
//	if (NULL != f)
//		lua_setallocf(L, f, L);
//		
//	luaL_openlibs(L);
//	return true;	
//}
//
//void Lua::set_alloc(lua_Alloc f)
//{
//	if (NULL != f && NULL != L)
//		lua_setallocf(L, f, L);
//}
//
//Lua::~Lua()
//{
//	release();
//}
//
//void Lua::release()
//{
//	if (NULL != L && m_auto_release)
//	{
//		lua_close(L);
//		L = NULL;
//	}
//}
//
//
//bool Lua::loadfile(const char* const file, bool execute, const char* const environment, bool see_all)
//{
//try {
//	_checkstack();
//
//	int r = luaL_loadfile(L, file);					// stack + 1
//	if (r)
//	{
//		_print_error();
//		return false;
//	}
//
//	if (NULL != environment && 0 != environment[0])
//	{
//		_set_environment(L, environment);
//		if (see_all)
//			_let_environment_see_all(environment);
//	}
//
//	if (execute)
//	{
//		r = lua_pcall(L, 0, LUA_MULTRET, 0);		//stack - 1
//		if (r)
//		{
//			_print_error();
//			return false;
//		}
//	}
//	else
//		lua_pop(L, 1);
//
//	_checkstack();
//
//	return true;
//	}
//catch(...) {
//	
//	const int top = lua_gettop(L);
//	if (top > 0)
//		lua_pop(L, top);
//
//	return false;
//	}
//}
//
//bool Lua::loadstring(const char* const script, bool execute, const char* const environment, bool see_all)
//{
//
//SCL_ASSERT_TRY {
//	_checkstack();
//
//	int r = luaL_loadstring(L, script);
//	if (r)
//	{
//		_print_error();
//		return false;
//	}
//	if (NULL != environment && 0 != environment[0])
//	{
//		_set_environment(L, environment);
//		if (see_all)
//			_let_environment_see_all(environment);
//	}
//
//	if (execute)
//	{
//		r = lua_pcall(L, 0, LUA_MULTRET, 0);		//stack - 1
//		if (r)
//		{
//			_print_error();
//			return false;
//		}
//	}
//	else
//		lua_pop(L, 1);
//
//	_checkstack();
//	return true;
//
//	}
//SCL_ASSERT_CATCH {
//
//	const int top = lua_gettop(L);
//	if (top > 0)
//		lua_pop(L, top);
//
//	return false;
//	}
//
//	return true;
//}
//
//
//bool Lua::loadbuffer(const char* const buffer, const int len, const char* const filename, bool execute, const char* const environment, bool see_all)
//{
//
//SCL_ASSERT_TRY {
//
//	stringPath myChunk = "mychunk";
//	if (NULL != filename)
//	{
//		myChunk = filename;
//	}
//
//	int r = luaL_loadbuffer(L, buffer, len, myChunk.c_str());
//	//lua_pop(L, 1);
//	if (r)
//	{
//		_print_error();
//		return false;
//	}
//	if (NULL != environment && 0 != environment[0])
//	{
//		_set_environment(L, environment);
//		if (see_all)
//			_let_environment_see_all(environment);
//	}
//
//	if (execute)
//	{
//		r = lua_pcall(L, 0, LUA_MULTRET, 0);		//stack - 1
//		if (r)
//		{
//			_print_error();
//			return false;
//		}
//	}
//	else
//		lua_pop(L, 1);
//
//	_checkstack();
//	return true;
//
//	}
//SCL_ASSERT_CATCH {
//
//	const int top = lua_gettop(L);
//	if (top > 0)
//		lua_pop(L, top);
//
//	return false;
//	}
//
//	return true;
//}
//
//
//void Lua::gc()
//{
//	lua_gc(L, LUA_GCCOLLECT, 0);
//}
//
//
//void Lua::register_class(const char* const className, const char* const parentClassName)
//{
//	_checkstack();
//
//	//create metatable for class
//	luaL_newmetatable(L, className);			// stack+ 1
//	const int si_metatable = lua_gettop(L);
//
//	//change the metatable's __index to metatable itself;
//	lua_pushliteral	(L, "__index");				// stack + 1
//	lua_pushvalue	(L, si_metatable);			// stack + 1
//	lua_settable	(L, si_metatable);			// stack - 2
//
//	// inherits from parent class
//	if (NULL != parentClassName && 0 != parentClassName[0])
//	{
//		// lookup metatable in Lua registry
//		luaL_getmetatable(L, parentClassName);	// stack + 1  
//		lua_setmetatable(L, si_metatable);		// stack - 1
//	}
//
//	lua_pop(L, 1);								// stack - 1
//
//	_checkstack();
//}
//
//void Lua::register_object(const char* const class_name, const char* const object_name, void* pobj)
//{
//	if (NULL == class_name || NULL == object_name || NULL == pobj || 0 == class_name[0] || 0 == object_name[0])
//	{
//		assert(false);
//		return;
//	}
//
//	LUAEX_CHECK_BEGIN;
//
//	string128 single_name;
//	int parent_stack = _get_parent_table(object_name, single_name); //stack + 1
//
//	//解析object_name
//	bool has_parent = parent_stack != 0;
//
//	//新建object在lua中对应的table，新table创建后被自动放在stacktop，
//	//然后保存新表格对象的stackindex，方便后面使用
//	lua_newtable(L);											// stack + 1
//	int si_newClassTable = lua_gettop(L);
//
//	//设置对象指针
//	lua_pushstring(L, _CHILL_LUA_CPP_PTR_NAME_);				// stack + 1
//	lua_pushlightuserdata(L, pobj);								// stack + 1
//	lua_settable(L, si_newClassTable);							// stack - 2
//
//	//设置对象的metatable
//	luaL_getmetatable(L, class_name);  // lookup metatable in Lua registry // stack + 1
//	lua_setmetatable(L, si_newClassTable);						// stack - 1
//
//	//将对象抛出给脚本
//	if (has_parent)
//	{
//		lua_setfield(L, parent_stack, single_name.c_str());		//stack - 1
//		lua_pop(L, 1); // pop the parent;						//stack - 1
//	}
//	else
//		lua_setglobal(L, single_name.c_str());
//
//	LUAEX_CHECK_END;
//}
//
//void Lua::_checkstack(const int i) // i = 0
//{
//#ifdef SCL_LUA_CHECK_STACK
//	assert(L);
//	const int top = lua_gettop(L);
//	assert(i == top);
//#endif
//}
//
//
////void Lua::_checkstack_begin()
////{
////	assert(L);
////	m_check_count = lua_gettop(L);
////}
////
////void Lua::_checkstack_end()
////{
////	assert(L);
////	int top = lua_gettop(L);
////	assert(m_check_count == top);
////}
//
//int Lua::_prepare_register_class_function(const char* const className, const char* const funcName, scl::class_function* pf)
//{
//	_checkstack();
//
//	luaL_getmetatable(L, className);  // lookup metatable in Lua registry
//
//	//函数名：在_postRegisterClassFunction的settable中有用
//	lua_pushstring(L, funcName);
//
//#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
//
//	//linux的情况 sizeof(scl::class_function) > sizeof(void*)
//	const int diff = sizeof(scl::class_function) - sizeof(void*);
//
//	//将函数指针的低位压入metatable
//	scl::ptr_t low = *reinterpret_cast<scl::ptr_t*>(pf);
//	lua_pushlightuserdata(L, low);
//
//	//将函数指针的高位压入metatable
//	scl::ptr_t high = NULL;
//	memcpy(&high, reinterpret_cast<char*>(pf) + diff, sizeof(high));
//	lua_pushlightuserdata(L, high);
//
//	return 2;
//
//#else
//
//	//windows的情况, sizeof(scl::class_function) == sizeof(void*)
//	scl::ptr_t ptr = *reinterpret_cast<scl::ptr_t*>(pf);
//	lua_pushlightuserdata(L, ptr);
//
//	return 1;
//
//#endif
//}
//
//void Lua::_post_register_class_function()
//{
//	lua_settable(L, 1);
//	lua_pop(L, 1);
//	_checkstack();
//}
//
//
//void Lua::_prepare_register_function(void* pf)
//{
//	_checkstack();
//
//	uint64 pFunc_as_int = *reinterpret_cast<uint64*>(pf);
//	lua_pushlightuserdata(L, (void*)pFunc_as_int);
//}
//
//void Lua::_post_register_function(const char* const funcname)
//{
//	lua_setglobal(L, funcname);
//	_checkstack();
//}
//
//void Lua::register_function(const char* const function, lua_CFunction f)
//{
//	_checkstack();
//	lua_register(L, function, f);
//	_checkstack();
//}
//
//int Lua::callv(const char *func, const char *sig, ...)
//{
//	_checkstack();
//
//	va_list ap;
//	va_start(ap, sig);
//	const int result_count = _call_va_args(func, sig, ap);
//	va_end(ap);
//	lua_pop(L, result_count);
//
//	_checkstack();
//	return result_count;
//}
//
//int Lua::callv_begin(const char *func, const char *sig, ...)
//{
//	_checkstack();
//
//	va_list ap;
//	va_start(ap, sig);
//	int result_count = _call_va_args(func, sig, ap);
//	va_end(ap);
//	return result_count;
//}
//
//void Lua::callv_end(const int result_count)
//{
//	lua_pop(L, result_count);
//	_checkstack();
//}
//
//int Lua::call_argv(const char *func, const char *sig, va_list ap)
//{
//	_checkstack();
//
//	const int result_count = _call_va_args(func, sig, ap);
//	lua_pop(L, result_count);
//
//	_checkstack();
//	return result_count;
//}
//
//int Lua::_call_va_args(const char *func, const char *sig, va_list vl)
//{	
//	char		last_separator			= 0;
//	string1024	fullname				= func;
//	char*		names[MAX_NAME_COUNT]	= { 0 };
//	int			name_count				= _parse_name(fullname.c_str(), names, MAX_NAME_COUNT, &last_separator); 
//	int			table_count				= _get_tables(names, name_count);
//	int			narg					= 0;
//
//	if (last_separator == ':')
//	{
//		lua_pushvalue(L, -2);		// narg + 1
//		++narg;
//	}
//
//	//push arguments												// stack + narg
//	while (*sig)			
//	{    //push arguments 
//
//		switch (*sig++) 
//		{
//		case 'f':  //float argument 
//		case 'd':  //double argument 
//			{
//				lua_pushnumber(L, va_arg(vl, double));
//			}
//			break;
//		case 'i':  //int argument 
//			{
//				lua_pushnumber(L, va_arg(vl, int));
//			}
//			break;
//		case 's':  //string argument 
//			{
//				lua_pushstring(L, va_arg(vl, char *));
//			}
//			break;
//		case '>':
//			{
//				goto endwhile;
//			}
//		default:
//			{
//				log_error << "invalid option (" << *(sig - 1) << ")" << scl::endl;
//			}
//		}
//		narg++;
//		luaL_checkstack(L, 1, "too many arguments");
//	} endwhile:
//
//	int nres = static_cast<int>(strlen(sig));  //number of expected results 
//	//do the call 
//	if (lua_pcall(L, narg, nres, 0) != 0)  //do the call							// stack - narg - 1(function)
//	{
//		log_error << "error running function `"<<func<<"': "<<lua_tostring(L, -1) << scl::endl;		// stack + 1
//		lua_pop(L, 1); //pop the error message										// stack - 1
//
//		lua_pop(L, table_count - 1);													// stack - (tableCount - 1)
//		return 0;
//	}
//
//	//retrieve results
//	nres = -nres;	//stack index of first result 
//	int result_count = 0;
//	while (*sig) 
//	{
//		switch (*sig++) 
//		{
//		case 'd':  //double result 
//			{
//				if (!lua_isnumber(L, nres))
//					log_error << "wrong result type" << scl::endl;
//				*va_arg(vl, double *) = lua_tonumber(L, nres);
//			}
//			break;
//		case 'f':  //double result 
//			{
//				if (!lua_isnumber(L, nres))
//					log_error << "wrong result type" << scl::endl;
//				*va_arg(vl, float *) = static_cast<float>(lua_tonumber(L, nres));
//			}
//			break;
//		case 'i':  //int result 
//			{
//				if (!lua_isnumber(L, nres))
//					log_error << "wrong result type" << scl::endl;
//				*va_arg(vl, int *) = (int)lua_tonumber(L, nres);
//			}
//			break;
//		case 's':  //string result
//			{
//				if (!lua_isstring(L, nres))
//					log_error << "wrong result type" << scl::endl;
//				const char* resultString = lua_tostring(L, nres);
//				*va_arg(vl, const char **) = resultString;
//			}
//			break;
//		default:
//			{
//				log_error << "invalid option (" << *(sig - 1) << ")" << scl::endl;
//			}
//		}
//		++nres;
//		++result_count;
//	}
//	lua_pop(L, table_count - 1); // pop all parent tables							// stack - (tableCount - 1)
//	return result_count;
//}
//
//void Lua::callf( const char* const function )
//{
//	_checkstack();
//	
//	char		last_separator			= 0;
//	string1024	fullname				= function;
//	char*		names[MAX_NAME_COUNT]	= { 0 };
//	int			name_count				= _parse_name(fullname.c_str(), names, MAX_NAME_COUNT, &last_separator); 
//	int			table_count				= _get_tables(names, name_count);
//	int			narg					= 0;
//
//	if (last_separator == ':')
//	{
//		lua_pushvalue(L, -2);		// narg + 1
//		++narg;
//	}
//	
//
//	const int callResult = lua_pcall(L, narg, 0, 0);
//	if (callResult != 0)
//	{
//		const char* errorMessage = lua_tostring(L, -1);
//		log_error << errorMessage << scl::endl;
//		lua_pop(L, 1 + table_count - 1);
//		_checkstack();
//		return;
//	}
//	
//	lua_pop(L, table_count - 1);
//	_checkstack();
//}
//
//void _stack_pushcclosure(lua_State*	L, lua_CFunction fn, int n)
//{
//	lua_pushcclosure(L, fn, n);
//}
//
//void _stack_pushinteger(lua_State *L, int n)
//{
//	lua_pushinteger	(L, n);
//}
//
//void _stack_pushnumber(lua_State *L, lua_Number n)
//{
//	lua_pushnumber	(L, n);
//}
//
//void _stack_pushboolean(lua_State *L, int b)
//{
//	lua_pushboolean	(L, b);
//}
//
//void _stack_pushlightuserdata(lua_State *L, void* p)
//{
//	lua_pushlightuserdata(L, p);
//}
//
//
//void _stack_pop(lua_State*	L, int index)
//{
//	lua_pop(L, index);
//}
//
//void _stack_pushstring(lua_State*	L, const char *s)
//{
//	lua_pushstring(L, s);
//}
//
//void _stack_gettable(lua_State*	L, int index)
//{
//	lua_gettable(L, index);
//}
//
//void* _stack_touserdata(lua_State*	L, int index)
//{
//	return lua_touserdata(L, index);
//}
//
//void _stack_remove(lua_State*	L, int index)
//{
//	lua_remove(L, index);
//}
//
//int _stack_tointeger(lua_State *L, int index)
//{
//	return static_cast<int>(lua_tointeger(L, index));
//}
//
//double _stack_tonumber(lua_State *L, int index)
//{
//	return lua_tonumber(L, index);
//}
//
//bool _stack_toboolean(lua_State *L, int index)
//{
//	return lua_toboolean(L, index) != 0;
//}
//
//const char* _stack_tostring(lua_State *L, int index)
//{
//	return lua_tostring(L, index);
//}
//
//bool Lua::register_lua_function_from_buffer(
//	const char* const function, 
//	const char* const buffer, 
//	const int bufferSizeInByte)
//{
//	_checkstack();
//	if (bufferSizeInByte <= 0 
//		|| NULL == buffer 
//		|| 0 == buffer[0] )
//	{
//		return false;
//	}
//	if (NULL == L)
//	{
//		assert(false);
//		return false;
//	}
//
//	int result = luaL_loadbuffer(L, buffer, bufferSizeInByte, "mychunk"); //"mychunk"  name is the chunk name, used for debug information and error messages. 
//	if (result != 0)
//	{
//		const char* err = lua_tostring(L, -1);
//		log_error << err << scl::endl;
//		//printf("%s\n", err);
//		//if (LUA_ERRSYNTAX == result)
//		//{
//		//	printf("LUA_ERRSYNTAX");
//		//}
//		//if (LUA_ERRMEM == result)
//		//{
//		//	printf("LUA_ERRMEM");
//		//}
//		return false;
//	}
//	lua_setglobal(L, function);
//	_checkstack();
//	return true;
//}
//
//bool Lua::is_exists(const char* const name)
//{
//	LUAEX_CHECK_BEGIN;
//
//	string1024	fullname				= name;
//	char*		names[MAX_NAME_COUNT]	= { 0 };
//	int			count					= _parse_name(fullname.c_str(), names, MAX_NAME_COUNT);
//	int			exist_count				= _get_tables(names, count);
//
//	lua_pop(L, exist_count);
//
//	LUAEX_CHECK_END;
//
//	return count == exist_count;
//}
//
//bool Lua::is_exists_single(const char* const name)
//{
//	_checkstack();
//	lua_getglobal(L, name);
//	bool exists = !lua_isnil(L, -1);
//	lua_pop(L, 1);
//	_checkstack();
//	return exists;
//}
//
//void* Lua::get_object_ptr(const char* const object_name)  // parentName = ""
//{
//	_checkstack();
//
//	void* ptr = NULL;
//	int table_count = _get_table_recursive(object_name);
//	if (table_count == 0)
//	{
//		int top = lua_gettop(L);
//		lua_pop(L, top);
//		_checkstack();
//		return NULL;
//	}
//
//	lua_getfield(L, -1, _CHILL_LUA_CPP_PTR_NAME_); //stack + 1
//	ptr = lua_touserdata(L, -1);
//	lua_pop(L, 2); //pop object and object ptr
//
//	_checkstack();
//
//	return ptr;
//}
//
//void Lua::delete_table(const char* const object_name)
//{
//	LUAEX_CHECK_BEGIN;
//
//	if (NULL == object_name || 0 == object_name[0])
//		return;
//
//	string128 single_name;
//	int si_parent = _get_parent_table(object_name, single_name); // stack + 1
//
//	lua_pushnil(L);							//stack + 1
//	if (si_parent == 0)
//		lua_setglobal(L, single_name.c_str()); //stack - 1
//	else
//	{
//		lua_setfield(L, si_parent, single_name.c_str()); //stack - 1
//		lua_pop(L, 1);							//stack - 1
//	}
//
//	LUAEX_CHECK_END;
//}
//
//
//void Lua::set_readonly(const char* const name)
//{
//	_checkstack();
//
//	//尝试寻找_CHILL_NIL_NEWINDEX_FUNCTION是否已经注册
//	if (!is_exists(_CHILL_NIL_NEWINDEX_FUNCTION))
//	{
//		//const char nil_newindex_func[64] = "error(\"error! the table is READ ONLY!\", 2)";
//		const char nil_newindex_func[256] = "local di = debug.getinfo(2); print(string.format(\"ERROR! %s [%d]. Attempted to update a read-only table.\", di.source, di.currentline));";
//		register_lua_function_from_buffer(_CHILL_NIL_NEWINDEX_FUNCTION, nil_newindex_func, static_cast<int>(strlen(nil_newindex_func)));
//	}
//
//	//lua_pushliteral(m_L, name);		// stack + 1
//	//const int object_name_index = lua_gettop(L);
//
//	//lua_getfield(L, LUA_GLOBALSINDEX, name);		// stack + 1
//	lua_getglobal(L, name);		// stack + 1
//	if (lua_isnil(L, -1))
//	{
//		return;
//	}
//	const int object_index = lua_gettop(L);
//
//	//创建一个新表，这个表将继承原有表的__index,同时修改__newindex为只读
//	//传入的name将被重新指向这个新表，原有的表作废了
//	lua_createtable(L, 0, 0);						// stack + 1
//	const int new_table_index = lua_gettop(L);
//	
//	string128 mt_name;
//	mt_name.format("_chill_%s_readonly_mt", name);
//
//	luaL_newmetatable(L, mt_name.c_str());	// stack + 1
//	const int mt_index = lua_gettop(L);
//
//	//设置meta table 的 __index
//	lua_pushliteral		(L, "__index");		// stack + 1
//	lua_pushvalue		(L, object_index);	// stack + 1
//	lua_settable		(L, mt_index);		// stack - 2
//
//	//设置meta table 的 __newindex
//	lua_pushliteral		(L, "__newindex");		// stack + 1
//	//lua_pushcfunction	(m_L, __newindex_readonly);	// stack + 1
//	//lua_getfield		(L, LUA_GLOBALSINDEX, _CHILL_NIL_NEWINDEX_FUNCTION);	// stack + 1
//	lua_getglobal		(L,  _CHILL_NIL_NEWINDEX_FUNCTION);	// stack + 1
//	lua_settable		(L, mt_index);			// stack - 2
//
//	lua_pushvalue		(L, mt_index);			// stack + 1
//	lua_setmetatable	(L, new_table_index);		// stack - 1
//
//	//将原table重定向至新的只读table
//	lua_pushvalue		(L, new_table_index);			// stack + 1
//	//lua_setfield		(L, LUA_GLOBALSINDEX, name);	// stack - 1
//	lua_setglobal		(L, name);	// stack - 1
//
//	lua_pop(L, 3);	// stack - 3;
//
//	_checkstack();
//
//	return;
//}
//
//
//void Lua::create_table_single(const char* const name)
//{
//	_checkstack();
//
//	lua_createtable(L, 0, 0);		//	stack + 1
//	lua_setglobal(L, name);			//	stack - 1
//
//	_checkstack();
//}
//
//
//void Lua::create_table(const char* const name, bool replace_exists)
//{
//	_checkstack();
//
//	string1024	fullname				= name;
//	char*		names[MAX_NAME_COUNT]	= { 0 };
//	int			name_count				= _parse_name(fullname.c_str(), names, MAX_NAME_COUNT);
//	int			start					= 0;
//
//	if (!replace_exists)
//		start = _get_tables(names, name_count);
//	
//	for (int i = start; i < name_count; ++i)
//	{
//		lua_createtable(L, 0, 0);					// stack + 1
//		lua_pushnil(L);								// stack + 1
//		lua_copy(L, -2, -1);							//copy table for child set field
//		if (i == 0)
//			lua_setglobal(L, names[i]);				// stack - 1
//		else
//			lua_setfield(L, -3, names[i]);			// stack - 1
//	}
//	lua_pop(L, name_count); 				// stack - name_count
//
//	_checkstack();
//}
//
//
//void _stack_getfield(lua_State* L, int index, const char* k)
//{
//	lua_getfield(L, index, k);
//}
//
//int _stack_gettop(lua_State* L)
//{
//	return lua_gettop(L);
//}
//
//void _stack_setfield(lua_State* L, int index, const char* k)
//{
//	lua_setfield(L, index, k);
//}
//
//bool _stack_isnil(lua_State* L, int index)
//{
//	return lua_isnil(L, index);
//}
//
//inline bool is_lua_identifier(char c)
//{
//	if (c >= 'A' && c <= 'Z')
//		return true;
//	if (c >= 'a' && c <= 'z')
//		return true;
//	if (c >= '0' && c <= '9')
//		return true;
//	if (c == '_')
//		return true;
//	return false;
//}
//
//int Lua::_get_tables(char** names, const int count)
//{
//	int	exist_count = 0;
//	for (int i = 0; i < count; ++i)
//	{
//		if (i == 0)
//			lua_getglobal(L, names[i]);
//		else
//			lua_getfield(L, -1, names[i]);
//
//		bool exists = !lua_isnil(L, -1);
//		if (exists)
//			++exist_count;
//		else
//		{
//			lua_pop(L, 1); // pop the nil
//			break;
//		}
//	}
//	return exist_count;
//}
//
//int Lua::_get_table_recursive(const char* const _fullname)
//{
//	string1024	fullname				= _fullname;
//	char*		names[MAX_NAME_COUNT]	= { 0 };
//	int			count					= _parse_name(fullname.c_str(), names, MAX_NAME_COUNT);
//	int			exist_count				= _get_tables(names, count);
//	if (exist_count > 1)
//	{
//		lua_copy(L, -1, -exist_count);
//		lua_pop(L, exist_count - 1);
//	}
//	return (exist_count == count && count > 0) ? 1 : 0;
//}
//
//
//void Lua::_set_environment(lua_State* L, const char* const environment)
//{
//	//assume the function is on top of the stack.
//	lua_getglobal(L, environment);			// stack + 1
//	if (lua_isnil(L, -1))
//	{
//		lua_newtable(L);					// stack + 1
//		lua_copy(L, -1, -2);
//		lua_setglobal(L, environment);	// stack - 1
//	}
//#ifdef LUAEX_51
//	int r = lua_setfenv(L, -2);
//	if (r != 1)
//	{
//		assert(false);
//	}
//#else
//	const char* upname = lua_setupvalue(L, -2, 1); // stack - 1
//	if (0 != strcmp("_ENV", upname))
//	{
//		assert(false);
//		lua_pop(L, 1);
//	}
//#endif
//}
//
//void Lua::_print_error(const int pop)
//{
//	//打印错误结果 
//	const char* error = lua_tostring(L, -1);
//	log_error << error << scl::endl;
//
//	lua_pop(L, 1);
//	if (pop > 0)
//		lua_pop(L, pop);
//
//	_checkstack();
//}
//
//int Lua::_parse_name(char* name, char** result, const int result_size, char* last_separator)
//{
//	char*	p		= name;
//	int		count	= 0;
//	bool	start	= false;
//	while (1)
//	{
//		const char c = *p++;
//		if (is_lua_identifier(c))
//		{
//			if (!start)
//			{
//				start = true;
//				result[count++] = p - 1;
//				if (count >= result_size)
//					break;
//			}
//		}
//		else
//		{
//			if (NULL != last_separator && c != 0)
//				*last_separator = c;
//			*(p - 1) = 0;
//			start = false;
//		}
//		if (c == 0)
//			break;
//	}
//	return count;
//}
//
//int Lua::_get_parent_table(const char* const object_name, string128& single_name)
//{
//	//解析object_name
//	string1024	fullname				= object_name; 
//	char*		names[MAX_NAME_COUNT]	= { 0 };
//	int			name_count				= _parse_name(fullname.c_str(), names, MAX_NAME_COUNT);
//	int			parent_count			= 0;
//	int			si_parent				= 0;
//	if (name_count > 0 && names[name_count - 1] != NULL)
//	{
//		single_name = names[name_count - 1];
//	}
//
//	//如果有parent，先把parent入栈
//	if (name_count > 1)
//	{
//		parent_count = _get_tables(names, name_count - 1);
//		if (parent_count != name_count - 1)
//		{
//			log_error << "cann't find parent" << scl::endl;
//			assert(false); //can't find parent
//			return 0;
//		}
//		else if (parent_count > 1)
//		{
//			lua_copy(L, -1, -parent_count);
//			lua_pop(L, parent_count - 1); // leave the last parent only
//		}
//		si_parent = lua_gettop(L);
//	}
//	return si_parent;
//}
//
//void Lua::_let_environment_see_all(const char* const _table)
//{
//	int tc = _get_table_recursive(_table);					// stack + 1
//	if (tc <= 0)
//		return;
//
//	int si_table = lua_gettop(L);
//
//	string256 mt_name = _table;
//	mt_name += "__luaex__env__metatable";
//
//	//get meta table, or create a new one
//	luaL_getmetatable(L, mt_name.c_str());						// stack + 1
//	const int si_metatable = lua_gettop(L);
//	if (lua_isnil(L, -1))
//	{
//		lua_pop(L, 1);											// stack - 1
//		luaL_newmetatable(L, mt_name.c_str());					// stack + 1
//
//		//set metatable.__index = _G
//		lua_pushliteral(L, "__index");							// stack + 1
//#ifdef LUAEX_51
//		lua_pushvalue(L, LUA_GLOBALSINDEX);
//#else
//		lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);	// stack + 1
//#endif
//		lua_settable(L, si_metatable);							// stack - 2
//	}
//
//	//set table's metatable 
//	lua_setmetatable(L, si_table);							// stack - 1
//
//	lua_pop(L, 1);											// stack - 1
//
//	//_checkstack();
//}
//
//void Lua::create_environment(const char* const name, bool see_all)
//{
//	_checkstack();
//
//	create_table(name);
//	if (see_all)
//		_let_environment_see_all(name);
//
//	_checkstack();
//}
//
//void Lua::set_table_to_table(const char* const table, const char* const field, const char* const table2)
//{
//	LUAEX_CHECK_BEGIN;
//
//	int table_count = _get_table_recursive(table);			// stack + 1 
//	if (table_count <= 0)
//	{
//		int top = lua_gettop(L);
//		lua_pop(L, top);
//		_checkstack();
//		return;
//	}
//	const int table_index = _stack_gettop(L);
//
//	table_count = _get_table_recursive(table2);					//stack + 1
//	if (table_count <= 0)
//	{
//		int top = lua_gettop(L);
//		lua_pop(L, top);
//		_checkstack();
//		return;
//	}
//	_stack_setfield(L, table_index, field);			// stack - 1
//
//	_stack_pop(L, 1);									// stack - 1
//
//	LUAEX_CHECK_END;
//}
//
//int Lua::get_table_unsafe(const char* const table)
//{
//	int table_count = _get_table_recursive(table);
//	assert(table_count == 0 || table_count == 1);
//	return table_count;
//}
//
//
//int luaState_getUpValueIndex(const int i)
//{
//	return lua_upvalueindex(i);
//}
//
//
//Table::Table(lua_State* luaState) : L(luaState), m_stack_index(0)
//{
//}
//
//Table::Table(lua_State* luaState, bool table_on_stack) : L(luaState), m_stack_index(0)
//{
//	if (table_on_stack)
//		m_stack_index = lua_gettop(L);
//}
//
//bool Table::has_field(const char* const field_name)
//{
//	lua_getfield(L, m_stack_index, field_name);
//	bool exists = !lua_isnil(L, -1);
//	lua_pop(L, 1);
//	return exists;
//}
//
//void Table::create()
//{
//	lua_newtable(L);
//	m_stack_index = lua_gettop(L);
//}
//
//int _stack_pcall(lua_State* L, int n, int r, int f)
//{
//#ifdef LUAEX_51
//	return lua_pcall(L, (n), (r), (f));
//#else
//	return lua_pcallk(L, (n), (r), (f), 0, NULL);
//#endif
//}
//
//void _stack_getglobal(lua_State* L, const char* name)
//{
//	lua_getglobal(L, name);
//}
//
//} //namespace luaex 
//
//
