#include "Engine/Core.h"
#include "EffectLayerParticlesCube.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{
	const Vector3 EffectLayerParticlesCube::DEFAULT_CP_AREA = Vector3(1.0f, 1.0f, 1.0f);

	EffectLayerParticlesCube::EffectLayerParticlesCube()
		:EffectLayerParticles()
		, mCubeArea(DEFAULT_CP_AREA)
	{
		mType = ELT_ParticlesCube;
	}

	EffectLayerParticlesCube::~EffectLayerParticlesCube()
	{
	}

	void EffectLayerParticlesCube::emitterParticle(EffectParticle* particle)
	{
		EffectLayerParticles::emitterParticle(particle);

		float x = Math::IntervalRandom(-mCubeArea.x, mCubeArea.x);
		float y = Math::IntervalRandom(-mCubeArea.y, mCubeArea.y);
		float z = Math::IntervalRandom(-mCubeArea.z, mCubeArea.z);
		if(mEmitterInSurface)
		{
			static int t = 0;
			switch(t++%3)
			{
			case 0: if(x>0.0f) x=mCubeArea.x; else x=-mCubeArea.x; break;
			case 1: if(y>0.0f) y=mCubeArea.y; else y=-mCubeArea.y; break;
			case 2: if(z>0.0f) z=mCubeArea.z; else z=-mCubeArea.z; break;
			}
			particle->originalPosition = particle->position = Vector3(x,y,z);
		}
		else
		{
			particle->originalPosition = particle->position = Vector3(x,y,z);
		}

		if(mbPerpendicular)
		{
			if(mEmitterInSurface)
			{
				particle->originalDirection = particle->direction = particle->position.normalizedCopy();
			}
		}
	}


	void EffectLayerParticlesCube::copyAttributesTo(EffectParticle* layer)
	{
		EffectLayerParticles::copyAttributesTo(layer);

		EffectLayerParticlesCube* cubeParticles = static_cast<EffectLayerParticlesCube*>(layer);
		cubeParticles->mCubeArea = mCubeArea;
	}

	void EffectLayerParticlesCube::exportData(DataStream* pStream, int version)
	{
		EffectLayerParticles::exportData(pStream, version);
		pStream->write(&mCubeArea, sizeof(Vector3));
	}

	void EffectLayerParticlesCube::importData(DataStream* pStream, int version)
	{
		EffectLayerParticles::importData(pStream, version);
		pStream->read(&mCubeArea, sizeof(Vector3));
	}

	void EffectLayerParticlesCube::getPropertyList(PropertyList& list)
	{
		EffectLayerParticles::getPropertyList(list);
		list.push_back(token[TOKEN_LY_CP_AREA]);
	}

	bool EffectLayerParticlesCube::getPropertyType(const String& name, PropertyType& type)
	{
		if(EffectLayerParticles::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_LY_CP_AREA])
		{
			type = IElement::PT_VECTOR3; return true;
		}

		return false;
	}

	bool EffectLayerParticlesCube::getPropertyValue(const String& name, String& value)
	{
		if(EffectLayerParticles::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_CP_AREA])
		{
			value = StringUtil::ToString(mCubeArea); return true;
		}

		return false;
	}

	bool EffectLayerParticlesCube::setPropertyValue(const String& name,const String& value)
	{
		if(EffectLayerParticles::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_CP_AREA])
		{
			mCubeArea = StringUtil::ParseVec3(value); 
			return true;
		}

		return false;
	}
}