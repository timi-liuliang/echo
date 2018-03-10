/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_MODULE_LOADER_H
#define NX_MODULE_LOADER_H

#include "NxApex.h"
#include "PsHashMap.h"

namespace physx
{
namespace apex
{

/**
\brief The NxModuleLoader is a utility class for loading APEX modules

\note The most useful methods for rapid integration are "loadAllModules()",
      "loadAllLegacyModules()", and "releaseLoadedModules()".

\note If you need to release APEX modules loaded with NxModuleLoader you should use one of
      the release-methods provided below. Note that you may never need it because SDK automatically
	  releases all modules when program ends.
*/
class NxModuleLoader : public NxModule
{
public:
	/// \brief Load and initialize a specific APEX module
	virtual NxModule* loadModule(const char* name) = 0;

	/**
	\brief Load and initialize a list of specific APEX modules
	\param [in] modules The modules array must be the same size as the names array to
	                    support storage of every loaded NxModule pointer. Use NULL if
	                    you do not need the list of created modules.
	 */
	virtual void loadModules(const char** names, PxU32 size, NxModule** modules = 0) = 0;

	/// \brief Load and initialize all APEX modules
	virtual void loadAllModules() = 0;

	/// \brief Load and initialize all legacy APEX modules (useful for deserializing legacy assets)
	virtual void loadAllLegacyModules() = 0;

	/// \brief Returns the number of loaded APEX modules
	virtual physx::PxU32 getLoadedModuleCount() const = 0;

	/// \brief Returns the APEX module specified by the index if it was loaded by this NxModuleLoader
	virtual NxModule* getLoadedModule(physx::PxU32 idx) const = 0;

	/// \brief Returns the APEX module specified by the name if it was loaded by this NxModuleLoader
	virtual NxModule* getLoadedModule(const char* name) const = 0;

	/// \brief Releases the APEX module specified by the index if it was loaded by this NxModuleLoader
	virtual void releaseModule(physx::PxU32 idx) = 0;

	/// \brief Releases the APEX module specified by the name if it was loaded by this NxModuleLoader
	virtual void releaseModule(const char* name) = 0;

	/// \brief Releases the APEX module specified by the index if it was loaded by this NxModuleLoader
	virtual void releaseModules(NxModule** modules, PxU32 size) = 0;

	/**
	\brief Releases the specified APEX module
	\note If the NxModuleLoader is used to load modules, this method must be used to
	      release individual modules.
	      Do not use the NxModule::release() method.
	*/
	virtual void releaseModule(NxModule* module) = 0;

	/**
	\brief Releases the APEX modules specified in the names list
	\note If the NxModuleLoader is used to load modules, this method must be used to
	      release individual modules.
	      Do not use the NxModule::release() method.
	*/
	virtual void releaseModules(const char** names, PxU32 size) = 0;

	/// \brief Releases all APEX modules loaded by this NxModuleLoader
	virtual void releaseLoadedModules() = 0;

	typedef physx::shdfnd::HashMap<const char*, physx::apex::NxApexCreateError> ModuleNameErrorMap;
	virtual const ModuleNameErrorMap& getLoadedModulesErrors() const = 0;
};

#if !defined(_USRDLL)
void instantiateModuleLoader();
#endif

}
} // end namespace physx::apex

#endif // NX_MODULE_LOADER_H
