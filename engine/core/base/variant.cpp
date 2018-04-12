#include "variant.h"

namespace Echo
{
	const Variant Variant::INVALID;

	Variant::Variant(const Echo::String& str)
		: m_type(Type_String)
		, m_str(nullptr)
	{
		copyStr(str.data(), str.size());
	}

	Variant::Variant(const Echo::Vector3& value)
	{
		m_type = Type_Vector3;
		m_vec3 = value;
	}

	Variant::~Variant()
	{

	}

	Variant::Variant(const Variant &orig)
	{
		m_type = orig.m_type;

		switch (orig.m_type)
		{
		case Type_String: copyStr(orig.m_str, strlen(orig.m_str)); break;
		}
	}

	// operator "="
	Variant&  Variant::operator=(const Variant& orig)
	{
		m_type = orig.m_type;

		switch (orig.m_type) 
		{
		case Type_String: copyStr(orig.m_str, strlen(orig.m_str)); break;
		case Type_Vector3: m_vec3 = orig.m_vec3;				   break;
		}

		return *this;
	}

	// to string
	Echo::String Variant::toString() const
	{
		switch (m_type)
		{
		case Type_String: return m_str;
		case Type_Vector3: return StringUtil::ToString(m_vec3);
		}

		static Echo::String invalid;
		return invalid; 
	}

	// copy string
	void Variant::copyStr(const char* str, int size)
	{
		if (m_type == Type_String && m_str)
		{
			EchoSafeFree(m_str);
		}

		m_str = (char*)EchoMalloc(size + 1);
		std::memcpy(m_str, str, size);
		m_str[size] = '\0';
	}
}