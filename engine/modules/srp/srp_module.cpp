#include "srp_module.h"
#include "srp_image.h"
#include "srp_screen.h"
#include "srp_offscreen.h"

namespace Echo
{
	SRPModule::SRPModule()
	{
	}

	void SRPModule::registerTypes()
	{
		Class::registerType<SRPImage>();
		Class::registerType<SRPOffscreen>();
		Class::registerType<SRPScreen>();
	}
}