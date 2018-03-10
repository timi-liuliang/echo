#include "KeyValues.h"

namespace Echo
{
	// 导出到字符串
	void KeyValues::exportToStr(String& str)
	{
		str.clear();

		for (auto& it : m_keyValues)
		{
			str = str + it.first + ":" + it.second + ";";
		}
	}

	// 根据字符串初始化
	void KeyValues::parseFromStr(const String& str)
	{
		StringArray keyVaues = StringUtil::Split(str, ";");
		for (const String& keyValue : keyVaues)
		{
			StringArray kv = StringUtil::Split(keyValue, ":");
			if (kv.size() > 1)
				m_keyValues[kv[0]] = kv[1];
		}
	}

	// 获取值
	const String& KeyValues::getValueStr(const String& key)const
	{
		map<String, String>::type::const_iterator it = m_keyValues.find(key);
		if (it != m_keyValues.end())
		{
			return it->second;
		}

		return StringUtil::BLANK;
	}

	bool KeyValues::getValueBool(const String& key, bool defaultValue) const
	{
		const String& value = getValueStr(key);
		if (value != StringUtil::BLANK)
		{
			return StringUtil::ParseBool(value);
		}

		return defaultValue;
	}

	i32 KeyValues::getValueI32(const String& key, i32 defaultValue)const
	{
		const String& value = getValueStr(key);
		if (value != StringUtil::BLANK)
		{
			return StringUtil::ParseI32(value);
		}

		return defaultValue;
	}

	float KeyValues::getValueFloat(const String& key, float defaultValue) const
	{
		const String& value = getValueStr(key);
		if (value != StringUtil::BLANK)
		{
			return StringUtil::ParseFloat(value);
		}

		return defaultValue;
	}

	const Vector3& KeyValues::getValueVec3(const String& key, const Vector3& defaultValue)const
	{
		const String& value = getValueStr(key);
		if (value != StringUtil::BLANK)
		{
			return StringUtil::ParseVec3(value);
		}

		return defaultValue;
	}

	// 设置值
	void KeyValues::setValue(const String& key, const String& value)
	{
		m_keyValues[key] = value;
	}

	void KeyValues::setValue(const String& key, bool value)
	{
		m_keyValues[key] = StringUtil::ToString(value);
	}

	// 设置值
	void KeyValues::setValue(const String& key, float value)
	{
		m_keyValues[key] = StringUtil::ToString(value);
	}

	// 设置值
	void KeyValues::setValue(const String& key, i32 value)
	{
		m_keyValues[key] = StringUtil::ToString(value);
	}

	void KeyValues::setValue(const String& key, const Vector3& value)
	{
		m_keyValues[key] = StringUtil::ToString(value);
	}
}