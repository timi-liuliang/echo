#pragma once

#include "TypeDef.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/math/Vector4.h"
#include "engine/core/render/render/Color.h"
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
			Vector2,
			Vector3,
			Color,
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
		Variant(bool value);
		Variant(int value);
		Variant(Real value);
		Variant(const String& str);
		Variant(const Vector2& value);
		Variant(const Vector3& value);
		Variant(const Color& value);
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
		operator const bool() const { return m_bool; }
		operator const int() const { return m_int; }
		operator const Real() const { return m_real; }
		operator const Vector2&() const { return any_cast<Vector2>(m_any); }
		operator const Vector3&() const { return any_cast<Vector3>(m_any); }
		operator const Color&() const { return any_cast<Color>(m_any); }
		operator const char*() const { return any_cast<String>(m_any).c_str(); }
		operator const String&() const { return any_cast<String>(m_any); }
		operator const ResourcePath&() const { return any_cast<ResourcePath>(m_any); }
		operator const Base64String&() const { return any_cast<Base64String>(m_any); }
		operator const StringOption&() const { return any_cast<StringOption>(m_any); }
		operator Object*() const { return m_obj; }

		// convert to other type
		const bool toBool() const { return m_bool; }
		const Real& toReal() const { return m_real; }
		const Vector2& toVector2() const { return any_cast<Vector2>(m_any); }
		const Vector3& toVector3() const { return any_cast<Vector3>(m_any); }
		const Color& toColor() const { return any_cast<Color>(m_any); }
		Object* toObj() const { return m_obj; }
		const ResourcePath& toResPath() const { return any_cast<ResourcePath>(m_any); }
		const StringOption& toStringOption() const { return any_cast<StringOption>(m_any); }

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
			mutable Object*	m_obj;
		};
	};
}