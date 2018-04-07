#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/util/KeyValues.h"

namespace Echo
{
	class Object
	{
	public:
		virtual ~Object(){}

		// get class name
		virtual const String& getClassName() const;

		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		virtual void setProperty(const String& key, const String& value){}
		virtual bool getProperty(const String& key, String& value) { return false; }

	protected:
		String			m_name;
		KeyValues		m_propertys;
	};
}