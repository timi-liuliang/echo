/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_ASSET_TRACKER_H
#define APEX_ASSET_TRACKER_H

#include "ApexString.h"
#include "NiApexSDK.h"
#include "NiResourceProvider.h"
#include "PsArray.h"

namespace physx
{
namespace apex
{

class NiIosAsset;
class NxIosAsset;

/* There are a couple confusing details here:
 * 1. If you are tracking particle system or material assets, use an *authoringTypeName*
 *    of "".
 * 2. When loading, this class checks first if the *authoringTypeName* == "" [A], it will
 *    then check the nameIdList.assetTypeName == "" [B]
 *    If A && B,  use particle system namespace defined in B
 *    If A && !B, use material namespace
 *    If !A && B, use namespace specified in constructor
 */
class ApexAssetTracker
{
public:
	ApexAssetTracker() : mSdk(0) {}

	ApexAssetTracker(NiApexSDK*	sdk, const char* authoringTypeName)
		:	mAuthoringTypeName(authoringTypeName),
		    mSdk(sdk)
	{}

	ApexAssetTracker(NiApexSDK*	sdk)
		:	mAuthoringTypeName(""),
		    mSdk(sdk)
	{}

	~ApexAssetTracker();

	NxIosAsset* 	getIosAssetFromName(const char* iosTypeName, const char* assetName);
	NxApexAsset* 	getAssetFromName(const char* assetName);
	NxApexAsset* 	getMeshAssetFromName(const char* assetName, bool isOpaqueMesh);
	bool			addAssetName(const char* assetName, bool isOpaqueMesh);
	bool			addAssetName(const char* iosTypeName, const char* assetName);
	void			removeAllAssetNames();

	physx::PxU32			forceLoadAssets();

	/* one function must be implemented to fill in the name to id mappin lists */
	//virtual void	initializeAssetNameTable() = 0;

	physx::Array<AssetNameIDMapping*>&	getNameIdList()
	{
		return mNameIdList;
	}

	ApexSimpleString					mAuthoringTypeName;
	NiApexSDK*							mSdk;
	physx::Array<AssetNameIDMapping*>	mNameIdList;
};

}
} // end namespace physx::apex

#endif	// APEX_ASSET_TRACKER_H
