#include <engine/core/main/module.h>

namespace Echo
{
	// implement by application or dll
	void registerModules()
	{
		REGISTER_MODULE(CameraModule)
		REGISTER_MODULE(Box2DModule)
		REGISTER_MODULE(AudioModule)
		REGISTER_MODULE(Live2DModule)
		REGISTER_MODULE(SpineModule)
		REGISTER_MODULE(GltfModule)
		REGISTER_MODULE(AnimModule)
		REGISTER_MODULE(EffectModule)
		REGISTER_MODULE(UiModule)
		REGISTER_MODULE(AIModule)
		REGISTER_MODULE(LightModule);
		REGISTER_MODULE(PhysxModule);
	}
}
