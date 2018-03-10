#pragma once

#include "engine/core/Util/StringUtil.h"

namespace Echo
{
	class Class
	{
	public:
		template<typename T>
		static void registerType()
		{
			T::initClassInfo();
		}

		// add class
		static void addClass(const char* className, const char* parentName);

		// get all child class
		static bool getChildClasses(StringArray& childClasses, const char* className);
	};
}

#define ECHO_CLASS(m_class, m_parent)					\
public:													\
static void initClassInfo()								\
{														\
	Class::addClass(#m_class, #m_parent);				\
}														\
private: