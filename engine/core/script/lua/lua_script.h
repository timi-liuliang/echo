#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
	class LuaScript : public Res
	{
		ECHO_RES(LuaScript, Res, ".lua", Res::create<LuaScript>, LuaScript::load);

	public:
		LuaScript();
		LuaScript(const ResourcePath& path);
		virtual ~LuaScript();

		// get src
		void setSrc(const char* src) { m_srcData = src; }
		const char* getSrc() const { return m_srcData.c_str(); }

		// load
		static Res* load(const ResourcePath& path);

		// save
		virtual void save() override;

	protected:
		String			m_srcData;
	};
	typedef ResRef<LuaScript> LuaResPtr;
}