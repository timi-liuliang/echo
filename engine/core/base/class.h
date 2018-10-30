#pragma once

#include "object.h"
#include "variant.h"
#include "class_method_bind.h"
#include "property_info.h"
#include "engine/core/editor/object_editor.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/script/lua/lua_binder.h"

namespace Echo
{
	struct ClassInfo
	{
		bool			m_singleton;		// singleton class
		bool			m_virtual;			// virtual class can't be instanced
		String			m_parent;
		PropertyInfos	m_propertyInfos;
		ClassMethodMap	m_methods;
	};

	struct ObjectFactory
	{
		String		m_name;
		ClassInfo	m_classInfo;

		virtual Object* create() = 0;

		// register property
		void registerProperty(PropertyInfo* property);

		// register method
		void registerMethod(const String& methodName, ClassMethodBind* method)
		{
			m_classInfo.m_methods[methodName] = method;
		}

		// return method bind
		ClassMethodBind* getMethodBind(const String& methodName)
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
        
#ifdef ECHO_EDITOR_MODE
        void initEditor(Object* obj);
#endif
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

		// get all class names
		static size_t getAllClasses(StringArray& classes);

		// create
		static Object* create(const String& className);

		// add class
		static void addClass(const String& className, ObjectFactory* objFactory);

		// is derived from
		static bool isDerivedFrom(const String& className, const String& parentClassName);

		// is virtual
		static bool isVirtual(const String& className);

		// is singleton
		static bool isSingleton(const String& className);

		// get parent class name
		static bool getParentClass(String& parentClassName, const String& className);

		// get all child class
		static bool getChildClasses(StringArray& childClasses, const String& className, bool recursive);

		// register method
		static bool registerMethodBind(const String& className, const String& methodName, ClassMethodBind* method);

		// get method
		static ClassMethodBind* getMethodBind(const String& className, const String& methodName);

		// add property
		static bool registerProperty(const String& className, const String& propertyName, const Variant::Type type, PropertyHint hint, const String& hintStr, const String& getter, const String& setter);

		// get propertys
		static ui32 getPropertys(const String& className, Object* classPtr, PropertyInfos& propertys, i32 flag=PropertyInfo::Static | PropertyInfo::Dynamic);

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
		static ClassMethodBind* bindMethod(const String& className, M method, N methodName)
		{
			ClassMethodBind* bind = createMethodBind(method);

			registerMethodBind(className, methodName, bind);

			return bind;
		}
	};
    
    template<typename T>
    struct ObjectFactoryT : public ObjectFactory
    {
        ObjectFactoryT(const String& name, const String& parent, bool isVirtual=false)
        {
            // register class to lua
			LuaBinder::instance()->registerClass( name.c_str(), parent.c_str());
            
            m_name = name;
            m_classInfo.m_singleton = false;
            m_classInfo.m_virtual = isVirtual;
            m_classInfo.m_parent = parent;
            
            Class::addClass(name, this);
            T::bindMethods();
        }
        
        virtual Object* create()
        {
			Object* obj = EchoNew(T);

		#ifdef ECHO_EDITOR_MODE
            initEditor(obj);
		#endif

			return obj;
        }
    };
    
    template<typename T>
    struct ObjectFactorySingletonT : public ObjectFactory
    {
        ObjectFactorySingletonT(const String& name, const String& parent, bool isVirtual = false)
        {
            // register class to lua
            LuaBinder::instance()->registerClass(name, parent.c_str());
			LuaBinder::instance()->registerObject(name, name, T::instance());
            
            m_name = name;
            m_classInfo.m_singleton = true;
            m_classInfo.m_virtual = isVirtual;
            m_classInfo.m_parent = parent;
            
            Class::addClass(name, this);
            T::bindMethods();
        }
        
        virtual Object* create()
        {
            return T::instance();
        }
    };
}

#define ECHO_CLASS_NAME(m_class) #m_class

#define ECHO_CLASS(m_class, m_parent)														\
public:																						\
	virtual const String& getClassName() const												\
	{																						\
		static String className=#m_class;													\
		return className;																	\
	}																						\
																							\
	static void initClassInfo()																\
	{																						\
		static Echo::ObjectFactoryT<m_class> G_OBJECT_FACTORY(#m_class, #m_parent);			\
	}																						\
																							\
	static void bindMethods();																\
																							\
																							\
private:															

#define ECHO_VIRTUAL_CLASS(m_class, m_parent)												\
public:																						\
	virtual const String& getClassName() const												\
	{																						\
		static String className=#m_class;													\
		return className;																	\
	}																						\
																							\
	static void initClassInfo()																\
	{																						\
		static Echo::ObjectFactoryT<m_class> G_OBJECT_FACTORY(#m_class, #m_parent, true);	\
	}																						\
																							\
	static void bindMethods();																\
																							\
private:				


#define ECHO_SINGLETON_CLASS(m_class, m_parent)												\
public:																						\
	virtual const String& getClassName() const												\
	{																						\
		static String className=#m_class;													\
		return className;																	\
	}																						\
																							\
	static void initClassInfo()																\
	{																						\
		static Echo::ObjectFactorySingletonT<m_class>										\
		G_OBJECT_FACTORY(#m_class, #m_parent, true);										\
	}																						\
																							\
	static void bindMethods();																\
																							\
private:		

#define CLASS_BIND_METHOD(m_class, method, methodName) \
	Echo::Class::bindMethod(#m_class, &m_class::method, methodName)

#define CLASS_REGISTER_PROPERTY(m_class, name, type, getter, setter) \
	Echo::Class::registerProperty(#m_class, name, type, PropertyHint::None, "", getter, setter)

#define CLASS_REGISTER_PROPERTY_WITH_HINT(m_class, name, type, hint, hintStr, getter, setter) \
	Echo::Class::registerProperty(#m_class, name, type, hint, hintStr, getter, setter)
