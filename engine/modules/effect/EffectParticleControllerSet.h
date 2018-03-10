#ifndef __EFFECT_PARTICLECONTROLLERSET_H__
#define __EFFECT_PARTICLECONTROLLERSET_H__

#include "EffectHeader.h"
#include "EffectController.h"
#include "EffectParticle.h"
#include "EffectKeyFrame.h"

namespace Echo
{
	class EffectLayerParticles;

	class EffectParticleControllerSet : public EffectKeyFrame
	{
		friend class ParticleControllerSetTranslator;
		friend class EffectParticleControllerSetWriter;
	public:
		//interface
		virtual void getPropertyList(PropertyList& list);
		virtual bool getPropertyType(const String& name, PropertyType& type);
		virtual bool getPropertyValue(const String& name, String& value);
		virtual bool setPropertyValue(const String& name,const String& value);

	public:
		typedef vector<EffectController*>::type EffectControllerList;
		typedef EffectControllerList::iterator EffectControllerIterator;

	public:
		EffectParticleControllerSet();
		~EffectParticleControllerSet();

		virtual void _notifyStart();

		virtual void _updateParticle(EffectParticle* particle);

		String getName() const { return "ParticleControllerSet"; }

		void copyAttributesTo(EffectParticleControllerSet* ParticleControllerSet);

	protected:
	};
}

#endif