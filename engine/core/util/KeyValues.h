#pragma once

#include "StringUtil.h"
#include "engine/core/Math/Vector3.h"

namespace Echo
{
	// 键值对
	struct KeyValues
	{
		map<String, String>::type m_keyValues;

		// 导出到字符串
		void exportToStr(String& str);

		// 根据字符串初始化
		void parseFromStr(const String& str);

		// 获取值
		const String& getValueStr(const String& key)const;
		bool getValueBool(const String& key, bool defaultValue) const;
		i32 getValueI32(const String& key, i32 defaultValue)const;
		float getValueFloat(const String& key, float defaultValue) const;
		const Vector3& getValueVec3(const String& key, const Vector3& defaultValue)const;

		// 设置值
		void setValue(const String& key, const String& value);
		void setValue(const String& key, bool value);
		void setValue(const String& key, i32 value);
		void setValue(const String& key, float value);
		void setValue(const String& key, const Vector3& value);

		// 清空
		void clear() { m_keyValues.clear(); }

		// 获取键值对数量
		i32 getSize() { return (i32)m_keyValues.size(); }
	};
}