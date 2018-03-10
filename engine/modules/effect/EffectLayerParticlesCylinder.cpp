#include "Engine/Core.h"
#include "EffectLayerParticlesCylinder.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{
	const Vector3 EffectLayerParticlesCylinder::DEFAULT_LP_AREA(1.0f, 1.0f, 1.0f);

	EffectLayerParticlesCylinder::EffectLayerParticlesCylinder()
		:EffectLayerParticles()
		, mCylinderArea(DEFAULT_LP_AREA)
	{
		mType = ELT_ParticlesColumn;
	}

	EffectLayerParticlesCylinder::~EffectLayerParticlesCylinder()
	{
	}

	void EffectLayerParticlesCylinder::emitterParticle(EffectParticle* particle)
	{
		EffectLayerParticles::emitterParticle(particle);

		float angle = Math::IntervalRandom(0, Math::PI_2);
		float height = Math::IntervalRandom(-mCylinderArea.y, mCylinderArea.y);
		float radius = 1.0f;
		if(!mEmitterInSurface)
			radius = Math::IntervalRandom(0.0f, 1.0f);

		float x, z;
		x = radius*Math::Cos(angle);
		z = radius*Math::Sin(angle);

		particle->originalPosition = particle->position = Vector3(x*mCylinderArea.x, height, z*mCylinderArea.z);

		if(mbPerpendicular)
		{
			if(mEmitterInSurface)
			{
				Vector3 dir = particle->position.normalizedCopy();
				particle->originalDirection = particle->direction = dir.cross(Vector3::UNIT_Y);
			}
		}
	}


	void EffectLayerParticlesCylinder::copyAttributesTo(EffectParticle* layer)
	{
		EffectLayerParticles::copyAttributesTo(layer);

		EffectLayerParticlesCylinder* cylinderParticles = static_cast<EffectLayerParticlesCylinder*>(layer);
		cylinderParticles->mCylinderArea = mCylinderArea;
	}

	void EffectLayerParticlesCylinder::exportData(DataStream* pStream, int version)
	{
		EffectLayerParticles::exportData(pStream, version);
		pStream->write(&mCylinderArea, sizeof(Vector3));
	}

	void EffectLayerParticlesCylinder::importData(DataStream* pStream, int version)
	{
		EffectLayerParticles::importData(pStream, version);
		pStream->read(&mCylinderArea, sizeof(Vector3));
	}

	void EffectLayerParticlesCylinder::getPropertyList(PropertyList& list)
	{
		EffectLayerParticles::getPropertyList(list);
		list.push_back(token[TOKEN_LY_LP_AREA]);
	}

	bool EffectLayerParticlesCylinder::getPropertyType(const String& name, PropertyType& type)
	{
		if(EffectLayerParticles::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_LY_LP_AREA])
		{
			type = IElement::PT_VECTOR3; return true;
		}

		return false;
	}

	bool EffectLayerParticlesCylinder::getPropertyValue(const String& name, String& value)
	{
		if(EffectLayerParticles::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_LP_AREA])
		{
			value = StringUtil::ToString(mCylinderArea); return true;
		}

		return false;
	}

	bool EffectLayerParticlesCylinder::setPropertyValue(const String& name,const String& value)
	{
		if(EffectLayerParticles::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_LP_AREA])
		{
			mCylinderArea = StringUtil::ParseVec3(value); 
			return true;
		}

		return false;
	}

}