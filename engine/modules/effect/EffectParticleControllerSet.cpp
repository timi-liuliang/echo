#include "EffectSystemManager.h"
#include "EffectParticleControllerSet.h"
#include "Engine/core/Geom/SimpleSpline.h"

namespace Echo
{
	EffectParticleControllerSet::EffectParticleControllerSet()
	{

	}

	EffectParticleControllerSet::~EffectParticleControllerSet()
	{
		destroyAllControllers();
	}

	void EffectParticleControllerSet::_notifyStart()
	{
		for(size_t i =0; i<mControllers.size(); ++i)
		{
			mControllers[i]->_notifyStart(NULL);
		}
	}

	void EffectParticleControllerSet::_updateParticle(EffectParticle* particle)
	{
		for(size_t i =0; i<mControllers.size(); ++i)
		{
			mControllers[i]->controll(particle->mTime, particle, true);
		}
	}

	void EffectParticleControllerSet::copyAttributesTo(EffectParticleControllerSet* ParticleControllerSet)
	{
		EffectController* cloneController = 0;
		for(size_t i=0; i<mControllers.size(); ++i)
		{
			cloneController = EffectSystemManager::instance()->cloneController(getController(i));
			ParticleControllerSet->addController(cloneController);
		}

		if(mSpline != NULL)
		{
			ParticleControllerSet->mSpline = EchoNew(SimpleSpline);
			for(size_t i=0; i<mSpline->getNumPoints(); ++i)
			{
				ParticleControllerSet->mSpline->addPoint(mSpline->getPoint(i));
			}
		}
	}

	void EffectParticleControllerSet::getPropertyList(PropertyList& list)
	{

	}

	bool EffectParticleControllerSet::getPropertyType(const String& name, PropertyType& type)
	{
		return false;
	}

	bool EffectParticleControllerSet::getPropertyValue(const String& name, String& value)
	{
		return false;
	}

	bool EffectParticleControllerSet::setPropertyValue(const String& name,const String& value)
	{
		return false;
	}
}