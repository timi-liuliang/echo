#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class Object
	{
	public:
		virtual ~Object(){}

		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

	private:
		String			m_name;
	};
}