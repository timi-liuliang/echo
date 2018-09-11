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

using namespace Echo;

REGISTER_MODULE(CameraModule)
REGISTER_MODULE(Box2DModule)
REGISTER_MODULE(AudioModule)
REGISTER_MODULE(Live2DModule)
REGISTER_MODULE(SpineModule)
REGISTER_MODULE(GltfModule)
REGISTER_MODULE(AnimModule)
REGISTER_MODULE(EffectModule)
REGISTER_MODULE(UIModule)
REGISTER_MODULE(AIModule)