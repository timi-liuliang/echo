#ifndef __EFFECT_LAYER_CUBEPARTICLE_H__
#define __EFFECT_LAYER_CUBEPARTICLE_H__

#include "EffectLayerParticles.h"

namespace Echo
{
	class EffectLayerParticlesCube : public EffectLayerParticles
	{
	public:
		static const Vector3 DEFAULT_CP_AREA;

		virtual void getPropertyList(PropertyList& list);
		virtual bool getPropertyType(const String& name, PropertyType& type);
		virtual bool getPropertyValue(const String& name, String& value);
		virtual bool setPropertyValue(const String& name,const String& value);

	public:
		EffectLayerParticlesCube();
		virtual ~EffectLayerParticlesCube();

		virtual void emitterParticle(EffectParticle* particle);

		virtual void copyAttributesTo(EffectParticle* layer);
		virtual void exportData(DataStream* pStream, int version);
		virtual void importData(DataStream* pStream, int version);
	protected:
		Vector3 mCubeArea;
	};
}

#endif