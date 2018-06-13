#include "object.h"

namespace Echo
{
	Object::Object()
	{
	}

	// get class name
	const String& Object::getClassName() const
	{
		static String className = "Object";
		return className;
	}

	// propertys (script property or dynamic property)
	const PropertyInfos& Object::getPropertys() const 
	{ 
		return m_propertys; 
	}

	// register property
	bool Object::registerProperty(const String& className, const String& propertyName, const Variant::Type type)
	{
		PropertyInfoDynamic* info = EchoNew(PropertyInfoDynamic);
		info->m_name = propertyName;
		info->m_type = type;
		info->m_className = className;

		m_propertys.push_back(info);

		return true;
	}
}