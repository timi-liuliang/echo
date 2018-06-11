#include "variant.h"
#include "object.h"

namespace Echo
{
	const Variant Variant::INVALID;

	Variant::Variant(int value)
		: m_type(Type::Int)
	{
		m_int = value;
	}

	Variant::Variant(const Echo::String& str)
		: m_type(Type::String)
	{
		m_any = str;
	}

	Variant::Variant(const Echo::Vector3& value)
	{
		m_type = Type::Vector3;
		m_vec3 = value;
	}

	Variant::Variant(const Base64String& value)
		: m_type(Type::Base64String)
	{
		m_any = value;
	}

	Variant::Variant(const StringOption& value)
		: m_type(Type::StringOption)
	{
		m_any = value;
	}

	Variant::Variant(const ResourcePath& value)
		: m_type(Type::ResourcePath)
	{
		m_any = value;
	}

	Variant::Variant(Object* value)
		: m_type(Type::Res)
	{
		m_obj = value;
	}

	Variant::~Variant()
	{

	}

	Variant::Variant(const Variant &orig)
	{
		m_type = orig.m_type;
		m_any  = orig.m_any;
		m_vec3 = orig.m_vec3;
	}

	// operator "="
	Variant&  Variant::operator=(const Variant& orig)
	{
		m_type = orig.m_type;
		m_any = orig.m_any;
		m_vec3 = orig.m_vec3;

		return *this;
	}

	// to string
	Echo::String Variant::toString() const
	{
		switch (m_type)
		{
		case Type::Int: return StringUtil::ToString(m_int);
		case Type::String: return any_cast<String>(m_any);
		case Type::Vector3: return StringUtil::ToString(m_vec3);
		case Type::ResourcePath: return (any_cast<ResourcePath>(m_any)).getPath();
		case Type::StringOption: return (any_cast<StringOption>(m_any)).getValue();
		}

		static Echo::String invalid;
		return invalid; 
	}

	// from string
	bool Variant::fromString(Type type, const String& str)
	{
		switch (type)
		{
		case Echo::Variant::Type::Int: m_int = StringUtil::ParseI32(str); return true;
		case Echo::Variant::Type::Vector3: { m_type = Type::Vector3; m_vec3 = StringUtil::ParseVec3(str); }return true;
		case Echo::Variant::Type::ResourcePath: { m_type = Type::ResourcePath; m_any = ResourcePath(str, nullptr); }return true;
		case Echo::Variant::Type::StringOption: { m_type = Type::StringOption; m_any = StringOption(str, nullptr); } return true;
		}

		return false;
	}
}