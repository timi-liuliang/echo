#include "class.h"
#include <map>

namespace Echo
{
	static std::map<String, ObjectFactory*>*	g_classInfos = nullptr;

	// add class
	void Class::addClass(const String& className, ObjectFactory* objFactory)
	{
		if (!g_classInfos)
		{
			g_classInfos = new std::map<String, ObjectFactory*>();
		}

		(*g_classInfos)[className] = objFactory;
	}

	// get parent class name
	bool Class::getParentClass(String& parentClassName, const String& className)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			parentClassName = it->second->m_classInfo.m_parent;
			return true;
		}

		return false;
	}

	// get all child class
	bool Class::getChildClasses(StringArray& childClasses, const String& className)
	{
		// keep clean
		childClasses.clear();

		if (!g_classInfos)
			return false;

		for (auto it : *g_classInfos)
		{
			if (it.second->m_classInfo.m_parent == className)
			{
				childClasses.push_back(it.first);
			}
		}

		return !childClasses.empty();
	}

	Object* Class::create(const char* className)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			Object* obj = it->second->create();
			if (obj)
				obj->setName(className);

			return obj;
		}

		return nullptr;
	}

	// register method
	bool Class::registerMethodBind(const String& className, const String& methodName, MethodBind* method)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			it->second->registerMethod( methodName, method);
		}

		return true;
	}

	// get method
	MethodBind* Class::getMethodBind(const String& className, const String& methodName)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			return it->second->getMethodBind(methodName);
		}

		return nullptr;
	}

	// add property
	bool Class::registerProperty(const String& className, const String& propertyName, const Variant::Type type, const String& getter, const String& setter)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			PropertyInfo info;
			info.m_name = propertyName;
			info.m_type = type;
			info.m_setter = setter;
			info.m_getter = getter;
			info.m_setterMethod = getMethodBind(className, setter);
			info.m_getterMethod = getMethodBind(className, getter);

			it->second->registerProperty(info);
		}

		return true;
	}

	// get propertys
	const PropertyInfos& Class::getPropertys(const String& className)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			return it->second->getPropertys();
		}

		static PropertyInfos invalid;
		return invalid;
	}

	// get property value
	bool Class::getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar)
	{
		String className = classPtr->getClassName();
		do 
		{		
			if (getPropertyValue(classPtr, className, propertyName, oVar))
				return true;

		} while (getParentClass( className, className));

		return false;
	}

	// get property value by class name
	bool Class::getPropertyValue(Object* classPtr, const String& className, const String& propertyName, Variant& oVar)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			ObjectFactory* objFactory = it->second;
			return objFactory->getPropertyValue(classPtr, propertyName, oVar);
		}

		return false;
	}

	// set property value
	bool Class::setPropertyValue(const Object* classPtr, const String& className, const String& propertyValue)
	{
		return false;
	}
}