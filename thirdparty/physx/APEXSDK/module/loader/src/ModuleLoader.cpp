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
#include <PsHashMap.h>
#include <PsHash.h>

#include "ModuleLoader.h"
#include "Modules.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace physx
{
namespace apex
{

#if !defined(_USRDLL)
#	define INSTANTIATE_MODULE(instFunc) do { instFunc(); } while(0)
#else
#	define INSTANTIATE_MODULE(instFunc)
#endif

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

	ModuleLoader* impl = PX_NEW(ModuleLoader)(inSdk);
	*niRef  = (NiModule*) impl;
	return (NxModule*) impl;
}

#else

void instantiateModuleLoader()
{
	NiApexSDK* sdk = NiGetApexSDK();
	ModuleLoader* impl = PX_NEW(ModuleLoader)(sdk);
	sdk->registerExternalModule((NxModule*) impl, (NiModule*) impl);
}

#endif

void ModuleLoader::loadModules(const char** names, PxU32 size, NxModule** modules)
{
	NX_WRITE_ZONE();
	typedef physx::HashMap<const char*, PxU32, physx::Hash<const char*> > MapName2Idx;
	MapName2Idx mapName2Idx;

	if (modules)
	{
		for (PxU32 i = 0; i < size; ++i)
		{
			mapName2Idx[names[i]] = i;
		}
	}

	// TODO: check that module is not yet loaded
#	define MODULE(name, instFunc, hasInit) { \
		if (mapName2Idx.find(name)) { \
			INSTANTIATE_MODULE(instFunc); \
			mCreateModuleError[name] = APEX_CE_NO_ERROR; \
			NxModule *module = mSdk->createModule(name, &mCreateModuleError[name]); \
			PX_ASSERT(module && name); \
			if (module) \
			{ \
				mModules.pushBack(module); \
				if( modules ) modules[mapName2Idx[name]] = module; \
				if( hasInit && module ) { \
					NxParameterized::Interface* params = module->getDefaultModuleDesc(); \
					module->init(*params); \
				} \
			} \
		} \
	}
#	include "ModuleXmacro.h"
}

// TODO: check that module is not yet loaded in all ModuleLoader::load*-methods

NxModule* ModuleLoader::loadModule(const char* name_)
{
	NX_WRITE_ZONE();
#	define MODULE(name, instFunc, hasInit) { \
		if( 0 == strcmp(name, name_) ) { \
			INSTANTIATE_MODULE(instFunc); \
			mCreateModuleError[name] = APEX_CE_NO_ERROR; \
			NxModule *module = mSdk->createModule(name, &mCreateModuleError[name]); \
			PX_ASSERT(module && name); \
			if (module) \
			{ \
				mModules.pushBack(module); \
				if( hasInit && module ) { \
					NxParameterized::Interface* params = module->getDefaultModuleDesc(); \
					module->init(*params); \
				} \
			} \
			return module; \
		} \
	}
#	include "ModuleXmacro.h"

	return 0;
}

void ModuleLoader::loadAllModules()
{
	NX_WRITE_ZONE();
	// TODO: check that module is not yet loaded
#	define MODULE(name, instFunc, hasInit) { \
		INSTANTIATE_MODULE(instFunc); \
		mCreateModuleError[name] = APEX_CE_NO_ERROR; \
		NxModule *module = mSdk->createModule(name, &mCreateModuleError[name]); \
		PX_ASSERT(module && name); \
		if (module) \
		{ \
			mModules.pushBack(module); \
			if( hasInit && module ) { \
				NxParameterized::Interface* params = module->getDefaultModuleDesc(); \
				module->init(*params); \
			} \
		} \
	}
#	include "ModuleXmacro.h"
}

const NxModuleLoader::ModuleNameErrorMap& ModuleLoader::getLoadedModulesErrors() const
{
	return mCreateModuleError;
}

void ModuleLoader::loadAllLegacyModules()
{
	NX_WRITE_ZONE();
	// TODO: check that module is not yet loaded
#	define MODULE(name, instFunc, hasInit)
#	define MODULE_LEGACY(name, instFunc, hasInit) { \
		INSTANTIATE_MODULE(instFunc); \
		mCreateModuleError[name] = APEX_CE_NO_ERROR; \
		NxModule *module = mSdk->createModule(name, &mCreateModuleError[name]); \
		PX_ASSERT(module && name); \
		if (module) \
		{ \
			mModules.pushBack(module); \
		} \
	}
#	include "ModuleXmacro.h"
}

physx::PxU32 ModuleLoader::getIdx(const char* name) const
{
	for (physx::PxU32 i = 0; i < mModules.size(); ++i)
		if (0 == strcmp(name, mModules[i]->getName()))
		{
			return i;
		}

	return PX_MAX_U32;
}

physx::PxU32 ModuleLoader::getIdx(NxModule* module) const
{
	for (physx::PxU32 i = 0; i < mModules.size(); ++i)
		if (module == mModules[i])
		{
			return i;
		}

	return PX_MAX_U32;
}

physx::PxU32 ModuleLoader::getLoadedModuleCount() const
{
	NX_READ_ZONE();
	return mModules.size();
}

NxModule* ModuleLoader::getLoadedModule(physx::PxU32 idx) const
{
	NX_READ_ZONE();
	return mModules[idx];
}

NxModule* ModuleLoader::getLoadedModule(const char* name) const
{
	NX_READ_ZONE();
	physx::PxU32 idx = getIdx(name);
	return  PX_MAX_U32 == idx ? 0 : mModules[idx];
}

void ModuleLoader::releaseModule(physx::PxU32 idx)
{
	NX_WRITE_ZONE();
	if (PX_MAX_U32 != idx)
	{
		mCreateModuleError.erase(mModules[idx]->getName());
		mSdk->releaseModule(mModules[idx]);
		mModules.remove(idx);
	}
}

void ModuleLoader::releaseModule(const char* name)
{
	NX_WRITE_ZONE();
	releaseModule(getIdx(name));
}

void ModuleLoader::releaseModule(NxModule* module)
{
	NX_WRITE_ZONE();
	releaseModule(getIdx(module));
}

void ModuleLoader::releaseModules(NxModule** modules, PxU32 size)
{
	for (physx::PxU32 i = 0; i < size; ++i)
	{
		releaseModule(modules[i]);
	}
}

void ModuleLoader::releaseModules(const char** names, PxU32 size)
{
	NX_WRITE_ZONE();
	for (physx::PxU32 i = 0; i < size; ++i)
	{
		releaseModule(names[i]);
	}
}

void ModuleLoader::releaseLoadedModules()
{
	NX_WRITE_ZONE();
	while (!mModules.empty())
	{
		releaseModule(physx::PxU32(0));
	}
}

}
} // physx::apex
