#pragma once

#include "variant.h"

namespace Echo
{
	enum PropertyFlag
	{
		Save = 1 << 0,		// property will be saved to files
		All = Save,
	};

	enum class PropertyHintType
	{
		None,
		Range,				// variable range
        Category,           // property category
		ResourceType,		// resource object type
		ReadOnly,
	};

    struct PropertyHint
    {
        PropertyHintType    m_type;
        String              m_value;

		PropertyHint(PropertyHintType type, const String& value)
			: m_type(type), m_value(value)
		{}
    };
    typedef vector<PropertyHint>::type PropertyHintArray;

	class Object;
	struct PropertyInfo
	{
		String			    m_name;
		Variant::Type	    m_type;
        PropertyHintArray   m_hints;
		enum Type
		{
			Static = 1<<0,
			Dynamic = 1<<1,
			Script = 1<<2,
			All = Static | Dynamic | Script
		}				m_infoType;

		PropertyInfo(Type type) : m_infoType(type) {}
		virtual ~PropertyInfo() {}

		// get property value
		virtual bool getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar) = 0;
		virtual bool getPropertyValueDefault(Object* classPtr, const String& propertyName, Variant& oVar) = 0;

		// set property value
		virtual void setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue) = 0;

		// get flag
		virtual i32 getPropertyFlag(Object* classPtr, const String& propertyName) { return PropertyFlag::All; }

        // get property hint
        const String& getHint(PropertyHintType hintType) const;

		// add hint
		void addHint( PropertyHint hint) { m_hints.emplace_back(hint); }

		// add hint
		template<typename ... T>
		void addHint( PropertyHint hint, T... args)
		{
			addHint(hint);
			addHint(args...);
		}
	};
	typedef vector<PropertyInfo*>::type PropertyInfos;

	class ClassMethodBind;
	struct PropertyInfoStatic : public PropertyInfo
	{
		String				m_getter;
		String				m_setter;
		ClassMethodBind*	m_getterMethod;
		ClassMethodBind*	m_setterMethod;

		PropertyInfoStatic()
			: PropertyInfo(Type::Static)
		{}

		// get property value
		virtual bool getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar) override;
		virtual bool getPropertyValueDefault(Object* classPtr, const String& propertyName, Variant& oVar) override;

		// set property value
		virtual void setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue) override;
	};

	struct PropertyInfoDynamic : public PropertyInfo
	{
		String	m_className;

		PropertyInfoDynamic() : PropertyInfo(Type::Dynamic) {}

		// get property value
		virtual bool getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar) override;
		virtual bool getPropertyValueDefault(Object* classPtr, const String& propertyName, Variant& oVar) override;

		// set property value
		virtual void setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue) override;

		// get flag
		virtual i32 getPropertyFlag(Object* classPtr, const String& propertyName) override;
	};
}
