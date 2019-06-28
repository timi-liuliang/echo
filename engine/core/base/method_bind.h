#pragma once

#include "engine/core/script/lua/lua_binder.h"

namespace Echo
{
	class MethodBind
	{
	public:
		virtual int call(lua_State* luaState) = 0;
	};

	template <typename R, typename P0>
	class MethodBind1R : public MethodBind
	{
	public:
		R(*method)(P0);

		virtual int call(lua_State* luaState) override
		{
			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 1);

			// exec method
			R result = (*method)(p0);

			// free value
			lua_freevalue<P0>(p0);

			// push the results
			lua_pushvalue<R>(luaState, result);

			// return number of results
			return 1;
		}
	};

	template<typename R, typename P0>
	MethodBind* createMethodBind(R(*method)(P0))
	{
		MethodBind1R<R, P0>* bind = new (MethodBind1R<R, P0>);
		bind->method = method;

		return bind;
	}

	template <typename R, typename P0, typename P1>
	class MethodBind2R : public MethodBind
	{
	public:
		R (*method)(P0, P1);

		virtual int call(lua_State* luaState) override
		{
			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 1);
			P1 p1 = lua_getvalue<P1>(luaState, 2);

			// exec method
			R result = (*method)(p0, p1);

			// push the results
			lua_pushvalue<R>(luaState, result);

			// free value
			lua_freevalue<P0>(p0);
			lua_freevalue<P1>(p1);

			// return number of results
			return 1;
		}
	};

	template<typename R, typename P0, typename P1>
	MethodBind* createMethodBind(R(*method)(P0, P1))
	{
		MethodBind2R<R, P0, P1>* bind = new (MethodBind2R<R, P0, P1>);
		bind->method = method;

		return bind;
	}

	template <typename R, typename P0, typename P1, typename P2>
	class MethodBind3R : public MethodBind
	{
	public:
		R(*method)(P0, P1, P2);

		virtual int call(lua_State* luaState) override
		{
			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 1);
			P1 p1 = lua_getvalue<P1>(luaState, 2);
			P2 p2 = lua_getvalue<P2>(luaState, 3);

			// exec method
			R result = (*method)(p0, p1, p2);

			// push the results
			lua_pushvalue<R>(luaState, result);

			// free value
			lua_freevalue<P0>(p0);
			lua_freevalue<P1>(p1);
			lua_freevalue<P2>(p2);

			// return number of results
			return 1;
		}
	};

	template<typename R, typename P0, typename P1, typename P2>
	MethodBind* createMethodBind(R(*method)(P0, P1, P2))
	{
		MethodBind3R<R, P0, P1, P2>* bind = new (MethodBind3R<R, P0, P1, P2>);
		bind->method = method;

		return bind;
	}


	template <typename R, typename P0, typename P1, typename P2, typename P3>
	class MethodBind4R : public MethodBind
	{
	public:
		R(*method)(P0, P1, P2, P3);

		virtual int call(lua_State* luaState) override
		{
			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 1);
			P1 p1 = lua_getvalue<P1>(luaState, 2);
			P2 p2 = lua_getvalue<P2>(luaState, 3);
			P3 p3 = lua_getvalue<P3>(luaState, 4);

			// exec method
			R result = (*method)(p0, p1, p2, p3);

			// push the results
			lua_pushvalue<R>(luaState, result);

			// free value
			lua_freevalue<P0>(p0);
			lua_freevalue<P1>(p1);
			lua_freevalue<P2>(p2);
			lua_freevalue<P3>(p3);

			// return number of results
			return 1;
		}
	};

	template<typename R, typename P0, typename P1, typename P2, typename P3>
	MethodBind* createMethodBind(R(*method)(P0, P1, P2, P3))
	{
		MethodBind4R<R, P0, P1, P2, P3>* bind = new (MethodBind4R<R, P0, P1, P2, P3>);
		bind->method = method;

		return bind;
	}

	// register method
	bool registerMethodBind(const String& methodName, MethodBind* method);

	// bind method
	template<typename N, typename M>
	static MethodBind* bindMethod( M method, N methodName)
	{
		MethodBind* bind = createMethodBind(method);

		registerMethodBind(methodName, bind);

		return bind;
	}
}

#define BIND_METHOD(method, methodName) \
	bindMethod(&method, methodName)