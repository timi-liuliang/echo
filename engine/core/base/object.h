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
		virtual ~Object();

		// get by id
		static Object* getById(i32 id);

		// get class name
		virtual const String& getClassName() const;

		// get id
		i32 getId() const { return m_id; }

		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// free this object from memory
		void queueFree() { ECHO_DELETE_T(this, Object); }
		void clear() { ECHO_DELETE_T(this, Object); }

	public:
		// propertys (script property or dynamic property)
		const PropertyInfos& getPropertys() const;

		// clear propertys
		void clearPropertys();

		// register property
		bool registerProperty(const String& className, const String& propertyName, const Variant::Type type);

		// get property value
		virtual bool getPropertyValue(const String& propertyName, Variant& oVar) { return false; }

		// set property value
		virtual bool setPropertyValue(const String& propertyName, const Variant& propertyValue) { return false; }

	protected:
		static Object* instanceObject(void* pugiNode);

		// remember property recursive
		static void loadPropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);

		// remember property recursive
		static void savePropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);

		// load propertys value
		static void loadPropertyValue(void* pugiNode, Echo::Object* classPtr, const Echo::String& className, i32 flag);

	protected:
		i32				m_id;
		String			m_name;
		PropertyInfos	m_propertys;
	};
}