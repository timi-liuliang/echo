#pragma once

#include "engine/core/base/variant.h"
#include "engine/core/math/Math.h"
#include "anim_curve.h"

namespace Echo
{
	class Node;
	struct AnimProperty
	{
		Echo::String				 m_name;														// name
		enum class Type
		{
			Unknown,
			Bool,
			Float,
			Vector3,
			Vector4,
			Quaternion,
			String,
			Object,
		}							 m_type;														// property type
		AnimCurve::InterpolationType m_interpolationType = AnimCurve::InterpolationType::Linear;	// interpolation type

		// constructor
		AnimProperty(Type type) : m_type(type) {}
        virtual ~AnimProperty() {}

		// create
		static AnimProperty* create(Type type);

		// set interpolation type
		virtual void setInterpolationType(AnimCurve::InterpolationType type);

		// optimize
		virtual void optimize() = 0;

		// update to time
		virtual void updateToTime(ui32 time, ui32 deltaTime) = 0;

		// get type
		Type getType() const { return m_type; }

		// get length
		virtual ui32 getLength() = 0;

		// get key value str
		virtual String getKeyValueStr(int idx) { return ""; }
	};

	struct AnimPropertyCurve : public AnimProperty
	{
		vector<AnimCurve*>::type m_curves;

		AnimPropertyCurve(Type type, i32 curveCount);
        virtual ~AnimPropertyCurve();

		// set interpolation type
		virtual void setInterpolationType(AnimCurve::InterpolationType type) override;

		// optimize
		virtual void optimize() override;

		// update to time
		virtual void updateToTime(ui32 time, ui32 deltaTime) override{}

		// get length
		virtual ui32 getLength() override;

		// set key value
		void setKeyValue(int curveIdx, int keyIdx, float value);

		// add key
		void addKeyToCurve(int curveIdx, ui32 time, float value);
	};

	struct AnimPropertyFloat : public AnimPropertyCurve
	{
		float		m_value;

		AnimPropertyFloat();

		// get value
		float getValue() { return m_value; }

		// add key
		void addKey(ui32 time, const float value);

		// update to time
		virtual void updateToTime(ui32 time, ui32 deltaTime) override;
	};

	struct AnimPropertyVec3 : public AnimPropertyCurve
	{
		Vector3		m_value;

		AnimPropertyVec3();

		// get value
		const Vector3& getValue() { return m_value; }

		// add key
		void addKey(ui32 time, const Vector3& value);

		// update to time
		virtual void updateToTime(ui32 time, ui32 deltaTime) override;
	};

	struct AnimPropertyVec4 : public AnimPropertyCurve
	{
		Vector4		m_value;

		AnimPropertyVec4();

		// get value
		const Vector4& getValue() { return m_value; }

		// add key
		void addKey(ui32 time, const Vector4& value);

		// update to time
		virtual void updateToTime(ui32 time, ui32 deltaTime) override;
	};

	struct AnimPropertyBool : public AnimProperty
	{
		typedef map<ui32, bool>::type KeyMap;

		KeyMap	m_keys;
		bool	m_value;
		bool	m_isActive = false;

		AnimPropertyBool() : AnimProperty(Type::Bool) {}

		// is active
		bool isActive() const { return m_isActive; }

		// get value
		bool getValue() { return m_value; }

		// add key
		void addKey(ui32 time, bool value);

		// correct data
		virtual void correct() {}

		// optimize
		virtual void optimize() override {}

		// update to time
		virtual void updateToTime(ui32 time, ui32 deltaTime) override;

		// get length
		virtual ui32 getLength() override;

		// start|end time
		ui32 getStartTime();
		ui32 getEndTime();
	};

	struct AnimPropertyQuat : public AnimProperty
	{
		struct Key
		{
			ui32		m_time;
			Quaternion	m_value;

			Key(ui32 time, const Quaternion& value)
				: m_time(time), m_value(value)
			{}
		};
		Quaternion				m_vlaue;
		vector<Key>::type		m_keys;

		AnimPropertyQuat() : AnimProperty(Type::Quaternion), m_vlaue(Quaternion::IDENTITY) {}

		// get value
		const Quaternion& getValue() { return m_vlaue; }

		// add key
		void addKey(ui32 time, const Quaternion& value);

		// correct data
		virtual void correct() {}

		// optimize
		virtual void optimize() override {}

		// update to time
		virtual void updateToTime(ui32 time, ui32 deltaTime) override;

		// get length
		virtual ui32 getLength() override;
	};

	struct AnimPropertyObject : public AnimProperty
	{
		AnimPropertyObject() : AnimProperty(Type::Object) {}

		// correct data
		virtual void correct() {}

		// optimize
		virtual void optimize() override {}

		// update to time
		virtual void updateToTime(ui32 time, ui32 deltaTime) override;

		// get length
		virtual ui32 getLength() override;
	};

	struct AnimPropertyString : public AnimProperty
	{
		typedef map<ui32, Echo::String>::type KeyMap;

		KeyMap			m_keys;
		Echo::String	m_value;
		bool			m_isActive = false;

		AnimPropertyString() : AnimProperty(Type::String) {}

		// is active
		bool isActive() const { return m_isActive; }

		// get value
		const String& getValue() { return m_value; }

		// add key
		void addKey(ui32 time, const String& value);

		// correct data
		virtual void correct() {}

		// optimize
		virtual void optimize() override {}

		// update to time
		virtual void updateToTime(ui32 time, ui32 deltaTime) override;

		// get length
		virtual ui32 getLength() override;

		// start|end time
		ui32 getStartTime();
		ui32 getEndTime();
	};
}
