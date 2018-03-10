#include "Engine/Core.h"
#include "EffectLayerParticlesCurve.h"
#include "Engine/core/Geom/SimpleSpline.h"

namespace Echo
{
	EffectLayerParticlesCurve::EffectLayerParticlesCurve()
		:EffectLayerParticles()
	{
		mType = ELT_ParticlesCruve;
	}

	EffectLayerParticlesCurve::~EffectLayerParticlesCurve()
	{
	}

	void EffectLayerParticlesCurve::emitterParticle(EffectParticle* particle)
	{
		EffectLayerParticles::emitterParticle(particle);

		SimpleSpline* spline = mParticleControllerSet->getSplinePtr();

		if(spline == NULL)
			particle->originalPosition = particle->position = Vector3::ZERO;
		else
		{
			float pos = Math::IntervalRandom(0.0f, 1.0f);
			Vector3 _position = spline->interpolate(pos);

			particle->originalPosition = particle->position = _position;
		}
		if(mbPerpendicular)
		{
			particle->originalDirection = particle->direction = particle->movementDirection;
		}
	}
}