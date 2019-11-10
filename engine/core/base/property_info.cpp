#include "property_info.h"
#include "object.h"

namespace Echo
{
    const String& PropertyInfo::getHint(PropertyHintType hintType) const
    {
        for(const PropertyHint& hint : m_hints)
        {
            if(hint.m_type == hintType)
                return hint.m_value;
        }
        
        return StringUtil::BLANK;
    }

	bool PropertyInfoStatic::getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar)
	{
		Variant::CallError error;
		oVar = m_getterMethod->call(classPtr, nullptr, 0, error);

		return true;
	}

	void PropertyInfoStatic::setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue)
	{
		Variant::CallError error;
		const Variant* args[1] = { &propertyValue };
		m_setterMethod->call(classPtr, args, 1, error);
	}

	bool PropertyInfoDynamic::getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar)
	{
		if (classPtr->getPropertyValue(propertyName, oVar))
			return true;

		return false;
	}

	void PropertyInfoDynamic::setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue)
	{
		classPtr->setPropertyValue(propertyName, propertyValue);
	}
}
