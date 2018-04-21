#include "variant.h"

namespace Echo
{
	const Variant Variant::INVALID;

	Variant::Variant(const Echo::String& str)
		: m_type(Type_String)
	{
		m_any = str;
	}

	Variant::Variant(const Echo::Vector3& value)
	{
		m_type = Type_Vector3;
		m_vec3 = value;
	}

	Variant::Variant(const ResourcePath& value)
		: m_type(Type_ResourcePath)
	{
		m_any = value;
	}

	Variant::~Variant()
	{

	}

	Variant::Variant(const Variant &orig)
	{
		m_type = orig.m_type;

		switch (orig.m_type)
		{
		case Type_String:
		case Type_ResourcePath:	 m_any = orig.m_any; break;
		}
	}

	// operator "="
	Variant&  Variant::operator=(const Variant& orig)
	{
		m_type = orig.m_type;

		switch (orig.m_type) 
		{
		case Type_ResourcePath:
		case Type_String: m_any = orig.m_any; break;
		case Type_Vector3: m_vec3 = orig.m_vec3;				   break;
		}

		return *this;
	}

	// to string
	Echo::String Variant::toString() const
	{
		switch (m_type)
		{
		case Type_String: return any_cast<String>(m_any);
		case Type_Vector3: return StringUtil::ToString(m_vec3);
		case Type_ResourcePath: return (any_cast<ResourcePath>(m_any)).getPath();
		}

		static Echo::String invalid;
		return invalid; 
	}

	// from string
	bool Variant::fromString(Type type, const String& str)
	{
		switch (type)
		{
		case Echo::Variant::Type_Vector3: m_type = Type_Vector3; m_vec3 = StringUtil::ParseVec3(str); return true;
		}

		return false;
	}
}