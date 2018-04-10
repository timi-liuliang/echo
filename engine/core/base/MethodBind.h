#pragma once

#include "variant.h"

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
	class MethodBind
	{
	public:
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error) = 0;
	};
	// please use hash map
	typedef std::map<String, MethodBind*>	MethodMap;

	// declare a nonexistent class..
	//class __UnexistingClass;
	template<typename T>
	class MethodBind0 : public MethodBind
	{
	public:
		void (T::*method)();
		
		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error)
		{
			T* instance = (T*)obj;

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
			void (__UnexistingClass::*dm)();
		} u;
		u.sm = method;
		bind->method = u.dm;

		return bind;
	}

	class __UnexistingClass;
	template<typename R, typename T>
	class MethodBind0R : public MethodBind
	{
	public:
		R (T::*method)();

		// exec the method
		virtual Variant call(Object* obj, const Variant** args, int argCount, Variant::CallError& error)
		{
			T* instance = (T*)obj;

			R ret = (instance->*method)();

			return Variant(ret);
		}
	};

	template<typename T, typename R>
	MethodBind* createMethodBind(R(T::*method)())
	{
		MethodBind0R<R, T> * bind = new (MethodBind0R<R, T>);

		//union
		//{
		//	R (T::*sm)();
		//	R (__UnexistingClass::*dm)();
		//} u;
		//u.sm = method;
		bind->method = method;

		return bind;
	}
}