#include "srp_screen.h"

namespace Echo
{
	SRPScreen::SRPScreen()
	{

	}

	SRPScreen::~SRPScreen()
	{

	}

	// instance
	SRPScreen* SRPScreen::instance()
	{
		static SRPScreen* inst = EchoNew(SRPScreen);
		return inst;
	}

	void SRPScreen::bindMethods()
	{
	}
}