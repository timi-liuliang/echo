#include "property_info.h"
#include "object.h"

namespace Echo
{
	// get property value
	bool PropertyInfoStatic::getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar)
	{
		Variant::CallError error;
		oVar = m_getterMethod->call(classPtr, nullptr, 0, error);

		return true;
	}

	// set property value
	void PropertyInfoStatic::setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue)
	{
		Variant::CallError error;
		const Variant* args[1] = { &propertyValue };
		m_setterMethod->call(classPtr, args, 1, error);
	}

	// get property value
	bool PropertyInfoDynamic::getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar)
	{
		if (classPtr->getPropertyValue(propertyName, oVar))
			return true;

		return false;
	}

	// set property value
	void PropertyInfoDynamic::setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue)
	{
		classPtr->setPropertyValue(propertyName, propertyValue);
	}
}