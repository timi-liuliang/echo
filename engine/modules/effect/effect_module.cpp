#include "effect_module.h"
#include "sprite.h"
#include "particle_system.h"
#include "particle_system_render.h"
#include "editor/particle_system_editor.h"

namespace Echo
{
	DECLARE_MODULE(EffectModule)

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
        Class::registerType<ParticleSystemRender>();
        
        REGISTER_OBJECT_EDITOR(ParticleSystem, ParticleSystemEditor)
	}
}
