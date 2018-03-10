/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NI_APEX_SDK_H
#define NI_APEX_SDK_H

/* Framework-internal interface class */

#include "NxApexSDK.h"
#include "NiResourceProvider.h"
#include "NiApexPhysXObjectDesc.h"

#include "PsString.h"
#include "foundation/PxErrors.h"

/* Every createModule() function should call this macro to initialize PxFoundation instance */
#include "PsFoundation.h"

#if defined(APEX_CUDA_SUPPORT)
namespace physx
{
	class PhysXGpuIndicator;
}
#endif

#if NX_SDK_VERSION_MAJOR == 2
// 
// The foundation instance is retrieved from the PhysXCommon DLL for dynamic lib configurations for PhysX3
// and from ApexFramework for PhysX28

# define APEX_INIT_FOUNDATION()


#elif NX_SDK_VERSION_MAJOR == 3

# define APEX_INIT_FOUNDATION()

#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace physx
{
	class PxProfileZone;
}

namespace PVD
{
	class PvdBinding;
}

namespace NxParameterized
{
class Traits;
};

namespace physx
{
namespace apex
{

class NiModule;
class NxApexActor;
class ApexActor;
class NiApexAuthorableObject;
class NxUserOpaqueMesh;

/**
Internal interface to the ApexSDK, available to Modules and Scenes
*/
class NiApexSDK : public NxApexSDK
{
public:
	/**
	Register an authorable object type with the SDK.  These IDs should be accessable
	from the NxModule* public interface so users can compare them against game objects
	in callbacks.
	*/
	virtual NxAuthObjTypeID			registerAuthObjType(const char*, NxResID nsid) = 0;
	virtual NxAuthObjTypeID			registerAuthObjType(const char*, NiApexAuthorableObject* authObjPtr) = 0;
	virtual NxAuthObjTypeID			registerNxParamAuthType(const char*, NiApexAuthorableObject* authObjPtr) = 0;
	virtual void					unregisterAuthObjType(const char*) = 0;
	virtual void					unregisterNxParamAuthType(const char*) = 0;
	/**
	Query the NxResID of an authorable object namespace.  This is useful if you have
	an authorable object class name, but not a module pointer.
	*/
	virtual NiApexAuthorableObject*	getAuthorableObject(const char*) = 0;
	virtual NiApexAuthorableObject*	getNxParamAuthObject(const char*) = 0;

	virtual NxApexAssetAuthoring*	createAssetAuthoring(const char* aoTypeName) = 0;
	virtual NxApexAssetAuthoring*	createAssetAuthoring(const char* aoTypeName, const char* name) = 0;
	virtual NxApexAsset*			createAsset(NxApexAssetAuthoring&, const char*) = 0;
	virtual NxApexAsset*			createAsset(NxParameterized::Interface*, const char*) = 0;
	virtual NxApexAsset*			createAsset(const char* opaqueMeshName, NxUserOpaqueMesh* om) = 0;
	virtual void					releaseAsset(NxApexAsset&) = 0;
	virtual void					releaseAssetAuthoring(NxApexAssetAuthoring&) = 0;
	/**
	Request an ApexActor pointer for an NxApexActor
	*/
	virtual ApexActor*              getApexActor(NxApexActor*) const = 0;

	/**
	When APEX creates a PhysX object and injects it into a PhysX scene, it must
	register that object with the ApexSDK so that the end user can differentiate between
	their own PhysX objects and those created by APEX.  The object descriptor will also
	provide a method for working their way back to the containing APEX data structure.
	*/
#if NX_SDK_VERSION_MAJOR == 2
	virtual NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxActor*) = 0;
	virtual NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxShape*) = 0;
	virtual NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxJoint*) = 0;
	virtual NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxCloth*) = 0;
	virtual NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxSoftBody*) = 0;
#else
	virtual NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxActor*) = 0;
	virtual NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxShape*) = 0;
	virtual NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxJoint*) = 0;
	virtual NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxCloth*) = 0;
#endif // NX_SDK_VERSION_MAJOR == 2

	/**
	Retrieve an object desc created by createObjectDesc.
	*/
	virtual NiApexPhysXObjectDesc* getGenericPhysXObjectInfo(const void*) const = 0;

	/**
	When APEX deletes a PhysX object or otherwise removes it from the PhysX scene, it must
	call this function to remove it from the ApexSDK object description cache.
	*/
	virtual void					releaseObjectDesc(void*) = 0;

	/* Utility functions intended to be used internally */
	virtual void reportError(physx::PxErrorCode::Enum code, const char* file, int line, const char* functionName, const char* message, ...) = 0;

	virtual physx::PxU32 getCookingVersion() const = 0;

	virtual void* getTempMemory(physx::PxU32 size) = 0;

	virtual void releaseTempMemory(void* data) = 0;

	/**
	 * ApexScenes and Modules can query the NiModule interfaces from the ApexSDK
	 */
	virtual NiModule** getNiModules() = 0;

	/**
	 * Returns the internal named resource provider.  The Ni interface allows
	 * resource creation and deletion
	 */
	virtual NiResourceProvider* getInternalResourceProvider() = 0;

	/**
	 * Allow 3rd party modules distributed as static libraries to link
	 * into the ApexSDK such that the normal NxApexSDK::createModule()
	 * will still work correctly.  The 3rd party lib must export a C
	 * function that instantiates their module and calls this function.
	 * The user must call that instantiation function before calling
	 * createModule() for it.
	 */
	virtual void registerExternalModule(NxModule* nx, NiModule* ni) = 0;

	/**
	 * Allow modules to fetch these ApexSDK global name spaces
	 */
	virtual NxResID getMaterialNameSpace() const = 0;
	virtual NxResID getOpaqueMeshNameSpace() const = 0;
	virtual NxResID getCustomVBNameSpace() const = 0;
	virtual NxResID getApexMeshNameSpace() = 0;
	virtual NxResID getCollisionGroupNameSpace() const = 0;
	virtual NxResID getCollisionGroup128NameSpace() const = 0;
	virtual NxResID getCollisionGroup64NameSpace() const = 0;
	virtual NxResID getCollisionGroupMaskNameSpace() const = 0;
	virtual NxResID getPhysicalMaterialNameSpace() const = 0;
	virtual NxResID getAuthorableTypesNameSpace() const = 0;

	/**
	 * Retrieve the user provided render resource manager
	 */
	virtual NxUserRenderResourceManager* getUserRenderResourceManager() const = 0;

	virtual physx::ErrorHandler& 		getErrorHandler() const = 0;

	virtual NxParameterized::Traits* 	getParameterizedTraits() = 0;

	/**
	 * Expose this pxtask API through NiApexSDK so modules do not have to link
	 * to pxtask.lib.
	 * Returns -1 if any of the following are true:
	 *    CUDA_SUPPORT was not defined when the APEX framework was compiled.
	 *    The user has disabled hardware physics in the control panel.
	 *    NVCUDA.dll is not found on the target system.
	 *    No CUDA compatible GPUs were found.
	 */
	virtual int  getSuggestedCudaDeviceOrdinal() = 0;

	virtual Foundation* getFoundation() const = 0;

	virtual NiModule* getNiModuleByName(const char* name) = 0;

	/**
	 * Update debug renderer color tables in each apex scene with NxParameterized color table.
	 */
	virtual void updateDebugColorParams(const char* color, physx::PxU32 val) = 0;

	virtual bool getRMALoadMaterialsLazily() = 0;

	virtual PVD::PvdBinding* getPvdBinding() = 0;
	virtual physx::PxProfileZone * getProfileZone() = 0;

	// applications can append strings to the APEX DLL filenames
	virtual const char* getCustomDllNamePostfix() const = 0;
#ifdef PX_WINDOWS
	/**
	 * Return the user-provided appGuid, or the default appGuid if the user didn't provide one.
	 */
	virtual const char* getAppGuid() = 0;
#endif

#if defined(APEX_CUDA_SUPPORT)
	virtual PhysXGpuIndicator* registerPhysXIndicatorGpuClient() = 0;
	virtual void unregisterPhysXIndicatorGpuClient(PhysXGpuIndicator* gpuIndicator) = 0;
#endif

	virtual NiModule *getNiModule(NxModule *module) = 0;
	virtual NxModule *getNxModule(NiModule *module) = 0;

	virtual void enterURR() = 0;
	virtual void leaveURR() = 0;
	virtual void checkURR() = 0;

protected:
	virtual ~NiApexSDK() {}

};

/**
Returns global SDK pointer.  Not sure if we can do this.
*/
NXAPEX_API NiApexSDK*   NX_CALL_CONV NiGetApexSDK();

}
} // end namespace physx::apex

#define APEX_SPRINTF_S(dest_buf, n, str_fmt, ...) \
	physx::string::sprintf_s((char *) dest_buf, n, str_fmt, ##__VA_ARGS__);

// gcc uses names ...s
#define APEX_INVALID_PARAMETER(_A, ...) \
	physx::NiGetApexSDK()->reportError(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, __FUNCTION__, _A, ##__VA_ARGS__)
#define APEX_INVALID_OPERATION(_A, ...) \
	physx::NiGetApexSDK()->reportError(physx::PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, __FUNCTION__, _A, ##__VA_ARGS__)
#define APEX_INTERNAL_ERROR(_A, ...) \
	physx::NiGetApexSDK()->reportError(physx::PxErrorCode::eINTERNAL_ERROR    , __FILE__, __LINE__, __FUNCTION__, _A, ##__VA_ARGS__)
#define APEX_DEBUG_INFO(_A, ...) \
	physx::NiGetApexSDK()->reportError(physx::PxErrorCode::eDEBUG_INFO          , __FILE__, __LINE__, __FUNCTION__, _A, ##__VA_ARGS__)
#define APEX_DEBUG_WARNING(_A, ...) \
	physx::NiGetApexSDK()->reportError(physx::PxErrorCode::eDEBUG_WARNING       , __FILE__, __LINE__, __FUNCTION__, _A, ##__VA_ARGS__)
#define APEX_DEPRECATED() \
	physx::NiGetApexSDK()->reportError(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, __FUNCTION__, "This method is deprecated")
#define APEX_DEPRECATED_ONCE() \
	{ static bool firstTime = true; if (firstTime) { firstTime = false; APEX_DEPRECATED(); } }


#if defined(PX_DEBUG) || defined(PX_CHECKED)

namespace physx
{
	namespace apex
	{
		class UpdateRenderResourcesScope
		{
		public:
			PX_INLINE	UpdateRenderResourcesScope() { NiGetApexSDK()->enterURR(); }
			PX_INLINE	~UpdateRenderResourcesScope() { NiGetApexSDK()->leaveURR(); }
		};
	}
}

# define URR_SCOPE UpdateRenderResourcesScope updateRenderResourcesScope
# define URR_CHECK NiGetApexSDK()->checkURR()
#else
# define URR_SCOPE
# define URR_CHECK
#endif


#endif // NI_APEX_SDK_H
