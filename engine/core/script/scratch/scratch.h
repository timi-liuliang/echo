#pragma once

#include "engine/core/util/StringUtil.h"
#include "engine/core/resource/Res.h"

namespace Echo
{
	class Scratch : public Res
	{
		ECHO_RES(Scratch, Res, ".scratch", Res::create<Scratch>, Scratch::load);

	public:
		Scratch();
		Scratch(const ResourcePath& path);
		~Scratch();

		// parent
		const String& getParentClass() const { return m_parentClass; }
		void setParentClass(const String& parent) { m_parentClass = parent; }

		// compile to lua
		bool compile() { return false; }

		// get lua content
		const String& getLua() { return m_lua; }

		// load|save
		static Res* load(const ResourcePath& path);

	private:
		String m_lua;
		String m_parentClass;
	};
	typedef ResRef<Scratch> ScratchPtr;
}