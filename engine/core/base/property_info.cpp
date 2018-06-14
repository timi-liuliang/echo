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

	// get property value
	bool PropertyInfoDynamic::getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar)
	{
		if (classPtr->getPropertyValue(propertyName, oVar))
			return true;

		return false;
	}
}