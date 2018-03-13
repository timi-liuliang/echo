#pragma once

#include "object.h"
#include "engine/core/Util/StringUtil.h"

namespace Echo
{
	struct ObjectFactory
	{
		String	m_name;
		String	m_parent;

		virtual Object* create() = 0;
	};

	template<typename T>
	struct ObjectFactoryT : public ObjectFactory
	{
		ObjectFactoryT(const String& name, const String& parent)
		{
			m_name = name;
			m_parent = parent;

			Class::addClass(name, this);
		}

		virtual Object* create()
		{
			return EchoNew(T);
		}
	};

	class Class
	{
	public:
		template<typename T>
		static void registerType()
		{
			T::initClassInfo();
		}

		template<typename T>
		static T create(const char* className)
		{
			Object* obj = create(className);
			if (obj)
			{
				return ECHO_DOWN_CAST<T>(obj);
			}

			return nullptr;
		}

		static Object* create(const char* className);

		// add class
		static void addClass(const String& className, ObjectFactory* objFactory);

		// get all child class
		static bool getChildClasses(StringArray& childClasses, const char* className);
	};
}

#define ECHO_CLASS(m_class, m_parent)											\
public:																			\
	static void initClassInfo()													\
{																				\
	static Echo::ObjectFactoryT<m_class> G_OBJECT_FACTORY(#m_class, #m_parent);	\
}																				\
private: