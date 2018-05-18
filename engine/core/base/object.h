#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/util/KeyValues.h"

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

	protected:
		String			m_name;
	};
}