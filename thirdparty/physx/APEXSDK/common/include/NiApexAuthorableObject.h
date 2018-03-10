/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NI_APEX_AUTHORABLE_OBJECT_H
#define NI_APEX_AUTHORABLE_OBJECT_H

#include "ApexString.h"
#include "NiApexSDK.h"
#include "ApexSDKHelpers.h"
#include "NiResourceProvider.h"
#include "ApexInterface.h"

class NxResourceList;

namespace NxParameterized
{
class Interface;
};

namespace physx
{
namespace apex
{

// This class currently contains implementation, this will be removed and put in APEXAuthorableObject
class NiApexAuthorableObject : public NxApexResource, public ApexResource
{
public:

	NiApexAuthorableObject(NiModule* m, NxResourceList& list, const char* aoTypeName)
		:	mAOTypeName(aoTypeName),
		    mModule(m)
	{
		list.add(*this);
	}

	virtual NxApexAsset* 			createAsset(NxApexAssetAuthoring& author, const char* name) = 0;
	virtual NxApexAsset* 			createAsset(NxParameterized::Interface* params, const char* name) = 0;
	virtual void					releaseAsset(NxApexAsset& nxasset) = 0;

	virtual NxApexAssetAuthoring* 	createAssetAuthoring() = 0;
	virtual NxApexAssetAuthoring* 	createAssetAuthoring(const char* name) = 0;
	virtual NxApexAssetAuthoring* 	createAssetAuthoring(NxParameterized::Interface* params, const char* name) = 0;
	virtual void					releaseAssetAuthoring(NxApexAssetAuthoring& nxauthor) = 0;

	virtual physx::PxU32					forceLoadAssets() = 0;
	virtual physx::PxU32					getAssetCount() = 0;
	virtual bool					getAssetList(NxApexAsset** outAssets, physx::PxU32& outAssetCount, physx::PxU32 inAssetCount) = 0;


	virtual NxResID					getResID() = 0;
	virtual ApexSimpleString&		getName() = 0;

	// NxApexResource methods
	virtual void					release() = 0;
	virtual void					destroy() = 0;

	// NxApexResource methods
	physx::PxU32							getListIndex() const
	{
		return m_listIndex;
	}

	void							setListIndex(NxResourceList& list, physx::PxU32 index)
	{
		m_listIndex = index;
		m_list = &list;
	}

	NxResID				mAOResID;
	NxResID				mAOPtrResID;
	ApexSimpleString	mAOTypeName;
	ApexSimpleString	mParameterizedName;

	NxResourceList		mAssets;
	NxResourceList		mAssetAuthors;

	NiModule* 			mModule;
};

}
} // end namespace physx::apex

#endif	// NI_APEX_AUTHORABLE_OBJECT_H
