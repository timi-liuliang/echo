/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_SDK_H
#define NX_APEX_SDK_H

/*!
\file
\brief APEX SDK classes
*/

#include "NxApexDefs.h"
#include "NxApexDesc.h"
#include "NxApexInterface.h"

#include "NxSerializer.h"
#include "NxParameterized.h"
#include "NxParameterizedTraits.h"
#include "NxApexShape.h"

namespace physx
{
	class PxFoundation;
	class PxCpuDispatcher;
	class PxCudaContextManagerDesc;
	class PxCudaContextManager;
}

#if NX_SDK_VERSION_MAJOR == 3

/**
PhysX 3.0 SDK classes
*/
namespace physx
{
class PxPhysics;
class PxCloth;
class PxJoint;
class PxCooking;
class PxActor;
class PxShape;
};

#else

#include "Nxp.h"
/**
PhysX 2.8 SDK classes
*/
class NxShape;
class NxActor;
class NxJoint;
class NxCloth;
class NxSoftBody;
class NxPhysicsSDK;
class NxCookingInterface;
class NxDebugRenderable;
#endif

namespace PVD
{
class PvdBinding;
}

//! \brief physx namespace
namespace physx
{
//! \brief apex namespace
namespace apex
{

class NxUserRenderResourceManager;
class NxApexScene;
class NxApexSceneDesc;
class NxApexAssetPreviewScene;
class NxRenderMeshAsset;
class NxRenderMeshAssetAuthoring;
class NxModule;
class NxApexPhysXObjectDesc;
class NxResourceProvider;
class NxResourceCallback;
class NxApexRenderDebug;
class NxApexAsset;
class NxApexAssetAuthoring;
class NxApexSDKCachedData;
class NxRenderMeshActor;

PX_PUSH_PACK_DEFAULT

/**
\brief 64-bit mask used for collision filtering between IOSes and Field Samplers.
*/
class NxGroupsMask64
{
public:
	/**
	\brief Default constructor.
	*/
	PX_INLINE NxGroupsMask64() 
	{
		bits0 = bits1 = 0;
	}

	/**
	\brief Constructor to set filter data initially.
	*/
	PX_INLINE NxGroupsMask64(PxU32 b0, PxU32 b1) : bits0(b0), bits1(b1) {}

	/**
	\brief (re)sets the structure to the default.	
	*/
	PX_INLINE void setToDefault()
	{
		*this = NxGroupsMask64();
	}

	/**	\brief Mask bits. */
	physx::PxU32		bits0;
	/**	\brief Mask bits. */
	physx::PxU32		bits1;
};

/**
\brief Collision groups filtering operations.
*/
struct NxGroupsFilterOp
{
	/**
	\brief Enum of names for group filtering operations.
	*/
	enum Enum
	{
		AND,
		OR,
		XOR,
		NAND,
		NOR,
		NXOR,
		SWAP_AND
	};
};

/**
\brief Integer values, each representing a unique authorable object class/type
*/
typedef unsigned int NxAuthObjTypeID;

/**
\brief Descriptor class for NxApexSDK
*/
class NxApexSDKDesc : public NxApexDesc
{
public:
#if NX_SDK_VERSION_MAJOR == 2
	/**
	\brief The allocator APEX should use when running with PhysX 2.8.x

	If left blank, APEX will use the PhysX allocator
	*/
	physx::PxAllocatorCallback* allocator;

	/**
	\brief The error callback (formerly called output stream) that APEX should use when running with PhysX 2.8.x
	*/
	physx::PxErrorCallback* outputStream;
#endif

	/**
	\brief The PhysX SDK version you are building with

	A particular APEX build will be against a particular PhysX version.
	Versioning the PhysX API will require versioning of APEX.
	*/
	physx::PxU32 physXSDKVersion;

	/**
	\brief Pointer to the physX sdk (PhysX SDK version specific structure)
	*/
#if NX_SDK_VERSION_MAJOR == 2
	NxPhysicsSDK* physXSDK;
#else
	PxPhysics* physXSDK;
#endif

	/**
	\brief Pointer to the cooking interface (PhysX SDK version specific structure)
	*/
#if NX_SDK_VERSION_MAJOR == 2
	NxCookingInterface* cooking;
#else
	PxCooking* cooking;
#endif

	/**
	\brief User defined interface for creating renderable resources.
	*/
	NxUserRenderResourceManager* renderResourceManager;

	/**
	\brief Pointer to a user-callback for unresolved named resources.

	This function will be called by APEX when
	an unresolved named resource is requested.  The function will be called at most once for each named
	resource.  The function must directly return the pointer to the resource or NULL.
	*/
	NxResourceCallback* resourceCallback;

	/**
	\brief Path to APEX module DLLs (Windows Only)

	If specified, this string will be prepended to the APEX module names prior to calling LoadLibrary() to load them.  You can
	use this mechanism to ship the APEX module DLLS into their own subdirectory.  For example:  "APEX/"
	*/
	const char* dllLoadPath;

	/**
	\brief Wireframe material name

	If specified, this string designates the material to use when rendering wireframe data.
	It not specified, the default value is "ApexWireframe"
	*/
	const char* wireframeMaterial;

	/**
	\brief Solid material name

	If specified, this string designates the material to use when rendering solid shaded (per vertex color) lit triangles.
	If not specified, the default value is "ApexSolidShaded"
	*/
	const char* solidShadedMaterial;

	/**
	\brief Delays material loading

	Any NxRenderMeshActor will issue callbacks to the NxResourceCallback to create one or more materials. This happens either
	at actor creation or lazily during updateRenderResource upon necessity.
	Additionally it will call NxUserRenderResourceManager::getMaxBonesPerMaterial for the freshly created material right
	after a successful material creation.
	If this is set to true, the updateRenderResource calls should not run asynchronously.
	*/
	bool renderMeshActorLoadMaterialsLazily;

	/**
	\brief APEX module DLL name postfix (Windows Only)

	If specified, this string will be appended to the APEX module DLL names prior to calling LoadLibrary() to load them.  You can
	use this mechanism to load the APEX module DLLS for two different PhysX SDKs in the same process (DCC tools).

	For example, the APEX_Destructible_x86.dll is renamed to APEX_Destructible_x86_PhysX-2.8.4.dll.
	dllNamePostfix would be "_PhysX-2.8.4"
	*/
	const char* dllNamePostfix;

	/**
	\brief Application-specific GUID (Windows Only)

	Provide an optional appGuid if you have made local modifications to your APEX DLLs.
	Application GUIDs are available from NVIDIA. For non-Windows platforms the appGuid
	is ignored.
	*/
	const char* appGuid;


	/**
	\brief Sets the resource provider's case sensitive mode.

	\note This is a reference to the NxResourceProvider string lookups
	*/
	bool resourceProviderIsCaseSensitive;

	/**
	\brief Sets the number of physX object descriptor table entries that are allocated when it gets full.

	*/
	physx::PxU32 physXObjDescTableAllocationIncrement;

	/**
	\brief Enables warnings upon detection of concurent access to the APEX SDK

	*/
	bool enableConcurrencyCheck;

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE NxApexSDKDesc() : NxApexDesc()
	{
		init();
	}

	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void setToDefault()
	{
		NxApexDesc::setToDefault();
		init();
	}

	/**
	\brief Returns true if the descriptor is valid.
	\return true if the current settings are valid
	*/
	PX_INLINE bool isValid() const
	{
		bool retVal = NxApexDesc::isValid();

		if (renderResourceManager == NULL)
		{
			return false;
		}
		if (physXSDK == NULL)
		{
			return false;
		}
		if (cooking == NULL)
		{
			return false;
		}
		if (physXSDKVersion != NX_PHYSICS_SDK_VERSION)
		{
			return false;
		}
#if NX_SDK_VERSION_MAJOR == 2
		if (outputStream == NULL)
		{
			return false;
		}
#endif
		return retVal;
	}

private:
	PX_INLINE void init()
	{
		renderResourceManager = NULL;
#if NX_SDK_VERSION_MAJOR == 2
		allocator = NULL;
		outputStream = NULL;
#endif
		physXSDKVersion = NX_PHYSICS_SDK_VERSION;
		physXSDK = NULL;
		cooking = NULL;
		resourceCallback = NULL;
		dllLoadPath = NULL;
		solidShadedMaterial = "ApexSolidShaded";
		wireframeMaterial = "ApexWireframe";
		renderMeshActorLoadMaterialsLazily = true;
		dllNamePostfix = NULL;
		appGuid = NULL;
		resourceProviderIsCaseSensitive = false;
		physXObjDescTableAllocationIncrement = 128;
		enableConcurrencyCheck = false;
	}
};


/**
\brief These errors are returned by the NxCreateApexSDK() and NxApexSDK::createModule() functions
*/
enum NxApexCreateError
{
	/**
	\brief No errors occurred when creating the Physics SDK.
	*/
	APEX_CE_NO_ERROR = 0,

	/**
	\brief Unable to find the libraries.
	For statically linked APEX, a module specific instantiate function must be called
	prior to the createModule call.
	*/
	APEX_CE_NOT_FOUND = 1,

	/**
	\brief The application supplied a version number that does not match with the libraries.
	*/
	APEX_CE_WRONG_VERSION = 2,

	/**
	\brief The supplied descriptor is invalid.
	*/
	APEX_CE_DESCRIPTOR_INVALID = 3,

	/**
	\brief This module cannot be created by the application, it is created via a parent module use as APEX_Particles or APEX_Legacy
	*/
	APEX_CE_CREATE_NO_ALLOWED = 4,

};


/**
\brief The ApexSDK abstraction. Manages scenes and modules.
*/
class NxApexSDK : public NxApexInterface
{
public:
	/**
	\brief Create an APEX Scene
	*/
	virtual NxApexScene* createScene(const NxApexSceneDesc&) = 0;

	/**
	\brief Release an APEX Scene
	*/
	virtual void releaseScene(NxApexScene*) = 0;

	/**
	\brief Create an APEX Asset Preview Scene
	*/
	virtual NxApexAssetPreviewScene* createAssetPreviewScene() = 0;

	/**
	\brief Release an APEX Asset Preview Scene
	*/
	virtual void releaseAssetPreviewScene(NxApexAssetPreviewScene* nxScene) = 0;

#if NX_SDK_VERSION_MAJOR == 2
	/**
	 \brief Allocates a CpuDispatcher with a thread pool of the specified number of threads

	 If numThreads is zero, the thread pool will use the default number of threads for the
	 current platform.
	*/
	virtual physx::PxCpuDispatcher* createCpuDispatcher(physx::PxU32 numThreads = 0) = 0;
	/**
	 \brief Releases a CpuDispatcher
	*/
	virtual void						   releaseCpuDispatcher(physx::PxCpuDispatcher& cd) = 0;

	/**
	\brief Create a physx::PxCudaContextManager
	*/
	virtual physx::PxCudaContextManager* createCudaContextManager(const physx::PxCudaContextManagerDesc& desc) = 0;

	/**
	\brief Get the PxFoundation instance
	*/
	virtual physx::PxFoundation* getPxFoundation() const = 0;
#endif

	/**
	\brief Create/Load a module
	*/
	virtual NxModule* createModule(const char* name, NxApexCreateError* err = NULL) = 0;

	/**
	\brief Return an object describing how APEX is using the PhysX Actor.
	\return NULL if PhysX Actor is not owned by APEX.
	*/
#if NX_SDK_VERSION_MAJOR == 2
	virtual const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxActor* actor) const = 0;
#else
	virtual const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxActor* actor) const = 0;
#endif

	/**
	\brief Return an object describing how APEX is using the PhysX Shape.
	\return NULL if PhysX Shape is not owned by APEX.
	*/
#if NX_SDK_VERSION_MAJOR == 2
	virtual const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxShape* shape) const = 0;
#else
	virtual const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxShape* shape) const = 0;
#endif

	/**
	\brief Return an object describing how APEX is using the PhysX Joint.
	\return NULL if PhysX Joint is not owned by APEX.
	*/
#if NX_SDK_VERSION_MAJOR == 2
	virtual const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxJoint* joint) const = 0;
#else
	virtual const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxJoint* joint) const = 0;
#endif


	/**
	\brief Return an object describing how APEX is using the PhysX Cloth.
	\return NULL if PhysX Cloth is not owned by APEX.
	*/
#if NX_SDK_VERSION_MAJOR == 2
	virtual const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxCloth* cloth) const = 0;
#else
	virtual const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxCloth* cloth) const = 0;
#endif

#if NX_SDK_VERSION_MAJOR == 2
	/**
	\brief Return an object describing how APEX is using the PhysX SoftBody.
	\return NULL if PhysX SoftBody is not owned by APEX.
	*/
	virtual const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxSoftBody* softbody) const = 0;
#endif

	/**
	\brief Return the user error callback.
	\deprecated Use getErrorCallback() instead.
	*/
	PX_DEPRECATED virtual physx::PxErrorCallback* getOutputStream() = 0;

	/**
	\brief Return the user error callback.
	*/
	virtual physx::PxErrorCallback* getErrorCallback() = 0;

	/**
	\brief Returns the cooking interface.
	*/
#if NX_SDK_VERSION_MAJOR == 2
	virtual NxCookingInterface* getCookingInterface() = 0;
#else
	virtual PxCooking* getCookingInterface() = 0;
#endif

	/**
	\brief Return the named resource provider.
	*/
	virtual NxResourceProvider* getNamedResourceProvider() = 0;

	/**
	\brief Return an APEX physx::general_PxIOStream2::PxFileBuf instance.  For use by APEX
	tools and samples, not by APEX internally. Internally,	APEX will use an
	physx::general_PxIOStream2::PxFileBuf directly provided by the user.
	*/
	virtual physx::general_PxIOStream2::PxFileBuf* createStream(const char* filename, physx::general_PxIOStream2::PxFileBuf::OpenMode mode) = 0;

	/**
	\brief Return a physx::general_PxIOStream2::PxFileBuf which reads from a buffer in memory.
	*/
	virtual physx::general_PxIOStream2::PxFileBuf* createMemoryReadStream(const void* mem, physx::PxU32 len) = 0;

	/**
	\brief Return a physx::general_PxIOStream2::PxFileBuf which writes to memory.
	*/
	virtual physx::general_PxIOStream2::PxFileBuf* createMemoryWriteStream(physx::PxU32 alignment = 0) = 0;

	/**
	\brief Return the address and length of the contents of a memory write buffer stream.
	*/
	virtual const void* getMemoryWriteBuffer(physx::general_PxIOStream2::PxFileBuf& stream, physx::PxU32& len) = 0;

	/**
	\brief Release a previously created physx::general_PxIOStream2::PxFileBuf used as a read stream
	*/
	virtual void releaseMemoryReadStream(physx::general_PxIOStream2::PxFileBuf& stream) = 0;

	/**
	\brief Release a previously created physx::general_PxIOStream2::PxFileBuf used as a write stream
	*/
	virtual void releaseMemoryWriteStream(physx::general_PxIOStream2::PxFileBuf& stream) = 0;

	/**
	\brief Return the PhysX SDK.
	*/
#if NX_SDK_VERSION_MAJOR == 2
	virtual NxPhysicsSDK* getPhysXSDK() = 0;
#else
	virtual PxPhysics* getPhysXSDK() = 0;
#endif

	/**
	\brief Return the number of modules.
	*/
	virtual physx::PxU32 getNbModules() = 0;

	/**
	\brief Return an array of module pointers.
	*/
	virtual NxModule** getModules() = 0;

	/**
	\brief Release a previously loaded module
	*/
	virtual void releaseModule(NxModule* module) = 0;

	/**
	\brief Creates an ApexDebugRender interface
	*/
	virtual NxApexRenderDebug* createApexRenderDebug(bool useRemoteDebugVisualization = false, bool debugVisualizeLocally = true) = 0;

	/**
	\brief Releases an ApexDebugRender interface
	*/
	virtual void releaseApexRenderDebug(NxApexRenderDebug& debug) = 0;

	/**
	\brief Creates an ApexSphereShape interface
	*/
	virtual NxApexSphereShape* createApexSphereShape() = 0;

	/**
	\brief Creates an ApexCapsuleShape interface
	*/
	virtual NxApexCapsuleShape* createApexCapsuleShape() = 0;

	/**
	\brief Creates an ApexBoxShape interface
	*/
	virtual NxApexBoxShape* createApexBoxShape() = 0;

	/**
	\brief Creates an ApexHalfSpaceShape interface
	*/
	virtual NxApexHalfSpaceShape* createApexHalfSpaceShape() = 0;

	/**
	\brief Release an NxApexShape interface
	*/
	virtual void releaseApexShape(NxApexShape& shape) = 0;

	/**
	\brief Return the number of assets force loaded by all of the existing APEX modules
	*/
	virtual physx::PxU32 forceLoadAssets() = 0;

	/**
	\brief Get a list of the APEX authorable types.

	The memory for the strings returned is owned by the APEX SDK and should only be read, not written or freed.
	Returns false if 'inCount' is not large enough to contain all of the names.
	*/
	virtual bool getAuthorableObjectNames(const char** authTypeNames, physx::PxU32& outCount, physx::PxU32 inCount) = 0;

	/**
	\brief Get the a pointer to APEX's instance of the ::NxParameterized::Traits class
	*/
	virtual ::NxParameterized::Traits* getParameterizedTraits() = 0;

	/**
	\brief Creates an APEX asset from an NxApexAssetAuthoring object.
	*/
	virtual NxApexAsset* createAsset(NxApexAssetAuthoring&, const char* name) = 0;

	/**
	\brief Creates an APEX asset from a parameterized object.
	\note The NxParameterized::Interface object's ownership passes to the asset, if a copy is needed, use the NxParameterized::Interface::copy() method
	*/
	virtual NxApexAsset* createAsset(::NxParameterized::Interface*, const char* name) = 0;

	/**
	\brief Release an APEX asset
	*/
	virtual void releaseAsset(NxApexAsset&) = 0;

	/**
	\brief Creates an APEX asset authoring object
	*/
	virtual NxApexAssetAuthoring* createAssetAuthoring(const char* authorTypeName) = 0;

	/**
	\brief Create an APEX asset authoring object with a name
	*/
	virtual NxApexAssetAuthoring* createAssetAuthoring(const char* authorTypeName, const char* name) = 0;

	/**
	\brief Create an APEX asset authoring object from a parameterized object.
	*/
	virtual NxApexAssetAuthoring* createAssetAuthoring(::NxParameterized::Interface*, const char* name) = 0;

	/**
	\brief Releases an APEX asset authoring object
	*/
	virtual void releaseAssetAuthoring(NxApexAssetAuthoring&) = 0;

	/**
	\brief Returns the scene data cache for actors in the scene
	*/
	virtual NxApexSDKCachedData& getCachedData() const = 0;

	/**
	\brief Create a Serialization object
	*/
	virtual ::NxParameterized::Serializer* createSerializer(::NxParameterized::Serializer::SerializeType type) = 0;

	/**
	\brief Create a Serialization object from custom traits
	*/
	virtual ::NxParameterized::Serializer* createSerializer(::NxParameterized::Serializer::SerializeType type, ::NxParameterized::Traits* traits) = 0;

	/**
	\brief Figure out whether a given chunk of data was xml or binary serialized.
	To properly detect XML formats, 32 bytes of data are scanned, so dlen should be at least 32 bytes
	*/
	virtual ::NxParameterized::Serializer::SerializeType getSerializeType(const void* data, PxU32 dlen) = 0;

	/**
	\brief Figure out whether a given chunk of data was xml or binary serialized.
	*/
	virtual ::NxParameterized::Serializer::SerializeType getSerializeType(physx::general_PxIOStream2::PxFileBuf& stream) = 0;

	/**
	\brief Find native platform for a given chunk of data.
	*/
	virtual NxParameterized::Serializer::ErrorType getSerializePlatform(physx::general_PxIOStream2::PxFileBuf& stream, NxParameterized::SerializePlatform& platform) = 0;

	/**
	\brief Find native platform for a given chunk of data.
	\note To properly detect platform 64 bytes of data are scanned, so dlen should be at least 64 bytes
	*/
	virtual NxParameterized::Serializer::ErrorType getSerializePlatform(const void* data, PxU32 dlen, NxParameterized::SerializePlatform& platform) = 0;

	/**
	\brief Get current (native) platform
	*/
	virtual void getCurrentPlatform(NxParameterized::SerializePlatform& platform) const = 0;

	/**
	\brief Get platform from human-readable name
	\param [in]		name		platform name
	\param [out]	platform	Platform corresponding to name
	\return Success

	Name format: compiler + compiler version (if needed) + architecture.
	Supported names: VcWin32, VcWin64, VcXbox (or VcXbox360), VcXboxOne, GccPs3 (both Gcc and Snc), GccPs4, AndroidARM, GccLinux32, GccLinux64, GccOsX32, Pib.
	*/
	virtual bool getPlatformFromString(const char* name, NxParameterized::SerializePlatform& platform) const = 0;

	/**
	\brief Get canonical name for platform
	*/
	virtual const char* getPlatformName(const NxParameterized::SerializePlatform& platform) const = 0;

	/**
	\brief Gets NxParameterized debug rendering color parameters.
	Modiying NxParameterized debug colors automatically updates color table in debug renderer
	*/
	virtual ::NxParameterized::Interface* getDebugColorParams() const = 0;

	/**
	\brief Gets a string for the material to use when rendering wireframe data.
	*/
	virtual const char* getWireframeMaterial() = 0;

	/**
	\brief Gets a string for the material to use when rendering solid shaded (per vertex color) lit triangles.
	*/
	virtual const char* getSolidShadedMaterial() = 0;

	/**
	\brief Get the PhysX Visual Debugger binding
	*/
	virtual PVD::PvdBinding* getPvdBinding() = 0;

	/**
	\brief Enable or disable the APEX module-specific stat collection (some modules can be time consuming)
	*/
	virtual void setEnableApexStats(bool enableApexStats) = 0;

	/**
	\brief Enable or disable the APEX concurrent access check
	*/
	virtual void setEnableConcurrencyCheck(bool enableConcurrencyChecks) = 0;

	/**
	\brief Returns current setting for APEX concurrent access check
	*/
	virtual bool isConcurrencyCheckEnabled() = 0;

protected:
	virtual ~NxApexSDK() {}

};

/**
\def NXAPEX_API
\brief Export the function declaration from its DLL
*/

/**
\def NX_CALL_CONV
\brief Use C calling convention, required for exported functions
*/

PX_POP_PACK

#ifdef NX_CALL_CONV
#undef NX_CALL_CONV
#endif

#if defined(PX_WINDOWS)

#define NXAPEX_API extern "C" __declspec(dllexport)
#define NX_CALL_CONV __cdecl
#else
#define NXAPEX_API extern "C"
#define NX_CALL_CONV /* void */
#endif

/**
\brief Global function to create the SDK object.  NX_APEX_SDK_VERSION must be passed for the APEXsdkVersion.
*/
NXAPEX_API NxApexSDK*	NX_CALL_CONV NxCreateApexSDK(const NxApexSDKDesc& desc, NxApexCreateError* errorCode = NULL, physx::PxU32 APEXsdkVersion = NX_APEX_SDK_VERSION);

/**
\brief Returns global SDK pointer.
*/
NXAPEX_API NxApexSDK*	NX_CALL_CONV NxGetApexSDK();

}
} // end namespace physx::apex

#endif // NX_APEX_SDK_H
