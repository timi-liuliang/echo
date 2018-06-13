#pragma once

#include "variant.h"
#include "MethodBind.h"

namespace Echo
{
	struct PropertyInfo
	{
		String			m_name;
		Variant::Type	m_type;
		enum class Type
		{
			Static,
			Dynamic,
			Script,
		}				m_infoType;

		PropertyInfo(Type type) : m_infoType(type) {}
		virtual ~PropertyInfo() {}
	};
	typedef vector<PropertyInfo*>::type PropertyInfos;

	struct PropertyInfoStatic : public PropertyInfo
	{
		String			m_getter;
		String			m_setter;
		MethodBind*		m_getterMethod;
		MethodBind*		m_setterMethod;

		PropertyInfoStatic()
			: PropertyInfo(Type::Static)
		{}
	};

	struct PropertyInfoDynamic : public PropertyInfo
	{
		String	m_className;

		PropertyInfoDynamic()
			: PropertyInfo(Type::Dynamic)
		{}
	};
}