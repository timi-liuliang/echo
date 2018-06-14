#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
	class LuaScript : public Res
	{
		ECHO_RES(LuaScript, Res, ".lua", Res::create<LuaScript>, Res::load);

	public:
	};
}