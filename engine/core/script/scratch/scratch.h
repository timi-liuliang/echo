#pragma once

#include "engine/core/util/StringUtil.h"

namespace Echo
{
	class Scratch
	{
	public:
		Scratch();
		~Scratch();

		// compile to lua
		bool compile() { return false; }

		// get lua content
		const String& getLua() { return m_lua; }

		// load|save
		void load(const char* pathName);
		void save(const char* pathName);

	private:
		String m_lua;
	};
}