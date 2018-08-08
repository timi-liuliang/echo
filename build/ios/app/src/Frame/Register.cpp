#include "VenusPentagramExample.h"
#include "EffectOcean.h"
#include "WeatherLightning.h"
#include "DestructionDemo.h"
#include "FXAA.h"
#include "SpaceDistortion.h"
#include "TextureSupport.h"
#include "VRModeRoaming.h"
#include "MultiThread.h"
#include "LuaModule.h"
#include "MultiThreadRendering.h"
#include "MultiThreadSkeleton.h"

namespace Examples
{
	// ×¢²áËùÓÐÊ¾Àý
	void registerAllExamples()
	{
		EXAMPLE_REGISTER(EffectOcean, "EffectOcean", "set:examples image:effectocean");
		EXAMPLE_REGISTER(VenusPentagram, "VenusPentagram", "set:examples image:venuspentagram");
		EXAMPLE_REGISTER(DestructionDemo, "Destruction", "set:examples image:destruction");
		//EXAMPLE_REGISTER(FXAA, "FXAA", "set:examples image:FXAA");
		EXAMPLE_REGISTER(MultiThreadRendering, "MultiThreadRendering", "");
		// EXAMPLE_REGISTER(SpaceDistortion, "SpaceDistortion", "set:examples image:spacedistortion");
		EXAMPLE_REGISTER(MultiThreadSkeleton, "MultiThreadSkeleton", "");
		EXAMPLE_REGISTER(TextureSupport, "TextureSupport", "set:examples image:texturesupport");
		EXAMPLE_REGISTER(VRModeRoaming, "VRModeRoaming", "set:examples image:vrmoderoaming");
		EXAMPLE_REGISTER(WeatherLightning, "WeatherLightning", "set:examples image:weatherlightning");

		EXAMPLE_REGISTER(MultiThread, "MultiThread", "set:examples image:multithread");
		EXAMPLE_REGISTER(LuaModule, "LuaModule", "set:examples image:luamodule");
	}
}
	