/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULELEGACY_H_
#define MODULELEGACY_H_

#include "NxApex.h"
#include "NiApexSDK.h"
#include "NiModule.h"
#include "Module.h"
#include "NxModuleLoader.h"
#include "PsArray.h"
#include "ApexRWLockable.h"

namespace physx
{
namespace apex
{

class ModuleLoader : public NxModuleLoader, public NiModule, public Module, public ApexRWLockable
{
	physx::Array<NxModule*> mModules; // Loaded modules
	NxModuleLoader::ModuleNameErrorMap mCreateModuleError; // Modules error during it creating

	physx::PxU32 getIdx(const char* name) const;

	physx::PxU32 getIdx(NxModule* module) const;

public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleLoader(NiApexSDK* inSdk)
	{
		name = "Loader";
		mSdk = inSdk;
		mApiProxy = this;
	}

	NxModule* loadModule(const char* name);

	void loadModules(const char** names, PxU32 size, NxModule** modules);

	void loadAllModules();

	const ModuleNameErrorMap& getLoadedModulesErrors() const;

	void loadAllLegacyModules();

	physx::PxU32 getLoadedModuleCount() const;

	NxModule* getLoadedModule(physx::PxU32 idx) const;

	NxModule* getLoadedModule(const char* name) const;

	void releaseModule(physx::PxU32 idx);

	void releaseModule(const char* name);

	void releaseModule(NxModule* module);

	void releaseModules(const char** modules, PxU32 size);

	void releaseModules(NxModule** modules, PxU32 size);

	void releaseLoadedModules();

protected:

	virtual ~ModuleLoader() {}

	// NxModule's stuff

	void						init(NxParameterized::Interface&) {}

	NxParameterized::Interface* getDefaultModuleDesc()
	{
		return 0;
	}

	void release()
	{
		Module::release();
	}
	void destroy()
	{
		Module::destroy();
		delete this;
	}

	const char*					getName() const
	{
		return Module::getName();
	}
	physx::PxU32				getNbParameters() const
	{
		return Module::getNbParameters();
	}
	NxApexParameter**			getParameters()
	{
		return Module::getParameters();
	}
	void setLODUnitCost(physx::PxF32 cost)
	{
		Module::setLODUnitCost(cost);
	}
	physx::PxF32 getLODUnitCost() const
	{
		return Module::getLODUnitCost();
	}
	void setLODBenefitValue(physx::PxF32 value)
	{
		Module::setLODBenefitValue(value);
	}
	physx::PxF32 getLODBenefitValue() const
	{
		return Module::getLODBenefitValue();
	}
	void setLODEnabled(bool enabled)
	{
		Module::setLODEnabled(enabled);
	}
	bool getLODEnabled() const
	{
		return Module::getLODEnabled();
	}
	void						setIntValue(physx::PxU32 /*parameterIndex*/, physx::PxU32 /*value*/) {}

	NxAuthObjTypeID				getModuleID() const
	{
		return PX_MAX_U32;
	}
	NxApexRenderableIterator* 	createRenderableIterator(const NxApexScene&)
	{
		return NULL;
	}

	// NiModule's stuff

	NiModuleScene* 				createNiModuleScene(NiApexScene&, NiApexRenderDebug*)
	{
		return NULL;
	}
	void						releaseNiModuleScene(NiModuleScene&) {}
	physx::PxU32				forceLoadAssets()
	{
		return 0;
	}
};

}
} // physx::apex

#endif
