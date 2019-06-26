#pragma once

#include "variant.h"
#include "signal.h"
#include "method_bind.h"

namespace Echo
{
#ifdef DEBUG_METHODS_ENABLED
	struct MethodDefinition
	{
		String					m_name;
		vector<String>::type	m_args;

		MethodDefinition() {}
		MethodDefinition(const String& name) : m_name(name) {}
	};

	Echo::MethodDefinition DEF_METHOD(const String& name);
	Echo::MethodDefinition DEF_METHOD(const String& name, const String& arg0);
#else
	#define DEF_METHOD(m_c, ...) m_c
#endif

	class Object;
	class ClassMethodBind
	{
	public:
        // call for c++
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) = 0;
        
        // call for lua
		virtual int call(Object* obj, lua_State* luaState)=0;
	};
	// please use hash map
	typedef std::map<String, ClassMethodBind*>	ClassMethodMap;

	// declare a empty class..
	class __AnEmptyClass{};

	class ClassMethodBind0 : public ClassMethodBind
	{
	public:
		void (__AnEmptyClass::*method)();
		
		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			(instance->*method)();

			return Variant();
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;
			(instance->*method)();

			return 0;
		}
	};

	template<typename T>
	ClassMethodBind* createMethodBind(void(T::*method)())
	{
		ClassMethodBind0* bind = new(ClassMethodBind0);

		union 
		{
			void (T::*sm)();
			void (__AnEmptyClass::*dm)();
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	template<typename R>
	class ClassMethodBind0R : public ClassMethodBind
	{
	public:
		R (__AnEmptyClass::*method)();

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			return (instance->*method)();
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			// exec method
			R result = (instance->*method)();

			// push the results
			lua_pushvalue<R>(luaState, result);

			// return number of results
			return 1;
		}
	};

	template<typename T, typename R>
	ClassMethodBind* createMethodBind(R(T::*method)())
	{
		ClassMethodBind0R<R> * bind = new (ClassMethodBind0R<R>);

		union
		{
			R (T::*sm)();
			R (__AnEmptyClass::*dm)();
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	template<typename R>
	class ClassMethodBind0RC : public ClassMethodBind
	{
	public:
		R(__AnEmptyClass::*method)() const;

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			return (instance->*method)();
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			// exec method
			R result = (instance->*method)();

			// push the results
			lua_pushvalue<R>(luaState, result);

			// return number of results
			return 1;
		}
	};

	template<typename T, typename R>
	ClassMethodBind* createMethodBind(R(T::*method)() const)
	{
		ClassMethodBind0RC<R> * bind = new (ClassMethodBind0RC<R>);

		union
		{
			R(T::*sm)() const;
			R(__AnEmptyClass::*dm)() const;
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	template <typename P0>
	class ClassMethodBind1 : public ClassMethodBind
	{
	public:
		void (__AnEmptyClass::*method)(P0);

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			P0 p0 = variant_cast<P0>(*args[0]);
			(instance->*method)(p0);

			return Variant();
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>( luaState, 2);

			// exec method
			(instance->*method)(p0);

			// free value
			lua_freevalue<P0>(p0);

			// return number of results
			return 0;
		}
	};

	template<typename T, typename P0>
	ClassMethodBind* createMethodBind(void(T::*method)(P0))
	{
		ClassMethodBind1<P0>* bind = new (ClassMethodBind1<P0>);

		union
		{
			void (T::*sm)(P0);
			void (__AnEmptyClass::*dm)(P0);
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	template <typename R, typename P0>
	class ClassMethodBind1R : public ClassMethodBind
	{
	public:
		R (__AnEmptyClass::*method)(P0);

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			P0 p0 = variant_cast<P0>(*args[0]);
			return (instance->*method)(p0);
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 2);

			// exec method
			R result = (instance->*method)(p0);

			// free value
			lua_freevalue<P0>(p0);

			// push the results
			lua_pushvalue<R>(luaState, result);

			// return number of results
			return 1;
		}
	};

	template<typename T, typename R, typename P0>
	ClassMethodBind* createMethodBind(R (T::*method)(P0))
	{
		ClassMethodBind1R<R, P0>* bind = new (ClassMethodBind1R<R, P0>);

		union
		{
			R (T::*sm)(P0);
			R (__AnEmptyClass::*dm)(P0);
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	template <typename P0, typename P1>
	class ClassMethodBind2 : public ClassMethodBind
	{
	public:
		void (__AnEmptyClass::*method)(P0, P1);

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			P0 p0 = variant_cast<P0>(*args[0]);
			P1 p1 = variant_cast<P1>(*args[1]);
			(instance->*method)(p0, p1);

			return Variant();
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 2);
			P0 p1 = lua_getvalue<P1>(luaState, 3);

			// exec method
			(instance->*method)(p0, p1);

			lua_freevalue<P0>(p0);
			lua_freevalue<P1>(p1);

			// return number of results
			return 0;
		}
	};

	template<typename T, typename P0, typename P1>
	ClassMethodBind* createMethodBind(void(T::*method)(P0, P1))
	{
		ClassMethodBind2<P0, P1>* bind = new (ClassMethodBind2<P0, P1>);

		union
		{
			void (T::*sm)(P0, P1);
			void (__AnEmptyClass::*dm)(P0, P1);
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	template <typename R, typename P0, typename P1>
	class ClassMethodBind2R : public ClassMethodBind
	{
	public:
		R(__AnEmptyClass::*method)(P0, P1);

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			P0 p0 = variant_cast<P0>(*args[0]);
			P1 p1 = variant_cast<P1>(*args[1]);
			return (instance->*method)(p0, p1);
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 2);
			P1 p1 = lua_getvalue<P1>(luaState, 3);

			// exec method
			R result = (instance->*method)(p0, p1);

			// free value
			lua_freevalue<P0>(p0);
			lua_freevalue<P1>(p1);

			// push the results
			lua_pushvalue<R>(luaState, result);

			// return number of results
			return 1;
		}
	};

	template<typename T, typename R, typename P0, typename P1>
	ClassMethodBind* createMethodBind(R(T::*method)(P0, P1))
	{
		ClassMethodBind2R<R, P0, P1>* bind = new (ClassMethodBind2R<R, P0, P1>);

		union
		{
			R(T::*sm)(P0, P1);
			R(__AnEmptyClass::*dm)(P0, P1);
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}
}
