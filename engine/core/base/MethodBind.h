#pragma once

#include "variant.h"

namespace Echo
{
	template<class T>
	struct VariantCaster
	{
		static T cast(const Variant& variant)
		{
			return variant;
		}
	};

	//template<class T>
	//struct VariantCaster
	//{
	//	static T cast(Object* obj)
	//	{
	//		return (T)obj;
	//	}
	//};

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
	class MethodBind
	{
	public:
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) = 0;
	};
	// please use hash map
	typedef std::map<String, MethodBind*>	MethodMap;

	// declare a empty class..
	class __AnEmptyClass{};

	class MethodBind0 : public MethodBind
	{
	public:
		void (__AnEmptyClass::*method)();
		
		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error)
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			(instance->*method)();

			return Variant();
		}
	};

	template<typename T>
	MethodBind* createMethodBind(void(T::*method)())
	{
		MethodBind0* bind = new(MethodBind0);

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
	class MethodBind0R : public MethodBind
	{
	public:
		R (__AnEmptyClass::*method)();

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error)
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			return (instance->*method)();
		}
	};

	template<typename T, typename R>
	MethodBind* createMethodBind(R(T::*method)())
	{
		MethodBind0R<R> * bind = new (MethodBind0R<R>);

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
	class MethodBind0RC : public MethodBind
	{
	public:
		R(__AnEmptyClass::*method)() const;

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error)
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			return (instance->*method)();
		}
	};

	template<typename T, typename R>
	MethodBind* createMethodBind(R(T::*method)() const)
	{
		MethodBind0RC<R> * bind = new (MethodBind0RC<R>);

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
	class MethodBind1 : public MethodBind
	{
	public:
		void (__AnEmptyClass::*method)(P0);

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error)
		{
			__AnEmptyClass* instance = (__AnEmptyClass*)obj;

			P0 p0 = VariantCaster<P0>::cast(*args[0]);
			(instance->*method)(p0);

			return Variant();
		}
	};

	template<typename T, typename P0>
	MethodBind* createMethodBind(void(T::*method)(P0))
	{
		MethodBind1<P0>* bind = new (MethodBind1<P0>);

		union
		{
			void (T::*sm)(P0);
			void (__AnEmptyClass::*dm)(P0);
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}
}