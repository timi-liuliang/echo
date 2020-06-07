#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
	class Translator : public Res
	{
		ECHO_RES(Translator, Res, ".translator", Res::create<Translator>, Translator::load);

	public:
		Translator();

		// load | save
		static Res* load(const ResourcePath& path);
		virtual void save() override;
	};
}