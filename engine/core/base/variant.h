#pragma once

#include "TypeDef.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class Variant
	{
	public:
		enum Type
		{
			Nil,
			Bool,
			Real,
			Vector3,
			String,
		};

		struct CallError
		{
			enum Error
			{
				CE_Ok,
				CE_InvalidMethod,
				CE_InvalidArgument,
				CE_TooManyArguments,
				CE_ToolFewArguments,
				CE_InstanceIsNull,
			};
		};

		static const Variant INVALID;

	public:
		Variant():m_type(Nil), m_str(nullptr){}
		Variant(const Echo::String& str);
		~Variant();

		// to string
		Echo::String toString();

		// operator "="
		Variant& operator=(const Variant& orig);
		Variant(const Variant &orig);

	private:
		// copy string
		void copyStr(const char* str, int size);

	private:
		Type			m_type;

		union 
		{
			bool			m_bool;
			i64				m_int;
			float			m_real;
			char*			m_str;
		};
	};
}