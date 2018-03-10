/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "ApexSDK.h"
#include "ProfilerCallback.h"
#include "PxTaskManager.h"
#include "PxCpuDispatcher.h"
#include "PxCudaContextManager.h"

#include "PsFoundation.h"
#include "foundation/PxErrorCallback.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "ThreadPool.h"
#include "NxUserAllocator.h"
#include "NxPhysicsSDK.h"
#include "NxFoundationSDK.h"

namespace physx
{
namespace apex
{

class ShimAllocator : public PxAllocatorCallback
{
public:
	ShimAllocator() : mInternal(NULL)
	{}

	ShimAllocator(NxUserAllocator& nxalloc)
	{
		init(nxalloc);
	}

	virtual ~ShimAllocator() {}

	void		init(NxUserAllocator& nxalloc)
	{
		mInternal = &nxalloc;

		char* test = (char*) mInternal->mallocDEBUG(8, __FILE__, __LINE__);
		if (test)
		{
			mUseDebug = true;
			mInternal->free(test);
		}
		else
		{
			mUseDebug = false;
		}
	}

	void*       allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		PX_ASSERT(mInternal);
		if (!mInternal)
		{
			return NULL;
		}

		PX_UNUSED(typeName);
		if (mUseDebug)
		{
			return mInternal->mallocDEBUG(size, filename, line);
		}
		else
		{
			return mInternal->malloc(size);
		}
	}

	void        deallocate(void* ptr)
	{
		PX_ASSERT(mInternal);
		if (!mInternal)
		{
			return;
		}

		mInternal->free(ptr);
	}

private:
	bool              mUseDebug;
	NxUserAllocator* mInternal;
};

ShimAllocator gStaticShimAllocatorInst;

}
} // end namespace physx::apex

#endif

namespace physx
{
namespace apex
{

ApexSDK* gApexSdk = NULL;	// For an SDK singleton

NXAPEX_API NxApexSDK*	NX_CALL_CONV NxGetApexSDK()
{
	return gApexSdk;
}
NXAPEX_API NiApexSDK*	NX_CALL_CONV NiGetApexSDK()
{
	return gApexSdk;
}

NxApexSDK* NxCreateApexSDK(const NxApexSDKDesc& desc, NxApexCreateError* errorCode, physx::PxU32 APEXsdkVersion)
{
	if (errorCode)
	{
		*errorCode = APEX_CE_NO_ERROR;
	}

	if (gApexSdk != NULL)
	{
		return gApexSdk;
	}

	if (APEXsdkVersion != NX_APEX_SDK_VERSION)
	{
		if (errorCode)
		{
			*errorCode = APEX_CE_WRONG_VERSION;
		}
		return NULL;
	}

	if (!desc.isValid())	//this checks for SDK and cooking version mismatch!
	{
		if (errorCode)
		{
			if (desc.physXSDKVersion != NX_PHYSICS_SDK_VERSION)
			{
				*errorCode = APEX_CE_WRONG_VERSION;
			}
			else
			{
				*errorCode = APEX_CE_DESCRIPTOR_INVALID;
			}
		}
		return NULL;
	}

	// If using PhysX3 APEX MUST share the foundation instance...
	// use setInstance(), if the foundation was created by something else it will be leaked (only a problem with DLL systems).
	bool ownsFoundation = false; 

#if NX_SDK_VERSION_MAJOR == 2
	PxAllocatorCallback* alloc;
	if (desc.allocator == NULL)
	{
		gStaticShimAllocatorInst.init(desc.physXSDK->getFoundationSDK().getAllocator());
		alloc = &gStaticShimAllocatorInst;
	}
	else
	{
		alloc = desc.allocator;
	}

	// TODO_FOUNDATION_28
	// maybe assume here that no one else is going to create a foundation...
	// I cannot find a way to test if the foundation exists
	ownsFoundation = true;
	PxCreateFoundation(PX_PHYSICS_VERSION, *alloc, *desc.outputStream);
	//ownsFoundation = physx::Foundation::createInstance(PX_PHYSICS_VERSION, *desc.outputStream, *alloc);

#elif NX_SDK_VERSION_MAJOR == 3

	// setInstance is no longer a foundation member, PxGetFoundation should be used
	// physx::Foundation::setInstance(static_cast<physx::shdfnd::Foundation*>(&desc.physXSDK->getFoundation()));

#endif


	gApexSdk = PX_NEW(ApexSDK)(errorCode, APEXsdkVersion);
	gApexSdk->init(desc, ownsFoundation);

	return gApexSdk;
}


#if NX_SDK_VERSION_MAJOR == 2

/* We route allocations of these objects through the APEX SDK because PXTASK objects
 * require a foundation instance.
 */

physx::PxCpuDispatcher* ApexSDK::getDefaultThreadPool()
{
	if (!mApexThreadPool)
	{
		mApexThreadPool = createDefaultThreadPool(0);
	}

	return mApexThreadPool;
}

physx::PxCpuDispatcher* ApexSDK::createCpuDispatcher(physx::PxU32 numThreads)
{
	PxCpuDispatcher* cd = createDefaultThreadPool(numThreads);
	mUserAllocThreadPools.pushBack(cd);
	return cd;
}

void ApexSDK::releaseCpuDispatcher(physx::PxCpuDispatcher& cd)
{
	if (&cd == mApexThreadPool)
	{
		PX_DELETE(mApexThreadPool);
		mApexThreadPool = 0;
		return;
	}
	for (PxU32 i = 0; i < mUserAllocThreadPools.size(); i++)
	{
		if (&cd == mUserAllocThreadPools[i])
		{
			PX_DELETE(&cd);
			mUserAllocThreadPools.replaceWithLast(i);
			return;
		}
	}
}

physx::PxCudaContextManager* ApexSDK::createCudaContextManager(const physx::PxCudaContextManagerDesc& desc)
{
	PX_ASSERT(getErrorCallback());
	physx::PxProfileZoneManager* profileZoneManager = NULL;

#ifdef PHYSX_PROFILE_SDK
	profileZoneManager = &mPvdBinding->getProfileManager();
#endif
#if defined(PX_WINDOWS) && !defined(PX_WINMODERN)
	return physx::PxCreateCudaContextManager(PxGetFoundation(), desc, profileZoneManager);
#else
	return NULL;
#endif
}

physx::PxFoundation* ApexSDK::getPxFoundation() const
{
	return &PxGetFoundation();
}

#endif

}
} // end namespace physx::apex
