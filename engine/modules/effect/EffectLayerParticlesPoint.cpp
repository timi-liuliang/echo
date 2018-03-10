#include "Engine/Core.h"
#include "EffectLayerParticlesPoint.h"
#include "Engine/core/Scene/Node.h"
#include "engine/core/Math/Quaternion.h"

namespace Echo
{

	EffectLayerParticlesPoint::EffectLayerParticlesPoint()
		:EffectLayerParticles()
	{
		mType = ELT_ParticlesPoint;
	}

	EffectLayerParticlesPoint::~EffectLayerParticlesPoint()
	{
	}

	void EffectLayerParticlesPoint::emitterParticle(EffectParticle* particle)
	{
		EffectLayerParticles::emitterParticle(particle);
		particle->originalPosition = particle->position = Vector3::ZERO;

		if(mbPerpendicular)
		{
			if (!mFallowParent)
			{
				Quaternion parentOritation = mParentSystem->getEffectSceneNode()->getWorldOrientation();
				particle->originalDirection = particle->direction = parentOritation * direction;
			}
		}
	}
}