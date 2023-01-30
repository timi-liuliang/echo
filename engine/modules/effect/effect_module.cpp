#include "effect_module.h"
#include "sprite.h"
#include "particle_system.h"
#include "trail.h"
#include "editor/sprite_editor.h"
#include "editor/particle_system_editor.h"
#include "editor/trail_editor.h"

namespace Echo
{
	DECLARE_MODULE(EffectModule, __FILE__)

	EffectModule::EffectModule()
	{
	}

	EffectModule* EffectModule::instance()
	{
		static EffectModule* inst = EchoNew(EffectModule);
		return inst;
	}

	void EffectModule::bindMethods()
	{

	}

	void EffectModule::registerTypes()
	{
		Class::registerType<Sprite>();
        Class::registerType<ParticleSystem>();
		Class::registerType<Trail>();
        
		CLASS_REGISTER_EDITOR(Sprite, SpriteEditor)
        CLASS_REGISTER_EDITOR(ParticleSystem, ParticleSystemEditor)
		CLASS_REGISTER_EDITOR(Trail, TrailEditor)
	}
}
