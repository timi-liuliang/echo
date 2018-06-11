#pragma once

#include "TypeDef.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/math/Vector4.h"
#include "engine/core/resource/ResourcePath.h"
#include "engine/core/util/Any.hpp"
#include "engine/core/util/base64.h"
#include "engine/core/util/StringOption.h"

namespace Echo
{
	class Object;
	class Variant
	{
	public:
		enum class Type
		{
			Nil,
			Bool,
			Int,
			Real,
			Vector3,
			String,
			ResourcePath,
			Base64String,
			StringOption,
			Object,
		};

		enum class CallError
		{
			Ok,
			InvalidMethod,
			InvalidArgument,
			TooManyArguments,
			ToolFewArguments,
			InstanceIsNull,
		};

		static const Variant INVALID;

	public:
		Variant():m_type(Type::Nil){}
		Variant(int value);
		Variant(const String& str);
		Variant(const Vector3& value);
		Variant(const ResourcePath& value);
		Variant(const Base64String& value);
		Variant(const StringOption& value);
		Variant(Object* value);
		~Variant();

		// operator "="
		Variant& operator=(const Variant& orig);
		Variant(const Variant &orig);

		// type
		Type getType() const { return m_type; }

		// reimplent operator
		operator const int() const { return m_int; }
		operator const Vector3&() const { return m_vec3; }
		operator const ResourcePath&() const { return any_cast<ResourcePath>(m_any); }
		operator const Base64String&() const { return any_cast<Base64String>(m_any); }
		operator const StringOption&() const { return any_cast<StringOption>(m_any); }
		operator const Object*() const { return m_obj; }

		// convert to other type
		const bool toVector3() { return m_bool; }
		const Vector3& toVector3() const { return m_vec3; }
		const ResourcePath& toResPath() const { return any_cast<ResourcePath>(m_any); }
		const StringOption& toStringOption() const { return any_cast<StringOption>(m_any); }
		const Object* toObj() const { return m_obj; }

		// is nil
		bool isNil() const { return m_type == Type::Nil; }

		// string convert
		Echo::String toString() const;
		bool fromString(Type type, const String& str);

	private:
		Type			m_type;
		any				m_any;

		union 
		{
			bool			m_bool;
			i32				m_int;
			float			m_real;
			Vector3			m_vec3;
			mutable Object*	m_obj;
		};
	};
}