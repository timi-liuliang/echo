#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class SRPScreen : public Object
	{
		ECHO_SINGLETON_CLASS(SRPScreen, Object)

	public:
		SRPScreen();
		virtual ~SRPScreen();

		// instance
		static SRPScreen* instance();

	protected:
	};
}