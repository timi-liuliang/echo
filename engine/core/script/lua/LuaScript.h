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

		// load
		static Res* load(const ResourcePath& path);

		// save
		virtual void save() override;

	protected:
		String			m_srcData;
	};
}