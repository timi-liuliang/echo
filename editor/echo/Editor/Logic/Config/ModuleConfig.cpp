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
		REGISTER_MODULE(SplineModule)
		REGISTER_MODULE(GltfModule)
		REGISTER_MODULE(AnimModule)
		REGISTER_MODULE(EffectModule)
		REGISTER_MODULE(UiModule)
		REGISTER_MODULE(AIModule)
		REGISTER_MODULE(LightModule);
		REGISTER_MODULE(ModelModule);
		REGISTER_MODULE(PhysxModule);
        REGISTER_MODULE(PCGModule);
		REGISTER_MODULE(RaytracingModule);
        REGISTER_MODULE(SceneModule);
#ifdef ECHO_PLATFORM_WINDOWS
		REGISTER_MODULE(VideoModule);
#endif
        REGISTER_MODULE(BuildModule);
		REGISTER_MODULE(EditorSettingsModule);
		REGISTER_MODULE(RecastModule)
		REGISTER_MODULE(HitProxyModule)
		REGISTER_MODULE(HLodModule)

		REGISTER_MODULE(Application)
	}
}
