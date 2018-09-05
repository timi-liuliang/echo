#pragma once

#include "variant.h"

namespace Echo
{
	enum class PropertyHint
	{
		None,
		Range,				// variable range
		ResourceType,		// resource object type
	};

	class Object;
	struct PropertyInfo
	{
		String			m_name;
		Variant::Type	m_type;
		PropertyHint	m_hint;
		String			m_hintStr;
		enum Type
		{
			Static = 1<<0,
			Dynamic = 1<<1,
			Script = 1<<2,
		}				m_infoType;

		PropertyInfo(Type type) : m_infoType(type) {}
		virtual ~PropertyInfo() {}

		// get property value
		virtual bool getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar) = 0;

		// set property value
		virtual void setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue) = 0;
	};
	typedef vector<PropertyInfo*>::type PropertyInfos;

	class ClassMethodBind;
	struct PropertyInfoStatic : public PropertyInfo
	{
		String			m_getter;
		String			m_setter;
		ClassMethodBind*		m_getterMethod;
		ClassMethodBind*		m_setterMethod;

		PropertyInfoStatic()
			: PropertyInfo(Type::Static)
		{}

		// get property value
		virtual bool getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar);

		// set property value
		virtual void setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue);
	};

	struct PropertyInfoDynamic : public PropertyInfo
	{
		String	m_className;

		PropertyInfoDynamic() : PropertyInfo(Type::Dynamic) {}

		// get property value
		virtual bool getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar);

		// set property value
		virtual void setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue);
	};
}