/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_SCENE_CACHED_DATA_H__

#define __APEX_SCENE_CACHED_DATA_H__

#include "PsShare.h"
#include "foundation/PxSimpleTypes.h"
#include "PxFileBuf.h"
#include "PsUserAllocated.h"
#include "PsArray.h"

#include "NxApexSDKCachedData.h"

namespace physx
{
namespace apex
{

class ApexScene;
class NiModuleScene;

/**
	Cached data is stored per-module.
*/
class NiApexModuleCachedData : public NxApexModuleCachedData
{
public:
	virtual NxAuthObjTypeID				getModuleID() const = 0;

	virtual NxParameterized::Interface*	getCachedDataForAssetAtScale(NxApexAsset& asset, const physx::PxVec3& scale) = 0;
	virtual physx::PxFileBuf&			serialize(physx::PxFileBuf& stream) const = 0;
	virtual physx::PxFileBuf&			deserialize(physx::PxFileBuf& stream) = 0;
	virtual void						clear(bool force = true) = 0;	// If force == false, data in use by actors will not be deleted
};

//**************************************************************************************************************************
//**************************************************************************************************************************
//**** APEX SCENE CACHED DATA
//**************************************************************************************************************************
//**************************************************************************************************************************

class ApexSDKCachedData : public NxApexSDKCachedData, public physx::UserAllocated
{
public:
	bool							registerModuleDataCache(NiApexModuleCachedData* cache);
	bool							unregisterModuleDataCache(NiApexModuleCachedData* cache);

	// NxApexSDKCachedData interface
	ApexSDKCachedData();
	virtual							~ApexSDKCachedData();

	virtual NxApexModuleCachedData*	getCacheForModule(NxAuthObjTypeID moduleID);
	virtual physx::PxFileBuf&		serialize(physx::PxFileBuf& stream) const;
	virtual physx::PxFileBuf&		deserialize(physx::PxFileBuf& stream);
	virtual void					clear(bool force = true);

	struct Version
	{
		enum Enum
		{
			First = 0,

			Count,
			Current = Count - 1
		};
	};

	// Data
	physx::Array<NiApexModuleCachedData*>	mModuleCaches;
};

}
} // end namespace physx::apex

#endif	// __APEX_SCENE_CACHED_DATA_H__
