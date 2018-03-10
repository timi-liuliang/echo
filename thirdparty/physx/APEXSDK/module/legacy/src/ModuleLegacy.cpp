/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PsShare.h"
#include "NxApex.h"
#include "ApexLegacyModule.h"
#include "ApexRWLockable.h"

#define SAFE_MODULE_RELEASE(x) if ( x ) { NiModule *m = mSdk->getNiModule(x); PX_ASSERT(m); m->setParent(NULL); m->setCreateOk(false); x->release(); x = NULL; }

namespace physx
{
namespace apex
{

#ifdef _USRDLL
NiApexSDK* gApexSdk = 0;
NxApexSDK* NxGetApexSDK()
{
	return gApexSdk;
}
NiApexSDK* NiGetApexSDK()
{
	return gApexSdk;
}
#endif

namespace legacy
{

#define MODULE_CHECK(x) if ( x == module ) { x = NULL; }
#define SAFE_MODULE_NULL(x) if ( x ) { NiModule *m = mSdk->getNiModule(x); PX_ASSERT(m); m->setParent(NULL); x = NULL;}

class ModuleLegacy : public ApexLegacyModule, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleLegacy(NiApexSDK* sdk);

	/**
	Notification from ApexSDK when a module has been released
	*/
	virtual void notifyChildGone(NiModule* imodule)
	{
		NxModule* module = mSdk->getNxModule(imodule);
		PX_ASSERT(module);
		PX_UNUSED(module);
		// notifyChildGoneMarker
#if APEX_USE_PARTICLES
#if NX_SDK_VERSION_MAJOR == 3
		MODULE_CHECK(mModuleParticlesLegacy);
		MODULE_CHECK(mModuleParticleIOSLegacy);
		MODULE_CHECK(mModuleForceFieldLegacy);
#endif
		MODULE_CHECK(mModuleBasicFSLegacy);
		MODULE_CHECK(mModuleTurbulenceFSLegacy);
#if NX_SDK_VERSION_MAJOR == 2
		MODULE_CHECK(mModuleFieldBoundaryLegacy);
		MODULE_CHECK(mModuleExplosionLegacy);
#endif
		MODULE_CHECK(mModuleBasicIOSLegacy);
		MODULE_CHECK(mModuleEmitterLegacy);
		MODULE_CHECK(mModuleIOFXLegacy);
#endif
		MODULE_CHECK(mModuleClothingLegacy);
		MODULE_CHECK(mModuleCommonLegacy);
		MODULE_CHECK(mModuleDestructibleLegacy);
		MODULE_CHECK(mModuleFrameworkLegacy);
	};

	// This is a notification that the ApexSDK is being released.  During the shutdown process
	// the APEX SDK will automatically release all currently registered modules; therefore we are no longer
	// responsible for releasing these modules ourselves.
	virtual void notifyReleaseSDK()
	{
#if APEX_USE_PARTICLES
		// notifyReleaseSDKMarker
		SAFE_MODULE_NULL(mModuleBasicIOSLegacy);
		SAFE_MODULE_NULL(mModuleEmitterLegacy);
		SAFE_MODULE_NULL(mModuleIOFXLegacy);
#if NX_SDK_VERSION_MAJOR == 3
		SAFE_MODULE_NULL(mModuleParticlesLegacy);
		SAFE_MODULE_NULL(mModuleParticleIOSLegacy);
		SAFE_MODULE_NULL(mModuleForceFieldLegacy);
#endif
		SAFE_MODULE_NULL(mModuleBasicFSLegacy);
		SAFE_MODULE_NULL(mModuleTurbulenceFSLegacy);
#if NX_SDK_VERSION_MAJOR == 2
		SAFE_MODULE_NULL(mModuleFieldBoundaryLegacy);
		SAFE_MODULE_NULL(mModuleExplosionLegacy);
#endif
#endif
		SAFE_MODULE_NULL(mModuleClothingLegacy);
		SAFE_MODULE_NULL(mModuleCommonLegacy);
		SAFE_MODULE_NULL(mModuleDestructibleLegacy);
		SAFE_MODULE_NULL(mModuleFrameworkLegacy);
	}

protected:
	void releaseLegacyObjects();

private:
	// Add custom conversions here
	// NxModulePointerDefinitionMarker
#if APEX_USE_PARTICLES
	physx::apex::NxModule*	mModuleBasicIOSLegacy;
	physx::apex::NxModule*	mModuleEmitterLegacy;
	physx::apex::NxModule*	mModuleIOFXLegacy;
#if NX_SDK_VERSION_MAJOR == 3
	physx::apex::NxModule*	mModuleParticlesLegacy;
	physx::apex::NxModule*	mModuleParticleIOSLegacy;
	physx::apex::NxModule*	mModuleForceFieldLegacy;
#endif
	physx::apex::NxModule*	mModuleBasicFSLegacy;
	physx::apex::NxModule*	mModuleTurbulenceFSLegacy;
#if NX_SDK_VERSION_MAJOR == 2
	physx::apex::NxModule*	mModuleFieldBoundaryLegacy;
	physx::apex::NxModule*	mModuleExplosionLegacy;
#endif
#endif	
	physx::apex::NxModule*	mModuleClothingLegacy;
	physx::apex::NxModule*	mModuleCommonLegacy;
	physx::apex::NxModule*	mModuleDestructibleLegacy;
	physx::apex::NxModule*	mModuleFrameworkLegacy;
};

}
#ifdef _USRDLL

NXAPEX_API NxModule*  NX_CALL_CONV createModule(
    NiApexSDK* inSdk,
    NiModule** niRef,
    physx::PxU32 APEXsdkVersion,
    physx::PxU32 PhysXsdkVersion,
    NxApexCreateError* errorCode)
{
	if (APEXsdkVersion != NX_APEX_SDK_VERSION)
	{
		if (errorCode)
		{
			*errorCode = APEX_CE_WRONG_VERSION;
		}
		return NULL;
	}

	if (PhysXsdkVersion != NX_PHYSICS_SDK_VERSION)
	{
		if (errorCode)
		{
			*errorCode = APEX_CE_WRONG_VERSION;
		}
		return NULL;
	}

	gApexSdk = inSdk;
	APEX_INIT_FOUNDATION();

	legacy::ModuleLegacy* impl = PX_NEW(legacy::ModuleLegacy)(inSdk);
	*niRef  = (NiModule*) impl;
	return (NxModule*) impl;
}
#else
void instantiateModuleLegacy()
{
	NiApexSDK* sdk = NiGetApexSDK();
	legacy::ModuleLegacy* impl = PX_NEW(legacy::ModuleLegacy)(sdk);
	sdk->registerExternalModule((NxModule*) impl, (NiModule*) impl);
}
#endif

namespace legacy
{
#if APEX_USE_PARTICLES
// instantiateDefinitionMarker
void instantiateModuleBasicIOSLegacy();
void instantiateModuleEmitterLegacy();
void instantiateModuleIOFXLegacy();
#if NX_SDK_VERSION_MAJOR == 3
void instantiateModuleParticlesLegacy();
void instantiateModuleParticleIOSLegacy();
void instantiateModuleForceFieldLegacy();
#endif
#if NX_SDK_VERSION_MAJOR == 2
void instantiateModuleFieldBoundaryLegacy();
void instantiateModuleExplosionLegacy();
#endif
void instantiateModuleBasicFSLegacy();
void instantiateModuleTurbulenceFSLegacy();
#endif

void instantiateModuleClothingLegacy();
void instantiateModuleCommonLegacy();
void instantiateModuleDestructibleLegacy();
void instantiateModuleFrameworkLegacy();


#define MODULE_PARENT(x) if ( x ) { NiModule *m = mSdk->getNiModule(x); PX_ASSERT(m); m->setParent(this); }

ModuleLegacy::ModuleLegacy(NiApexSDK* inSdk)
{
	name = "Legacy";
	mSdk = inSdk;
	mApiProxy = this;

	// Register legacy stuff

	NxParameterized::Traits* t = mSdk->getParameterizedTraits();
	if (!t)
	{
		return;
	}

#if APEX_USE_PARTICLES
	// instantiateCallMarker	
	
	instantiateModuleBasicIOSLegacy();
	mModuleBasicIOSLegacy = mSdk->createModule("BasicIOS_Legacy", NULL);
	PX_ASSERT(mModuleBasicIOSLegacy);
	MODULE_PARENT(mModuleBasicIOSLegacy);

	instantiateModuleEmitterLegacy();
	mModuleEmitterLegacy = mSdk->createModule("Emitter_Legacy", NULL);
	PX_ASSERT(mModuleEmitterLegacy);
	MODULE_PARENT(mModuleEmitterLegacy);

	instantiateModuleIOFXLegacy();
	mModuleIOFXLegacy = mSdk->createModule("IOFX_Legacy", NULL);
	PX_ASSERT(mModuleIOFXLegacy);
	MODULE_PARENT(mModuleIOFXLegacy);

#if NX_SDK_VERSION_MAJOR == 3
		
	instantiateModuleParticlesLegacy();
	mModuleParticlesLegacy = mSdk->createModule("Particles_Legacy", NULL);
	PX_ASSERT(mModuleParticlesLegacy);
	MODULE_PARENT(mModuleParticlesLegacy);	
	
	instantiateModuleParticleIOSLegacy();
	mModuleParticleIOSLegacy = mSdk->createModule("ParticleIOS_Legacy", NULL);
	PX_ASSERT(mModuleParticleIOSLegacy);
	MODULE_PARENT(mModuleParticleIOSLegacy);

	instantiateModuleForceFieldLegacy();
	mModuleForceFieldLegacy = mSdk->createModule("ForceField_Legacy", NULL);
	PX_ASSERT(mModuleForceFieldLegacy);
	MODULE_PARENT(mModuleForceFieldLegacy);	
#endif

#if NX_SDK_VERSION_MAJOR == 2
	instantiateModuleFieldBoundaryLegacy();
	mModuleFieldBoundaryLegacy = mSdk->createModule("FieldBoundary_Legacy", NULL);
	PX_ASSERT(mModuleFieldBoundaryLegacy);
	MODULE_PARENT(mModuleFieldBoundaryLegacy);

	instantiateModuleExplosionLegacy();
	mModuleExplosionLegacy = mSdk->createModule("Explosion_Legacy", NULL);
	PX_ASSERT(mModuleExplosionLegacy);
	MODULE_PARENT(mModuleExplosionLegacy);

#endif
	
	instantiateModuleBasicFSLegacy();
	mModuleBasicFSLegacy = mSdk->createModule("BasicFS_Legacy", NULL);
	PX_ASSERT(mModuleBasicFSLegacy);
	MODULE_PARENT(mModuleBasicFSLegacy);	

	instantiateModuleTurbulenceFSLegacy();
	mModuleTurbulenceFSLegacy = mSdk->createModule("TurbulenceFS_Legacy", NULL);
	PX_ASSERT(mModuleTurbulenceFSLegacy);
	MODULE_PARENT(mModuleTurbulenceFSLegacy);
#endif

	instantiateModuleCommonLegacy();
	mModuleCommonLegacy = mSdk->createModule("Common_Legacy", NULL);
	PX_ASSERT(mModuleCommonLegacy);
	MODULE_PARENT(mModuleCommonLegacy);

	instantiateModuleDestructibleLegacy();
	mModuleDestructibleLegacy = mSdk->createModule("Destructible_Legacy", NULL);
	PX_ASSERT(mModuleDestructibleLegacy);
	MODULE_PARENT(mModuleDestructibleLegacy);

	instantiateModuleFrameworkLegacy();
	mModuleFrameworkLegacy = mSdk->createModule("Framework_Legacy", NULL);
	PX_ASSERT(mModuleFrameworkLegacy);
	MODULE_PARENT(mModuleFrameworkLegacy);

	instantiateModuleClothingLegacy();
	mModuleClothingLegacy = mSdk->createModule("Clothing_Legacy", NULL);
	PX_ASSERT(mModuleClothingLegacy);
	MODULE_PARENT(mModuleClothingLegacy);
}

void ModuleLegacy::releaseLegacyObjects()
{
	//Release legacy stuff

	NxParameterized::Traits* t = mSdk->getParameterizedTraits();
	if (!t)
	{
		return;
	}

#if APEX_USE_PARTICLES
		// releaseLegacyObjectsMarker
	SAFE_MODULE_RELEASE(mModuleBasicIOSLegacy);
	SAFE_MODULE_RELEASE(mModuleEmitterLegacy);
	SAFE_MODULE_RELEASE(mModuleIOFXLegacy);
#if NX_SDK_VERSION_MAJOR == 3
	SAFE_MODULE_RELEASE(mModuleParticlesLegacy);
	SAFE_MODULE_RELEASE(mModuleParticleIOSLegacy);
	SAFE_MODULE_RELEASE(mModuleForceFieldLegacy);
#endif
#if NX_SDK_VERSION_MAJOR == 2
	SAFE_MODULE_RELEASE(mModuleFieldBoundaryLegacy);
	SAFE_MODULE_RELEASE(mModuleExplosionLegacy);
#endif
	SAFE_MODULE_RELEASE(mModuleBasicFSLegacy);
	SAFE_MODULE_RELEASE(mModuleTurbulenceFSLegacy);
#endif

	SAFE_MODULE_RELEASE(mModuleClothingLegacy);
	SAFE_MODULE_RELEASE(mModuleCommonLegacy);
	SAFE_MODULE_RELEASE(mModuleDestructibleLegacy);
	SAFE_MODULE_RELEASE(mModuleFrameworkLegacy);
}

}
}
} // end namespace physx::apex
