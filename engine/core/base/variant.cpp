#include "variant.h"

namespace Echo
{
	const Variant Variant::INVALID;

	Variant::Variant(const Echo::String& str)
		: m_type(String)
		, m_str(nullptr)
	{
		copyStr(str.data(), str.size());
	}

	Variant::~Variant()
	{

	}

	Variant::Variant(const Variant &orig)
	{
		m_type = orig.m_type;

		switch (orig.m_type)
		{
		case String: copyStr(orig.m_str, strlen(orig.m_str)); break;
		}
	}

	// operator "="
	Variant&  Variant::operator=(const Variant& orig)
	{
		m_type = orig.m_type;

		switch (orig.m_type) 
		{
		case String: copyStr(orig.m_str, strlen(orig.m_str)); break;
		}

		return *this;
	}

	// to string
	Echo::String Variant::toString() 
	{
		switch (m_type)
		{
		case String: return m_str;
		}

		static Echo::String invalid;
		return invalid; 
	}

	// copy string
	void Variant::copyStr(const char* str, int size)
	{
		if (m_type == String && m_str)
		{
			delete[] m_str;
		}

		m_str = new char[size + 1];
		std::memcpy(m_str, str, size);
		m_str[size] = '\0';
	}
}