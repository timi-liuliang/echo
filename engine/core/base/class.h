#pragma once

#include "object.h"
#include "variant.h"
#include "MethodBind.h"
#include "property_info.h"
#include "engine/core/Util/StringUtil.h"
#include "engine/core/script/lua/luaex.h"

namespace Echo
{
	struct ClassInfo
	{
		String			m_parent;
		PropertyInfos	m_propertyInfos;
		MethodMap		m_methods;
	};

	struct ObjectFactory
	{
		String		m_name;
		ClassInfo	m_classInfo;

		virtual Object* create() = 0;

		// register property
		void registerProperty(PropertyInfo* property)
		{
			PropertyInfo* pi = getProperty(property->m_name);
			if (!pi)
			{
				m_classInfo.m_propertyInfos.push_back(property);
			}
			else
			{
				EchoLogError("property [%s] already exist", property->m_name.c_str());
			}
		}

		// register method
		void registerMethod(const String& methodName, MethodBind* method)
		{
			m_classInfo.m_methods[methodName] = method;
		}

		// return method bind
		MethodBind* getMethodBind(const String& methodName)
		{
			auto it = m_classInfo.m_methods.find(methodName);
			if (it != m_classInfo.m_methods.end())
			{
				return it->second;
			}

			return nullptr;
		}

		// get propertys
		const PropertyInfos& getPropertys()
		{
			return m_classInfo.m_propertyInfos;
		}

		// get property
		PropertyInfo* getProperty(const String& propertyName)
		{
			for (PropertyInfo* pi : m_classInfo.m_propertyInfos)
			{
				if (pi->m_name == propertyName)
				{
					return pi;
				}
			}

			return nullptr;
		}
	};

	template<typename T>
	struct ObjectFactoryT : public ObjectFactory
	{
		ObjectFactoryT(const String& name, const String& parent)
		{
			// register class to lua
			luaex::LuaEx::instance()->register_class(name.c_str(), parent.c_str());

			m_name = name;
			m_classInfo.m_parent = parent;

			Class::addClass(name, this);
			T::bindMethods();
		}

		virtual Object* create()
		{
			return EchoNew(T);
		}
	};

	class Class
	{
	public:
		template<typename T>
		static void registerType()
		{
			T::initClassInfo();
		}

		template<typename T>
		static T create(const String& className)
		{
			Object* obj = create(className);
			if (obj)
			{
				return ECHO_DOWN_CAST<T>(obj);
			}

			return nullptr;
		}

		static Object* create(const String& className);

		// add class
		static void addClass(const String& className, ObjectFactory* objFactory);

		// get parent class name
		static bool getParentClass(String& parentClassName, const String& className);

		// get all child class
		static bool getChildClasses(StringArray& childClasses, const String& className, bool recursive);

		// register method
		static bool registerMethodBind(const String& className, const String& methodName, MethodBind* method);

		// get method
		static MethodBind* getMethodBind(const String& className, const String& methodName);

		// add property
		static bool registerProperty(const String& className, const String& propertyName, const Variant::Type type, const String& getter, const String& setter);

		// get propertys
		static ui32 getPropertys(const String& className, Object* classPtr, PropertyInfos& propertys);

		// get property
		static PropertyInfo* getProperty(const String& className, Object* classPtr, const String& propertyName);

		// get property value
		static bool getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar);

		// get property type
		static Variant::Type getPropertyType(Object* classPtr, const String& propertyName);

		// set property value
		static bool setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue);

		// bind method
		template<typename N, typename M>
		static MethodBind* bindMethod(const String& className, M method, N methodName)
		{
			MethodBind* bind = createMethodBind(method);

			registerMethodBind(className, methodName, bind);

			return bind;
		}
	};
}

#define ECHO_CLASS_NAME(m_class) #m_class

#define ECHO_CLASS(m_class, m_parent)												\
public:																				\
	virtual const String& getClassName() const										\
	{																				\
		static String className=#m_class;											\
		return className;															\
	}																				\
																					\
	static void initClassInfo()														\
	{																				\
		static Echo::ObjectFactoryT<m_class> G_OBJECT_FACTORY(#m_class, #m_parent);	\
	}																				\
																					\
	static void bindMethods();														\
																					\
private:															

#define CLASS_BIND_METHOD(m_class, method, methodName) \
	Echo::Class::bindMethod(#m_class, &##m_class::method, methodName)

#define CLASS_REGISTER_PROPERTY(m_class, name, type, getter, setter) \
	Echo::Class::registerProperty(#m_class, name, type, getter, setter)
