/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexSDKCachedData.h"
#include "NxParameterized.h"

namespace physx
{
namespace apex
{

bool ApexSDKCachedData::registerModuleDataCache(NiApexModuleCachedData* cache)
{
	if (cache == NULL)
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < mModuleCaches.size(); ++i)
	{
		if (cache == mModuleCaches[i])
		{
			return false;
		}
	}

	mModuleCaches.pushBack(cache);

	return true;
}

bool ApexSDKCachedData::unregisterModuleDataCache(NiApexModuleCachedData* cache)
{
	if (cache == NULL)
	{
		return false;
	}

	for (physx::PxU32 i = mModuleCaches.size(); i--;)
	{
		if (cache == mModuleCaches[i])
		{
			mModuleCaches.replaceWithLast(i);
			break;
		}
	}

	return false;
}

ApexSDKCachedData::ApexSDKCachedData()
{
}

ApexSDKCachedData::~ApexSDKCachedData()
{
}

NxApexModuleCachedData* ApexSDKCachedData::getCacheForModule(NxAuthObjTypeID moduleID)
{
	for (physx::PxU32 i = 0; i < mModuleCaches.size(); ++i)
	{
		if (moduleID == mModuleCaches[i]->getModuleID())
		{
			return mModuleCaches[i];
		}
	}

	return NULL;
}

physx::PxFileBuf& ApexSDKCachedData::serialize(physx::PxFileBuf& stream) const
{
	stream.storeDword((physx::PxU32)Version::Current);

	for (physx::PxU32 i = 0; i < mModuleCaches.size(); ++i)
	{
		mModuleCaches[i]->serialize(stream);
	}

	return stream;
}

physx::PxFileBuf& ApexSDKCachedData::deserialize(physx::PxFileBuf& stream)
{
	clear(false); // false => don't delete cached data for referenced sets

	/*const physx::PxU32 version =*/
	stream.readDword();	// Original version

	for (physx::PxU32 i = 0; i < mModuleCaches.size(); ++i)
	{
		mModuleCaches[i]->deserialize(stream);
	}

	return stream;
}

void ApexSDKCachedData::clear(bool force)
{
	for (physx::PxU32 i = mModuleCaches.size(); i--;)
	{
		mModuleCaches[i]->clear(force);
	}
}

}
} // end namespace physx::apex
