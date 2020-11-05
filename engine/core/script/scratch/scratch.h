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

		// compile to lua
		bool compile() { return false; }

		// get lua content
		const String& getLua() { return m_lua; }

		// load|save
		static Res* load(const ResourcePath& path);
		void save(const char* pathName);

	private:
		String m_lua;
	};
	typedef ResRef<Scratch> ScratchPtr;
}