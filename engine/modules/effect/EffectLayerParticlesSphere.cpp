#include "Engine/Core.h"
#include "EffectLayerParticlesSphere.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{
	const Vector3 EffectLayerParticlesSphere::DEFAULT_SP_AREA = Vector3(1.0f, 1.0f, 1.0f);

	EffectLayerParticlesSphere::EffectLayerParticlesSphere()
		:EffectLayerParticles()
		, mSphereArea(DEFAULT_SP_AREA)
	{
		mType = ELT_ParticlesSphere;
	}

	EffectLayerParticlesSphere::~EffectLayerParticlesSphere()
	{

	}

	void EffectLayerParticlesSphere::emitterParticle(EffectParticle* particle)
	{
		EffectLayerParticles::emitterParticle(particle);

		float angleLongitude = Math::IntervalRandom(0.0f, Math::PI_2);
		float angleLatitude = Math::IntervalRandom(-Math::PI_DIV2, Math::PI_DIV2);
		float radius = 1.0f;
		if(!mEmitterInSurface)
			radius = Math::IntervalRandom(0.0f, 1.0f);

		float x, y, z, projectxz;
		y = radius * Math::Sin(angleLatitude);
		projectxz = radius * Math::Cos(angleLatitude);
		x = projectxz * Math::Cos(angleLongitude);
		z = projectxz * Math::Sin(angleLongitude);

		particle->originalPosition = particle->position = Vector3(mSphereArea.x*x, mSphereArea.y*y, mSphereArea.z*z);
		//Vec3 dir = particle->position.normalisedCopy();
		if(mbPerpendicular)
		{
			if(mEmitterInSurface) 
			{
				particle->originalDirection = particle->direction = particle->position.normalizedCopy();
			}
		}
	}

	void EffectLayerParticlesSphere::copyAttributesTo(EffectParticle* layer)
	{
		EffectLayerParticles::copyAttributesTo(layer);

		EffectLayerParticlesSphere* cubeParticles = static_cast<EffectLayerParticlesSphere*>(layer);
		cubeParticles->mSphereArea = mSphereArea;
	}

	void EffectLayerParticlesSphere::exportData(DataStream* pStream, int version)
	{
		EffectLayerParticles::exportData(pStream, version);
		pStream->write(&mSphereArea, sizeof(Vector3));
	}

	void EffectLayerParticlesSphere::importData(DataStream* pStream, int version)
	{
		EffectLayerParticles::importData(pStream, version);
		pStream->read(&mSphereArea, sizeof(Vector3));
	}

	void EffectLayerParticlesSphere::getPropertyList(PropertyList& list)
	{
		EffectLayerParticles::getPropertyList(list);
		list.push_back(token[TOKEN_LY_SP_AREA]);
	}

	bool EffectLayerParticlesSphere::getPropertyType(const String& name, PropertyType& type)
	{
		if(EffectLayerParticles::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_LY_SP_AREA])
		{
			type = IElement::PT_VECTOR3; return true;
		}

		return false;
	}

	bool EffectLayerParticlesSphere::getPropertyValue(const String& name, String& value)
	{
		if(EffectLayerParticles::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_SP_AREA])
		{
			value = StringUtil::ToString(mSphereArea); return true;
		}

		return false;
	}

	bool EffectLayerParticlesSphere::setPropertyValue(const String& name,const String& value)
	{
		if(EffectLayerParticles::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_SP_AREA])
		{
			mSphereArea = StringUtil::ParseVec3(value); 
			return true;
		}

		return false;
	}

}