/*相关API:
	http://pgl.yoyo.org/luai/i/_

	lua_createtable
	原型: void lua_createtable (lua_State *L, int narr, int nrec);
	描述: 创建一个新的table并将之放在栈顶.narr是该table数组部分的长度,nrec是该table hash部分的长度.
		  当我们确切的知道要放多少元素到table的时候,使用这个函数,lua可以预分配一些内存,提升性能.
		  如果不确定要存放多少元素可以使用 lua_newtable 函数来创建table.

	lua_newtable
	原型: void lua_newtable (lua_State *L);
	描述: 创建一个新的table并将之放在栈顶. 等同于lua_createtable(L, 0, 0).

	lua_getfield
	原型: void lua_getfield (lua_State *L, int index, const char *k);
	描述: 将t[k]元素push到栈顶. 其中t是index处的table.
		  这个函数可能触发index元方法.

	lua_setfield
	原型: void lua_setfield (lua_State *L, int index, const char *k);
	描述: 为table中的key赋值. t[k] = v . 其中t是index处的table , v为栈顶元素.
		  这个函数可能触发newindex元方法.
		  调用完成后弹出栈顶元素(value).

	lua_gettable
	原型: void lua_gettable (lua_State *L, int index);
	描述: 将t[k]元素push到栈顶. 其中t是index处的table,k为栈顶元素.
		  这个函数可能触发index元方法.
		  调用完成后弹出栈顶元素(key).

	lua_rawget
	原型: void lua_rawget (lua_State *L, int index);
	描述: 与lua_gettable函数类似, 但是不会触发__index元方法.

	lua_rawset
	原型: void lua_rawset (lua_State *L, int index);
	描述: 与lua_settable函数类似, 但是不会触发newindex元方法.

	lua_rawgeti
	原型: void lua_rawgeti (lua_State *L, int index, int n);
	描述: 将t[n]元素push到栈顶.其中t是index处的table.
		  这个函数不会触发index元方法.

	lua_rawseti
	原型: void lua_rawseti (lua_State *L, int index, int n);
	描述: 为table中的key赋值. t[n] = v .其中t是index处的table , v为栈顶元素.
		  这个函数不会触发newindex元方法.
		  调用完成后弹出栈顶元素.

	lua_rawgetp
	原型: void lua_rawgetp (lua_State *L, int index, const void *p);
	描述: 将t[p]元素push到栈顶.其中t是index处的table. p是一个lightuserdata.
		  这个函数不会触发index元方法.

	lua_rawsetp
	原型: void lua_rawsetp (lua_State *L, int index, const void *p);
	描述: 为table中的key赋值. t[p] = v .其中t是index处的table , p是一个lightuserdata , v为栈顶元素.
		  这个函数不会触发newindex元方法.
		  调用完成后弹出栈顶元素.

	lua_getmetatable
	原型: int lua_getmetatable (lua_State *L, int index);
	描述: 将index处元素的元表push到栈顶. 如果该元素没有元表, 函数返回0 , 不改变栈.

	lua_setmetatable
	prototype  : int lua_setmetatable (lua_State *L, int index);
	description: Pops a table from the stack and sets it as the new metatable for the value at the given acceptable index.

	lua_istable
	原型: int lua_istable (lua_State *L, int index);
	描述: 判断index处元素是否为一个table , 如果是返回1,否则返回0.

	lua_pushglobaltable
	原型: void lua_pushglobaltable (lua_State *L);
	描述: 将lua的全局表放在栈顶.

	luaL_newmetatable
	原型: int luaL_newmetatable (lua_State *L, const char *tname);
	描述: 如果注册表中已经有名为tname的key,则返回0.
		  否则创建一个新table作为userdata的元表. 这个元表存储在注册表中,并以tname为key. 返回1.
		  函数完成后将该元表置于栈顶.

	luaL_getmetatable
	原型: void luaL_getmetatable (lua_State *L, const char *tname);
	描述: 将注册表中以tname为key的元表push到栈顶.

	luaL_setmetatable
	原型: void luaL_setmetatable (lua_State *L, const char *tname);
	描述: 将栈顶元素存储到注册表中, 它的key为tname.

	luaL_getsubtable
	原型: int luaL_getsubtable (lua_State *L, int idx, const char *fname);
	描述: 将 t[fname] push到栈顶, 其中t是index处的table , 并且 t[fname] 也为一个table.
	如果 t[fname] 原本就存在,返回 true ,否则返回false,并且将 t[fname] 新建为一张空表.

	lua_getglobal
	原型: void lua_getglobal (lua_State *L, const char *name);
	描述: 将 t[name] 元素push到栈顶, 其中t为全局表.

	lua_setglobal
	prototype  : void lua_setglobal (lua_State *L, const char *name);
	description: Pops a value from the stack and sets it as the new value of global name. 
				 It is defined as a macro:
				 #define lua_setglobal(L,s)   lua_setfield(L, LUA_GLOBALSINDEX, s)

	luaL_newlibtable
	原型: void luaL_newlibtable (lua_State *L, const luaL_Reg l[]);
	描述: 创建一张空表, lua预先分配足够的内存用来存储我们创建的函数库.
	稍后我们可以使用 luaL_setfuncs 函数注册我们的函数库.

	luaL_setfuncs
	原型: void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);
	描述: 将所有 luaL_Reg数组中的函数注册到栈顶的table中.
		  当upvalue个数不为0时,所创建的所有函数共享这些upvalue. -2到-(nup+1)的元素为要注册的upvalue.
		  (注意:这些upvalue是c中的upvalue,不是lua中的upvalue,可以在注册的c函数中通过 lua_upvalueindex(n)获取其值.)
		  调用完成后弹出栈顶的所有upvalue.

	luaL_newlib
	原型: void luaL_newlib (lua_State *L, const luaL_Reg *l);
	描述: 创建一个新的table , 并将luaL_Reg数组中的函数注册到其中.
		  它是一个宏 (luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

	lua_next
	原型: int lua_next (lua_State *L, int index);
	描述: 该函数用来遍历一个table.
		  从栈顶弹出一个key , 并且push一个 key-value对(栈顶key的下一个键值对) ,到栈顶.
		  如果table中没有更多的元素, 函数返回0.
		  遍历开始时栈顶为一个nil , 函数取出第一个键值对.

	通常遍历方法为:
	lua_pushnil(L);  // first key
	while (lua_next(L, t) != 0) {
		// uses 'key' (at index -2) and 'value' (at index -1)
		printf("%s - %s\n",
		lua_typename(L, lua_type(L, -2)),
		lua_typename(L, lua_type(L, -1)));
		// removes 'value'; keeps 'key' for next iteration
		lua_pop(L, 1);
	}
	注意: 在遍历table的时候 ,除非明确的知道key为字符串,不要对栈上的key使用 lua_tolstring 函数 ,
		  因为这样有可能改变key的类型 , 影响下一次 lua_next调用.

	lua_rawlen
	原型: size_t lua_rawlen (lua_State *L, int index);
	描述: 获取index处元素的长度.
		  对于字符串来说,返回字符串长度.
		  对于table来说,返回#操作符的长度. 不受元方法影响.
		  对于userdata来说,返回内存的大小.
		  其他元素返回0.

	lua_len
	原型: void lua_len (lua_State *L, int index);
	描述: 获取index处元素#操作符的结果 , 放置在栈顶.


	其他概念:
		1.伪索引:
			Lua栈的正常索引 从栈顶算,栈顶为-1,向栈低递减. 从栈低算,栈低为1,向栈顶递增.
			伪索引是一种索引,他不在栈的位置中,通过一个宏来定义伪索引的位置.
			伪索引被用来访问注册表,或者在lua_CFunction中访问upvalue.
		2.注册表:
			Lua的注册表是一个预定义的table, 可以提供给c api存储一切想要存储的值.
			注册表通过 LUA_REGISTRYINDEX 伪索引来访问.
			例如 lua_getfield 函数可以像下面这样使用来获取注册表中的一个以"hello"为key的值 :
			lua_getfield( L , LUA_REGISTRYINDEX , "hello");
		3. upvalue:
			在使用 lua_pushcfunction 或者 luaL_setfuncs 将一个lua_CFunction 注册到Lua环境中时,
			可以同时为这个函数设置一些upvalue .
			而后在这些lua_CFunction 中可以使用 lua_upvalueindex(n) 函数来获取对应位置的upvalue.


	lua_pushcclosure
	prototype  : void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n);
	description: Pushes a new C closure onto the stack.
				 When a C function is created, it is possible to associate some values with it, thus creating 
				 a C closure (see §3.4); these values are then accessible to the function whenever it is called. 
				 To associate values with a C function, first these values should be pushed onto the stack 
				 (when there are multiple values, the first value is pushed first). Then lua_pushcclosure is 
				 called to create and push the C function onto the stack, with the argument n telling how many 
				 values should be associated with the function. lua_pushcclosure also pops these values from 
				 the stack.
				 The maximum value for n is 255.

	lua_gettop
	prototype  ; int lua_gettop (lua_State *L);
	description: The lua_gettop function returns the number of elements in the stack, which is also the 
				 index of the top element. Notice that a negative index -x is equivalent to the positive 
				 index gettop - x + 1.

	lua_settop
	prototype  : lua_settop( lua_State *L, int idx)
	description: lua_settop sets the top (that is, the number of elements in the stack) to a specific value. 
				 If the previous top was higher than the new one, the top values are discarded. Otherwise, 
				 the function pushes nils on the stack to get the given size. As a particular case, lua_settop(L, 0) 
				 empties the stack. You can also use negative indices with lua_settop; that will set the top 
				 element to the given index. Using this facility, the API offers the following macro, which 
				 pops n elements from the stack:
				 #define lua_pop(L,n)  lua_settop(L, -(n)-1)

	lua_pushliteral
	prototype  :void lua_pushliteral (lua_State *L, const char *s);
	description:This macro is equivalent to lua_pushlstring, but can be used only when s is a literal string. 
				In these cases, it automatically provides the string length.

	lua_pushvalue
	prototype  :void lua_pushvalue (lua_State *L, int index);
	description:Pushes a copy of the element at the given valid index onto the stack

	lua_settable
	prototype  :void lua_settable (lua_State *L, int index);
	description:Does the equivalent to t[k] = v, where t is the value at the given valid index, v is the value 
				at the top of the stack, and k is the value just below the top.

				This function pops both the key and the value from the stack. As in Lua, this function may 
				trigger a metamethod for the "newindex" event (see §2.8)

	lua_pop
	prototype  :void lua_pop (lua_State *L, int n);
	description:Pops n elements from the stack.
*/