#pragma once

#include "type_def.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/math/Vector4.h"
#include "engine/core/math/matrix.h"
#include "engine/core/math/color.h"
#include "engine/core/resource/ResourcePath.h"
#include "engine/core/util/Any.hpp"
#include "engine/core/util/base64.h"
#include "engine/core/util/StringOption.h"
#include "engine/core/scene/node_path.h"

namespace Echo
{
	class Object;
    class Node;
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
			Vector4,
			VectorN,
			Matrix4,
			MatrixN,
			Color,
			String,
			ResourcePath,
			NodePath,
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
		Variant(ui32 value);
		Variant(Real value);
		Variant(const String& str);
		Variant(const Vector2& value);
		Variant(const Vector3& value);
		Variant(const Vector4& value);
		Variant(const Matrix& value);
		Variant(const Color& value);
		Variant(const ResourcePath& value);
		Variant(const NodePath& value);
		Variant(const Base64String& value);
		Variant(const StringOption& value);
		Variant(Object* value);
		Variant(const RealVector& value);
		~Variant();

		// operator "="
		Variant& operator=(const Variant& orig);
		Variant(const Variant &orig);

		// type
		Type getType() const { return m_type; }

		// reimplent operator
		operator const bool() const { return m_bool; }
		operator const int() const { return m_int; }
		operator const ui32() const { return m_uint; }
		operator const Real() const { return m_real; }
		operator const Vector2&() const { return any_cast<Vector2>(m_any); }
		operator const Vector3&() const { return any_cast<Vector3>(m_any); }
		operator const Vector4&() const { return any_cast<Vector4>(m_any); }
		operator const Matrix&() const  { return any_cast<Matrix>(m_any); }
		operator const Color&() const { return any_cast<Color>(m_any); }
		operator const char*() const { return any_cast<String>(m_any).c_str(); }
		operator const String&() const { return any_cast<String>(m_any); }
		operator const ResourcePath&() const { return any_cast<ResourcePath>(m_any); }
		operator const NodePath&() const { return any_cast<NodePath>(m_any); }
		operator const Base64String&() const { return any_cast<Base64String>(m_any); }
		operator const StringOption&() const { return any_cast<StringOption>(m_any); }
		operator Object*() const { return m_obj; }
		operator const RealVector&() const { return any_cast<RealVector>(m_any); }

		// convert to other type
		const bool toBool() const { return m_bool; }
		const Real& toReal() const { return m_real; }
		const Vector2& toVector2() const { return any_cast<Vector2>(m_any); }
		const Vector3& toVector3() const { return any_cast<Vector3>(m_any); }
		const Vector4& toVector4() const { return any_cast<Vector4>(m_any); }
		const Color& toColor() const { return any_cast<Color>(m_any); }
		Object* toObj() const { return m_obj; }
		const ResourcePath& toResPath() const { return any_cast<ResourcePath>(m_any); }
		const NodePath& toNodePath() const { return any_cast<NodePath>(m_any); }
		const StringOption& toStringOption() const { return any_cast<StringOption>(m_any); }
		const RealVector& toRealVector() const { return any_cast<RealVector>(m_any); }

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
			ui32			m_uint;
			float			m_real;
			mutable Object*	m_obj;
		};
	};
    
    template<typename T> INLINE T variant_cast(const Variant& variant)
    {
        return variant;
    }
    
    template<> INLINE Node* variant_cast(const Variant& variant)
    {
        Object* obj = variant.toObj();
        return (Node*)(obj);
    }
}
