#include "engine/modules/camera/camera_module.h"
#include "engine/modules/box2d/box2d_module.h"
#include "engine/modules/audio/audio_module.h"
#include "engine/modules/effect/effect_module.h"
#include "engine/modules/live2d/live2d_module.h"
#include "engine/modules/spine/spine_module.h"
#include "engine/modules/ui/ui_module.h"
#include "engine/modules/gltf/gltf_module.h"
#include "engine/modules/anim/anim_module.h"
#include "engine/modules/ai/ai_module.h"
#include "engine/modules/light/light_module.h"
#include "engine/modules/physx/physx_module.h"
#include "engine/modules/terrain/terrain_module.h"
#include "engine/modules/sray/sray_module.h"
#include "engine/modules/geom/geom_module.h"
#include "engine/modules/video/video_module.h"
#include "build_module.h"

namespace Echo
{
	// implement by application or dll
	void registerModules()
	{
        REGISTER_MODULE(GeomModule)
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
        REGISTER_MODULE(TerrainModule);
        REGISTER_MODULE(SRayModule);
		REGISTER_MODULE(VideoModule);
        REGISTER_MODULE(BuildModule);
	}
}
