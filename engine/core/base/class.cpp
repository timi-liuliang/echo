#include "class.h"
#include <map>
#include "engine/core/script/lua/LuaBind.h"

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
}