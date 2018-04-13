#pragma once

#include "TypeDef.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/math/Vector4.h"

namespace Echo
{
	class Variant
	{
	public:
		enum Type
		{
			Type_Nil,
			Type_Bool,
			Type_Real,
			Type_Vector3,
			Type_String,
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
		Variant():m_type(Type_Nil), m_str(nullptr){}
		Variant(const String& str);
		Variant(const Vector3& value);
		~Variant();

		// to string
		Echo::String toString() const;
		
		// from string
		bool fromString(Type type, const String& str);

		// operator "="
		Variant& operator=(const Variant& orig);
		Variant(const Variant &orig);

		// type
		Type getType() const { return m_type; }

		const Vector3& getValue() const { return m_vec3; }

		// is nil
		bool isNil() const { return m_type == Type_Nil; }

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
			Vector3			m_vec3;
			char*			m_str;
		};
	};
}