/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_RESOURCE_HELPER_H__
#define __APEX_RESOURCE_HELPER_H__

#include "NxApex.h"
#include "NiResourceProvider.h"

#if NX_SDK_VERSION_MAJOR == 3
#include <PxFiltering.h>
#endif

namespace physx
{
namespace apex
{

class ApexResourceHelper
{
	ApexResourceHelper() {}
public:

#if NX_SDK_VERSION_MAJOR == 2
	static PX_INLINE NxCollisionGroup resolveCollisionGroup(const char* collisionGroupName)
	{
		NxCollisionGroup group = 0xFFFF;

		if (collisionGroupName)
		{
			NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
			NxResID collisionGroupNS = NiGetApexSDK()->getCollisionGroupNameSpace();
			NxResID id = nrp->createResource(collisionGroupNS, collisionGroupName);
			physx::PxU16 collisionGroup = (physx::PxU16)(size_t)(nrp->getResource(id));

			if (collisionGroup < 32)
			{
				group = collisionGroup;
			}
			else
			{
				APEX_INVALID_PARAMETER("APEX CollisionGroup Resource: '%s' = %d. Must be less than 32.", collisionGroupName, collisionGroup);
			}
		}
		return group;
	}

	static PX_INLINE NxGroupsMask resolveCollisionGroup128(const char* collisionGroup128Name)
	{
		NxGroupsMask result;
		//default constructor is empty, so set default value to 0 here
		result.bits0 = result.bits1 = result.bits2 = result.bits3 = 0;

		if (collisionGroup128Name)
		{
			/* create namespace for Collision Group (if it has not already been created) */
			NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
			NxResID collisionGroup128NS = NiGetApexSDK()->getCollisionGroup128NameSpace();
			NxResID id = nrp->createResource(collisionGroup128NS, collisionGroup128Name);
			const physx::PxU32* resourcePtr = static_cast<const physx::PxU32*>(nrp->getResource(id));
			if (resourcePtr)
			{
				result.bits0 = resourcePtr[0];
				result.bits1 = resourcePtr[1];
				result.bits2 = resourcePtr[2];
				result.bits3 = resourcePtr[3];
			}
		}
		return result;
	}
#elif NX_SDK_VERSION_MAJOR == 3
	static PX_INLINE PxFilterData resolveCollisionGroup128(const char* collisionGroup128Name)
	{
		PxFilterData result; //default constructor sets all words to 0

		if (collisionGroup128Name)
		{
			/* create namespace for Collision Group (if it has not already been created) */
			NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
			NxResID collisionGroup128NS = NiGetApexSDK()->getCollisionGroup128NameSpace();
			NxResID id = nrp->createResource(collisionGroup128NS, collisionGroup128Name);
			const physx::PxU32* resourcePtr = static_cast<const physx::PxU32*>(nrp->getResource(id));
			if (resourcePtr)
			{
				result.word0 = resourcePtr[0];
				result.word1 = resourcePtr[1];
				result.word2 = resourcePtr[2];
				result.word3 = resourcePtr[3];
			}
		}
		return result;
	}
#endif

	static PX_INLINE NxGroupsMask64 resolveCollisionGroup64(const char* collisionGroup64Name)
	{
		NxGroupsMask64 result(0, 0);

		if (collisionGroup64Name)
		{
			/* create namespace for Collision Group (if it has not already been created) */
			NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
			NxResID collisionGroup64NS = NiGetApexSDK()->getCollisionGroup64NameSpace();

			NxResID id = nrp->createResource(collisionGroup64NS, collisionGroup64Name);
			const physx::PxU32* resourcePtr = static_cast<const physx::PxU32*>(nrp->getResource(id));
			if (resourcePtr)
			{
				result.bits0 = resourcePtr[0];
				result.bits1 = resourcePtr[1];
			}
		}
		return result;
	}

	static PX_INLINE physx::PxU32 resolveCollisionGroupMask(const char* collisionGroupMaskName, physx::PxU32 defGroupMask = 0xFFFFFFFFu)
	{
		physx::PxU32 groupMask = defGroupMask;
		if (collisionGroupMaskName)
		{
			NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
			NxResID collisionGroupMaskNS = NiGetApexSDK()->getCollisionGroupMaskNameSpace();
			NxResID id = nrp->createResource(collisionGroupMaskNS, collisionGroupMaskName);
			groupMask = (physx::PxU32)(size_t)(nrp->getResource(id));
		}
		return groupMask;
	}
};

}
} // end namespace physx::apex

#endif	// __APEX_RESOURCE_HELPER_H__
