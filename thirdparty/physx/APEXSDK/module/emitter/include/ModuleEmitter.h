/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_EMITTER_H__
#define __MODULE_EMITTER_H__

#include "NxApex.h"
#include "NxModuleEmitter.h"
#include "NiApexSDK.h"
#include "Module.h"
#include "NiModule.h"
#include "ApexEmitterAsset.h"
#include "GroundEmitterAsset.h"
#include "ModulePerfScope.h"
#include "ImpactObjectEvent.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "ImpactExplosionEvent.h"
#endif

#include "EmitterParamClasses.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace physx
{
namespace apex
{
namespace emitter
{

class EmitterScene;

#define EMITTER_SCALABLE_PARAM_RATE 0
#define EMITTER_SCALABLE_PARAM_DENSITY 1
#define EMITTER_SCALABLE_PARAM_GROUND_DENSITY 2

class NxModuleEmitterDesc : public NxApexDesc
{
public:

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE NxModuleEmitterDesc() : NxApexDesc()
	{
		init();
	}

	/**
	\brief sets members to default values.
	*/
	PX_INLINE void setToDefault()
	{
		NxApexDesc::setToDefault();
		init();
	}

	/**
	\brief checks if this is a valid descriptor.
	*/
	PX_INLINE bool isValid() const
	{
		bool retVal = NxApexDesc::isValid();
		return retVal;
	}


private:

	PX_INLINE void init()
	{
	}
};

class ModuleEmitter : public NxModuleEmitter, public NiModule, public Module, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleEmitter(NiApexSDK* inSdk);
	~ModuleEmitter();

	void						init(const NxModuleEmitterDesc& moduleEmitterDesc);

	// base class methods
	void						init(NxParameterized::Interface&) {}
	NxParameterized::Interface* getDefaultModuleDesc();
	void						release()
	{
		Module::release();
	}
	void						destroy();
	const char*					getName() const
	{
		NX_READ_ZONE();
		return Module::getName();
	}
	physx::PxU32				getNbParameters() const
	{
		NX_READ_ZONE();
		return Module::getNbParameters();
	}
	NxApexParameter**			getParameters()
	{
		NX_READ_ZONE();
		return Module::getParameters();
	}
	void setLODUnitCost(physx::PxF32 cost)
	{
		NX_WRITE_ZONE();
		Module::setLODUnitCost(cost);
	}
	physx::PxF32 getLODUnitCost() const
	{
		NX_READ_ZONE();
		return Module::getLODUnitCost();
	}
	void setLODBenefitValue(physx::PxF32 value)
	{
		NX_WRITE_ZONE();
		Module::setLODBenefitValue(value);
	}
	physx::PxF32 getLODBenefitValue() const
	{
		NX_READ_ZONE();
		return Module::getLODBenefitValue();
	}
	void setLODEnabled(bool enabled)
	{
		NX_WRITE_ZONE();
		Module::setLODEnabled(enabled);
	}
	bool getLODEnabled() const
	{
		NX_READ_ZONE();
		return Module::getLODEnabled();
	}
	void setIntValue(physx::PxU32 parameterIndex, physx::PxU32 value)
	{
		NX_WRITE_ZONE();
		return Module::setIntValue(parameterIndex, value);
	}

	NxApexRenderableIterator* 	createRenderableIterator(const NxApexScene&);
	NiModuleScene* 				createNiModuleScene(NiApexScene&, NiApexRenderDebug*);
	void						releaseNiModuleScene(NiModuleScene&);
	physx::PxU32				forceLoadAssets();

	NxAuthObjTypeID				getModuleID() const;
	NxAuthObjTypeID             getEmitterAssetTypeID() const;

	ApexActor* 					getApexActor(NxApexActor*, NxAuthObjTypeID) const;

private:
	EmitterScene* 				getEmitterScene(const NxApexScene& apexScene);
	NxResourceList				mAuthorableObjects;
	NxResourceList				mEmitterScenes;

private:
#	define PARAM_CLASS(clas) PARAM_CLASS_DECLARE_FACTORY(clas)
#	include "EmitterParamClasses.inc"

	EmitterModuleParameters* 			mModuleParams;

	friend class ApexEmitterAsset;
	friend class GroundEmitterAsset;
	friend class GroundEmitterActor;
	friend class ImpactEmitterAsset;
	friend class ImpactEmitterActor;
};

}
}
} // namespace physx::apex

#endif // __MODULE_EMITTER_H__
