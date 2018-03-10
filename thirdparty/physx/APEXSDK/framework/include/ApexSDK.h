/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_SDK_H
#define APEX_SDK_H

#include "NxApex.h"
#include "NxParameterizedTraits.h"
#include "ApexInterface.h"
#include "ApexSDKHelpers.h"
#include "ApexContext.h"
#include "ApexPhysXObjectDesc.h"
#include "FrameworkPerfScope.h"
#include "NiApexSDK.h"
#include "NiApexAuthorableObject.h"
#include "NiApexScene.h"
#include "NiApexRenderDebug.h"
#include "ApexRenderMeshAsset.h"
#include "ApexAuthorableObject.h"
#include "ApexParameterizedTraits.h"
#include "NiModule.h"
#include "DebugColorParamsEx.h"

#include "CommonParamClasses.h"
#include "FrameworkParamClasses.h"

#include "ApexSDKCachedData.h"

#if defined(APEX_CUDA_SUPPORT)
namespace physx
{
	class PhysXGpuIndicator;
}
#endif

namespace physx
{
class PxCudaContextManager;
class PxCudaContextManagerDesc;
class PxCpuDispatcher;
};

namespace PVD
{
class PvdDataStream;
class PvdBinding;
}


namespace physx
{
namespace apex
{

class ApexResourceProvider;
class ApexScene;
class NxRenderMeshAsset;

typedef PxFileBuf* (CreateStreamFunc)(const char* filename, physx::PxFileBuf::OpenMode mode);


// ApexAuthorableObject needs a "module", so we'll give it one that
// describes the APEX framework with regards to render mesh stuff
// needed: NiModule, no methods implemented
class ModuleFramework : public NiModule
{
	void destroy() {}

	NiModuleScene* createNiModuleScene(NiApexScene& apexScene, NiApexRenderDebug* rd)
	{
		PX_UNUSED(apexScene);
		PX_UNUSED(rd);
		return NULL;
	}

	void           releaseNiModuleScene(NiModuleScene& moduleScene)
	{
		PX_UNUSED(moduleScene);
	}

public:

	void init(NxParameterized::Traits* t);
	void release(NxParameterized::Traits* t);

#	define PARAM_CLASS(clas) PARAM_CLASS_DECLARE_FACTORY(clas)
#	include "FrameworkParamClasses.inc"

#	define PARAM_CLASS(clas) PARAM_CLASS_DECLARE_FACTORY(clas)
#	include "CommonParamClasses.inc"
};


class ApexSDK : public NiApexSDK, public ApexRWLockable, public physx::UserAllocated
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ApexSDK(NxApexCreateError* errorCode, physx::PxU32 APEXsdkVersion);
	void					init(const NxApexSDKDesc& desc, bool ownsFoundation);
	virtual Foundation*		getFoundation() const;

	/* NxApexSDK */
	NxApexScene* 			createScene(const NxApexSceneDesc&);
	void					releaseScene(NxApexScene* scene);
	NxApexAssetPreviewScene* createAssetPreviewScene();
	void					releaseAssetPreviewScene(NxApexAssetPreviewScene* nxScene);

	NxModule* 				createModule(const char* name, NxApexCreateError* err);

#if NX_SDK_VERSION_MAJOR == 2
	physx::PxCpuDispatcher* createCpuDispatcher(physx::PxU32 numThreads);
	physx::PxCudaContextManager* createCudaContextManager(const physx::PxCudaContextManagerDesc& desc);
	physx::PxCpuDispatcher* getDefaultThreadPool();
	void						   releaseCpuDispatcher(physx::PxCpuDispatcher& cd);

	physx::PxFoundation* getPxFoundation() const;
#endif

	/**
	Creates/releases an ApexDebugRender interface
	*/
	virtual NxApexRenderDebug* createApexRenderDebug(bool useRemoteDebugVisualization, bool debugVisualizeLocally);
	virtual void                releaseApexRenderDebug(NxApexRenderDebug& debug);


	/**
	Create/release ApexShape interfaces
	*/

	virtual NxApexSphereShape* createApexSphereShape();
	virtual NxApexCapsuleShape* createApexCapsuleShape();
	virtual NxApexBoxShape* createApexBoxShape();
	virtual NxApexHalfSpaceShape* createApexHalfSpaceShape();

	virtual void releaseApexShape(NxApexShape& shape);

#if NX_SDK_VERSION_MAJOR == 2
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxActor*);
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxShape*);
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxJoint*);
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxCloth*);
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const NxSoftBody*);
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxActor*) const;
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxShape*) const;
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxJoint*) const;
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxCloth*) const;
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const NxSoftBody*) const;
	NxCookingInterface* 	getCookingInterface();
	NxPhysicsSDK* 			getPhysXSDK();
#else
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxActor*);
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxShape*);
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxJoint*);
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxCloth*);
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxParticleSystem*);
	NiApexPhysXObjectDesc* 	createObjectDesc(const NxApexActor*, const PxParticleFluid*);
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxActor*) const;
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxShape*) const;
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxJoint*) const;
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxCloth*) const;
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxParticleSystem*) const;
	const NxApexPhysXObjectDesc* getPhysXObjectInfo(const PxParticleFluid*) const;
	PxCooking* 	            getCookingInterface();
	PxPhysics* 	    		getPhysXSDK();
#endif // NX_SDK_VERSION_MAJOR == 2

	ApexActor*              getApexActor(NxApexActor*) const;

	// deprecated, use getErrorCallback()
	PxErrorCallback* 		getOutputStream();
	PxErrorCallback* 		getErrorCallback();
	NxResourceProvider* 	getNamedResourceProvider();
	NiResourceProvider* 	getInternalResourceProvider();
	physx::PxFileBuf* 		createStream(const char* filename, physx::PxFileBuf::OpenMode mode);
	physx::PxFileBuf* 		createMemoryReadStream(const void* mem, physx::PxU32 len);
	physx::PxFileBuf* 		createMemoryWriteStream(physx::PxU32 alignment = 0);
	const void* 			getMemoryWriteBuffer(physx::PxFileBuf& stream, physx::PxU32& len);
	void					releaseMemoryReadStream(physx::PxFileBuf& stream);
	void					releaseMemoryWriteStream(physx::PxFileBuf& stream);

	physx::PxU32			getNbModules();
	NxModule**				getModules();
	NiModule**				getNiModules();
	void					releaseModule(NxModule* module);
	NiModule*				getNiModuleByName(const char* name);

	physx::PxU32			forceLoadAssets();

	const char* 			checkAssetName(NiApexAuthorableObject& ao, const char* inName, ApexSimpleString& autoNameStorage);
	NxApexAssetAuthoring*	createAssetAuthoring(const char* authorTypeName);
	NxApexAssetAuthoring*	createAssetAuthoring(const char* authorTypeName, const char* name);
	NxApexAssetAuthoring*	createAssetAuthoring(NxParameterized::Interface* params, const char* name);
	NxApexAsset*			createAsset(NxApexAssetAuthoring&, const char*);
	NxApexAsset* 			createAsset(NxParameterized::Interface* params, const char* name);
	virtual NxApexAsset*	createAsset(const char* opaqueMeshName, NxUserOpaqueMesh* om);
	void					releaseAsset(NxApexAsset& nxasset);
	void					releaseAssetAuthoring(NxApexAssetAuthoring&);
	/* NiApexSDK */
	void					reportError(PxErrorCode::Enum code, const char* file, int line, const char* functionName, const char* message, ...);
	void*					getTempMemory(PxU32 size);
	void					releaseTempMemory(void* data);
	ErrorHandler& 			getErrorHandler() const;
	NxParameterized::Traits* getParameterizedTraits()
	{
		return mParameterizedTraits;
	}
	physx::PxU32			getCookingVersion() const
	{
		return cookingVersion;
	}
	void					registerExternalModule(NxModule* nx, NiModule* ni)
	{
		registerModule(nx, ni);
	}
	int						getSuggestedCudaDeviceOrdinal();

	NxAuthObjTypeID			registerAuthObjType(const char*, NxResID nsid);
	NxAuthObjTypeID			registerAuthObjType(const char*, NiApexAuthorableObject* authObjPtr);
	NxAuthObjTypeID			registerNxParamAuthType(const char*, NiApexAuthorableObject* authObjPtr);
	void					unregisterAuthObjType(const char*);
	void					unregisterNxParamAuthType(const char*);
	NiApexAuthorableObject*	getAuthorableObject(const char*);
	NiApexAuthorableObject*	getNxParamAuthObject(const char*);
	bool					getAuthorableObjectNames(const char** authTypeNames, physx::PxU32& outCount, physx::PxU32 inCount);
	NxResID					getMaterialNameSpace() const
	{
		return mMaterialNS;
	}
	NxResID					getOpaqueMeshNameSpace() const
	{
		return mOpaqueMeshNS;
	}
	NxResID					getCustomVBNameSpace() const
	{
		return mCustomVBNS;
	}
	NxResID					getApexMeshNameSpace();
	NxResID					getCollisionGroupNameSpace() const
	{
		return mCollGroupNS;
	}
	NxResID					getCollisionGroup128NameSpace() const
	{
		return mCollGroup128NS;
	}
	NxResID					getCollisionGroup64NameSpace() const
	{
		return mCollGroup64NS;
	}
	NxResID					getCollisionGroupMaskNameSpace() const
	{
		return mCollGroupMaskNS;
	}
	NxResID					getPhysicalMaterialNameSpace() const
	{
		return mPhysMatNS;
	}
	NxResID					getAuthorableTypesNameSpace() const
	{
		return mObjTypeNS;
	}

	void					releaseObjectDesc(void*);
	NxUserRenderResourceManager* getUserRenderResourceManager() const
	{
		return renderResourceManagerWrapper ? renderResourceManagerWrapper : renderResourceManager;
	}

	const char* 			getWireframeMaterial();
	const char* 			getSolidShadedMaterial();
	virtual PVD::PvdBinding* getPvdBinding();
	virtual physx::PxProfileZone * getProfileZone();

	virtual void			acquirePhysXSdkLock();
	virtual void			releasePhysXSdkLock();

	virtual void setEnableApexStats(bool enableApexStats)
	{
		mEnableApexStats = enableApexStats;
	}

	virtual void setEnableConcurrencyCheck(bool enableConcurrencyCheck)
	{
		mEnableConcurrencyCheck = enableConcurrencyCheck;
	}

	virtual bool isConcurrencyCheckEnabled()
	{
		return mEnableConcurrencyCheck;
	}

	bool isApexStatsEnabled() const
	{
		return mEnableApexStats;
	}

#ifdef PX_WINDOWS
	virtual const char*		getAppGuid();
#endif

#if defined(APEX_CUDA_SUPPORT)
	virtual PhysXGpuIndicator* registerPhysXIndicatorGpuClient();
	virtual void unregisterPhysXIndicatorGpuClient(PhysXGpuIndicator* gpuIndicator);
#endif

	NxApexSDKCachedData&	getCachedData() const
	{
		return *mCachedData;
	}

	NxParameterized::Serializer* createSerializer(NxParameterized::Serializer::SerializeType type);
	NxParameterized::Serializer* createSerializer(NxParameterized::Serializer::SerializeType type, NxParameterized::Traits* traits);

	NxParameterized::Serializer::SerializeType getSerializeType(const void* data, PxU32 dlen);
	NxParameterized::Serializer::SerializeType getSerializeType(physx::PxFileBuf& stream);

	NxParameterized::Serializer::ErrorType getSerializePlatform(const void* data, PxU32 dlen, NxParameterized::SerializePlatform& platform);
	NxParameterized::Serializer::ErrorType getSerializePlatform(physx::PxFileBuf& stream, NxParameterized::SerializePlatform& platform);
	void getCurrentPlatform(NxParameterized::SerializePlatform& platform) const;
	bool getPlatformFromString(const char* name, NxParameterized::SerializePlatform& platform) const;
	const char* getPlatformName(const NxParameterized::SerializePlatform& platform) const;

	NxParameterized::Interface* 	getDebugColorParams() const
	{
		return mDebugColorParams;
	}
	void					updateDebugColorParams(const char* color, physx::PxU32 val);

	bool					getRMALoadMaterialsLazily();

	// applications can append strings to the APEX DLL filenames
	const char* getCustomDllNamePostfix() const
	{
		return mCustomDllNamePostfix.c_str();
	}

	virtual NiModule *getNiModule(NxModule *module);
	virtual NxModule *getNxModule(NiModule *module);

	virtual void enterURR();
	virtual void leaveURR();
	virtual void checkURR();

protected:
	virtual ~ApexSDK();
	void					registerModule(NxModule*, NiModule*);
	void					release();

private:

	void debugAsset(NxApexAsset* asset, const char* name);

	ApexSimpleString                mDllLoadPath;
	ApexSimpleString                mCustomDllNamePostfix;

	ApexSimpleString                mWireframeMaterial;
	ApexSimpleString                mSolidShadedMaterial;

#ifdef PX_WINDOWS
	ApexSimpleString                mAppGuid;
#endif

	NxResourceList* 				mAuthorableObjects;

	physx::Array<NxModule*>			modules;
	physx::Array<NxModule*>			moduleListForAPI;
	physx::Array<NiModule*>			imodules;
	physx::Array<NxApexScene*>		mScenes;

	NiApexPhysXObjectDesc* 			createObjectDesc(const NxApexActor*, const void*);
	NiApexPhysXObjectDesc* 			getGenericPhysXObjectInfo(const void*) const;

	enum { DescHashSize = 1024U * 32U };

	physx::PxU32					mBatchSeedSize;

	mutable physx::Mutex			mPhysXObjDescsLock;
#if NX_SDK_VERSION_MAJOR == 2
	mutable physx::Mutex			mPhysXSceneLock;
#endif
	physx::Array<ApexPhysXObjectDesc> mPhysXObjDescs;
	physx::PxU32					mPhysXObjDescHash[DescHashSize];
	physx::PxU32					mDescFreeList;
	NxResID                         mObjTypeNS;
	NxResID							mNxParamObjTypeNS;

	char*							mErrorString;
	//temp memories:
	struct TempMemory
	{
		TempMemory() : memory(NULL), size(0), used(0) {}
		void* memory;
		PxU32 size;
		PxU32 used;
	};
	physx::Array<TempMemory>		mTempMemories;
	physx::PxU32					mNumTempMemoriesActive;	//temp memories are a LIFO, mNumTempMemoriesActive <= mTempMemories.size()
	physx::Mutex					mTempMemoryLock;
	physx::Mutex					mReportErrorLock;

#if NX_SDK_VERSION_MAJOR == 2
	PxCpuDispatcher*			mApexThreadPool;
	physx::Array<PxCpuDispatcher*> mUserAllocThreadPools;
	NxPhysicsSDK*					physXSDK;
	NxCookingInterface*				cooking;
#else
	PxPhysics*	    				physXSDK;
	PxCooking*	     			    cooking;
#endif // NX_SDK_VERSION_MAJOR == 2

	bool							mOwnsFoundation;

	NxUserRenderResourceManager*	renderResourceManager;
	NxUserRenderResourceManager*	renderResourceManagerWrapper;
	ApexResourceProvider*			apexResourceProvider;
	physx::PxU32					physXsdkVersion;
	physx::PxU32					cookingVersion;

	ApexParameterizedTraits*		mParameterizedTraits;

	NxResID							mMaterialNS;
	NxResID							mOpaqueMeshNS;
	NxResID							mCustomVBNS;
	NxResID							mCollGroupNS;
	NxResID							mCollGroup128NS;
	NxResID							mCollGroup64NS;
	NxResID							mCollGroupMaskNS;
	NxResID							mPhysMatNS;

	ModuleFramework					frameworkModule;

	ApexSDKCachedData*				mCachedData;

	DebugColorParamsEx*				mDebugColorParams;

	bool							mRMALoadMaterialsLazily;

#ifdef PHYSX_PROFILE_SDK
	PVD::PvdBinding*				mPvdBinding;
	physx::PxProfileZone			*mProfileZone;
#endif

	PxU32							mURRdepthTLSslot;

	bool							mEnableApexStats;
	bool							mEnableConcurrencyCheck;
};


///////////////////////////////////////////////////////////////////////////////
// ApexRenderMeshAssetAuthoring
///////////////////////////////////////////////////////////////////////////////
// Didn't use ApexAuthorableObject<> here because there were enough differences
// from a "normal" asset to make it difficult.  I could probably bend the will
// of the APEX render mesh to get it to comply, but it wasn't worth it at the time.
// -LRR

class RenderMeshAuthorableObject : public NiApexAuthorableObject
{
public:
	RenderMeshAuthorableObject(NiModule* m, NxResourceList& list, const char* parameterizedName) :
		NiApexAuthorableObject(m, list, ApexRenderMeshAsset::getClassName())
	{
		// Register the proper authorable object type in the NRP (override)
		mAOResID = NiGetApexSDK()->getInternalResourceProvider()->createNameSpace(mAOTypeName.c_str());
		mAOPtrResID = NiGetApexSDK()->registerAuthObjType(mAOTypeName.c_str(), this);

		mParameterizedName = parameterizedName;

		// Register the parameterized name in the NRP to point to this authorable object
		NiGetApexSDK()->registerNxParamAuthType(mParameterizedName.c_str(), this);
	}

	NxApexAsset* 			createAsset(NxApexAssetAuthoring& author, const char* name);
	NxApexAsset* 			createAsset(NxParameterized::Interface* params, const char* name);

	void					releaseAsset(NxApexAsset& nxasset);

	NxApexAssetAuthoring* 	createAssetAuthoring();
	NxApexAssetAuthoring* 	createAssetAuthoring(const char* name);
	NxApexAssetAuthoring* 	createAssetAuthoring(NxParameterized::Interface* params, const char* name);
	void					releaseAssetAuthoring(NxApexAssetAuthoring& nxauthor);

	physx::PxU32					forceLoadAssets();
	virtual physx::PxU32			getAssetCount()
	{
		return mAssets.getSize();
	}
	virtual bool			getAssetList(NxApexAsset** outAssets, physx::PxU32& outAssetCount, physx::PxU32 inAssetCount)
	{
		PX_ASSERT(outAssets);
		PX_ASSERT(inAssetCount >= mAssets.getSize());

		if (!outAssets || inAssetCount < mAssets.getSize())
		{
			outAssetCount = 0;
			return false;
		}

		outAssetCount = mAssets.getSize();
		for (physx::PxU32 i = 0; i < mAssets.getSize(); i++)
		{
			ApexRenderMeshAsset* asset = static_cast<ApexRenderMeshAsset*>(mAssets.getResource(i));
			outAssets[i] = static_cast<NxApexAsset*>(asset);
		}

		return true;
	}


	NxResID					getResID()
	{
		return mAOResID;
	}

	ApexSimpleString&		getName()
	{
		return mAOTypeName;
	}

	// NxApexResource methods
	void					release();
	void					destroy();
};

}
} // end namespace physx::apex


#endif // APEX_SDK_H
