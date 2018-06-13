#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/base/class.h"
#include "engine/core/base/property_info.h"

namespace Echo
{
	class Object
	{
	public:
		Object();
		virtual ~Object(){}

		// get class name
		virtual const String& getClassName() const;

		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// free this object from memory
		void queueFree() { ECHO_DELETE_T(this, Object); }
		void free() { ECHO_DELETE_T(this, Object); }

	public:
		// propertys (script property or dynamic property)
		const PropertyInfos& getPropertys() const;

		// register property
		bool registerProperty(const String& className, const String& propertyName, const Variant::Type type);

	protected:
		static Object* instanceObject(void* pugiNode);

		// remember property recursive
		static void loadPropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);

	protected:
		String			m_name;
		PropertyInfos	m_propertys;
	};
}