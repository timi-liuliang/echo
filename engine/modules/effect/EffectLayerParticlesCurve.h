#ifndef __EFFECT_LAYER_CURVEPARTICLE_H__
#define __EFFECT_LAYER_CURVEPARTICLE_H__

#include "EffectLayerParticles.h"

namespace Echo
{
	class EffectLayerParticlesCurve : public EffectLayerParticles
	{
	public:
		EffectLayerParticlesCurve();
		virtual ~EffectLayerParticlesCurve();

		virtual void emitterParticle(EffectParticle* particle);
	};
}

#endif