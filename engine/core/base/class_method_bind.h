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
			P1 p1 = lua_getvalue<P1>(luaState, 3);

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

	// three parameters
	template <typename P0, typename P1, typename P2>
	class ClassMethodBind3 : public ClassMethodBind
	{
	public:
		void (__AnEmptyClass::* method)(P0, P1, P2);

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			P0 p0 = variant_cast<P0>(*args[0]);
			P1 p1 = variant_cast<P1>(*args[1]);
			P2 p2 = variant_cast<P2>(*args[2]);
			(instance->*method)(p0, p1, p2);

			return Variant();
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 2);
			P1 p1 = lua_getvalue<P1>(luaState, 3);
			P2 p2 = lua_getvalue<P2>(luaState, 4);

			// exec method
			(instance->*method)(p0, p1, p2);

			lua_freevalue<P0>(p0);
			lua_freevalue<P1>(p1);
			lua_freevalue<P2>(p2);

			// return number of results
			return 0;
		}
	};

	template<typename T, typename P0, typename P1, typename P2>
	ClassMethodBind* createMethodBind(void(T::* method)(P0, P1, P2))
	{
		ClassMethodBind3<P0, P1, P2>* bind = new (ClassMethodBind3<P0, P1, P2>);

		union
		{
			void (T::* sm)(P0, P1, P2);
			void (__AnEmptyClass::* dm)(P0, P1, P2);
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	template <typename R, typename P0, typename P1, typename P2>
	class ClassMethodBind3R : public ClassMethodBind
	{
	public:
		R(__AnEmptyClass::* method)(P0, P1, P2);

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			P0 p0 = variant_cast<P0>(*args[0]);
			P1 p1 = variant_cast<P1>(*args[1]);
			P2 p2 = variant_cast<P2>(*args[2]);
			return (instance->*method)(p0, p1, p2);
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 2);
			P1 p1 = lua_getvalue<P1>(luaState, 3);
			P2 p2 = lua_getvalue<P2>(luaState, 4);

			// exec method
			R result = (instance->*method)(p0, p1, p2);

			// free value
			lua_freevalue<P0>(p0);
			lua_freevalue<P1>(p1);
			lua_freevalue<P2>(p2);

			// push the results
			lua_pushvalue<R>(luaState, result);

			// return number of results
			return 1;
		}
	};

	template<typename T, typename R, typename P0, typename P1, typename P2>
	ClassMethodBind* createMethodBind(R(T::* method)(P0, P1, P2))
	{
		ClassMethodBind3R<R, P0, P1, P2>* bind = new (ClassMethodBind3R<R, P0, P1, P2>);

		union
		{
			R(T::* sm)(P0, P1, P2);
			R(__AnEmptyClass::* dm)(P0, P1, P2);
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	// Four parameters
	template <typename P0, typename P1, typename P2, typename P3>
	class ClassMethodBind4 : public ClassMethodBind
	{
	public:
		void (__AnEmptyClass::* method)(P0, P1, P2, P3);

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			P0 p0 = variant_cast<P0>(*args[0]);
			P1 p1 = variant_cast<P1>(*args[1]);
			P2 p2 = variant_cast<P2>(*args[2]);
			P3 p3 = variant_cast<P3>(*args[3]);
			(instance->*method)(p0, p1, p2, p3);

			return Variant();
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 2);
			P1 p1 = lua_getvalue<P1>(luaState, 3);
			P2 p2 = lua_getvalue<P2>(luaState, 4);
			P3 p3 = lua_getvalue<P3>(luaState, 5);

			// exec method
			(instance->*method)(p0, p1, p2, p3);

			lua_freevalue<P0>(p0);
			lua_freevalue<P1>(p1);
			lua_freevalue<P2>(p2);
			lua_freevalue<P3>(p3);

			// return number of results
			return 0;
		}
	};

	template<typename T, typename P0, typename P1, typename P2, typename P3>
	ClassMethodBind* createMethodBind(void(T::* method)(P0, P1, P2, P3))
	{
		ClassMethodBind4<P0, P1, P2, P3>* bind = new (ClassMethodBind4<P0, P1, P2, P3>);

		union
		{
			void (T::* sm)(P0, P1, P2, P3);
			void (__AnEmptyClass::* dm)(P0, P1, P2, P3);
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	template <typename R, typename P0, typename P1, typename P2, typename P3>
	class ClassMethodBind4R : public ClassMethodBind
	{
	public:
		R(__AnEmptyClass::* method)(P0, P1, P2, P3);

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			P0 p0 = variant_cast<P0>(*args[0]);
			P1 p1 = variant_cast<P1>(*args[1]);
			P2 p2 = variant_cast<P2>(*args[2]);
			P3 p3 = variant_cast<P3>(*args[3]);
			return (instance->*method)(p0, p1, p2, p3);
		}

		virtual int call(Object* obj, lua_State* luaState) override
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			//check and fetch the arguments
			P0 p0 = lua_getvalue<P0>(luaState, 2);
			P1 p1 = lua_getvalue<P1>(luaState, 3);
			P2 p2 = lua_getvalue<P2>(luaState, 4);
			P3 p3 = lua_getvalue<P3>(luaState, 5);

			// exec method
			R result = (instance->*method)(p0, p1, p2, p3);

			// free value
			lua_freevalue<P0>(p0);
			lua_freevalue<P1>(p1);
			lua_freevalue<P2>(p2);
			lua_freevalue<P3>(p3);

			// push the results
			lua_pushvalue<R>(luaState, result);

			// return number of results
			return 1;
		}
	};

	template<typename T, typename R, typename P0, typename P1, typename P2, typename P3>
	ClassMethodBind* createMethodBind(R(T::* method)(P0, P1, P2, P3))
	{
		ClassMethodBind4R<R, P0, P1, P2, P3>* bind = new (ClassMethodBind4R<R, P0, P1, P2, P3>);

		union
		{
			R(T::* sm)(P0, P1, P2, P3);
			R(__AnEmptyClass::* dm)(P0, P1, P2, P3);
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}
}
