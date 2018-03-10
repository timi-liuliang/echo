#ifndef __EFFECT_LAYER_POINTPARTICLE_H__
#define __EFFECT_LAYER_POINTPARTICLE_H__

#include "EffectLayerParticles.h"

namespace Echo
{
	class EffectLayerParticlesPoint : public EffectLayerParticles
	{
	public:
		EffectLayerParticlesPoint();
		virtual ~EffectLayerParticlesPoint();

		virtual void emitterParticle(EffectParticle* particle);
	};
}

#endif