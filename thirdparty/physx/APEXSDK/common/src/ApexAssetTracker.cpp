/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexAssetTracker.h"
#include "NxApex.h"
#include "NiApexSDK.h"
#include "ApexInterface.h"
#include "ApexSDKHelpers.h"
#include "NiApexAuthorableObject.h"

namespace physx
{
namespace apex
{

/******************************************************************************
 * ApexAssetTracker class
 * Intended to be a base class for asset classes that have named sub-assets.
 * - Allows their actors to easily get asset pointers
 * - Uses the NRP in an appropriate fashion
 *   - calls to checkResource(), createResource(), and getResource()
 *   - handles that tricky IOS asset double call mechanism
 * - Sub class must implement initializeAssetNameTable()
 *
 */
ApexAssetTracker::~ApexAssetTracker()
{
	/* Get the NRP */
	if (mSdk)
	{
		NiResourceProvider* nrp = mSdk->getInternalResourceProvider();

		/* release references to rendermesh assets */
		for (physx::PxU32 j = 0 ; j < mNameIdList.size() ; j++)
		{
			AssetNameIDMapping *nameId = mNameIdList[j];
			if (nameId->resID != INVALID_RESOURCE_ID)
			{
				if ( nameId->isOpaqueMesh )
				{
					NxResID opaqueMeshId = nrp->createResource(mSdk->getOpaqueMeshNameSpace(),nameId->assetName.c_str(),false);
					PX_ASSERT( opaqueMeshId != INVALID_RESOURCE_ID );
					if (nrp->checkResource(opaqueMeshId))
					{
						nrp->releaseResource(opaqueMeshId);
					}
					PxU32 refCount;
					bool found =nrp->findRefCount(NX_RENDER_MESH_AUTHORING_TYPE_NAME,nameId->assetName.c_str(),refCount);
					PX_ASSERT(found);
					PX_UNUSED(found);
					if (nrp->checkResource(nameId->resID))
					{
						if ( refCount == 1 )
						{
							void *asset = nrp->getResource(nameId->resID);
							PX_ASSERT(asset);
							if ( asset )
							{
								NxApexAsset *apexAsset = (NxApexAsset *)asset;
								apexAsset->release();
							}
						}
						nrp->releaseResource(nameId->resID);
					}
				}
				else
				{
					if (nrp->checkResource(nameId->resID))
					{
						nrp->releaseResource(nameId->resID);
					}
				}
			}
			delete nameId;
		}
	}
}

bool ApexAssetTracker::addAssetName(const char* assetName, bool isOpaqueMesh)
{
	/* first see if the name is already here */
	for (physx::PxU32 i = 0; i < mNameIdList.size(); i++)
	{
		if (mNameIdList[i]->assetName == assetName && mNameIdList[i]->isOpaqueMesh == isOpaqueMesh)
		{
			return false;
		}
	}

	/* now add it to the list */
	mNameIdList.pushBack(PX_NEW(AssetNameIDMapping)(assetName, isOpaqueMesh));

	return true;
}

bool ApexAssetTracker::addAssetName(const char* iosTypeName, const char* assetName)
{
	/* first see if the name is already here */
	for (physx::PxU32 i = 0; i < mNameIdList.size(); i++)
	{
		if (mNameIdList[i]->assetName == assetName &&
		        mNameIdList[i]->iosAssetTypeName == iosTypeName)
		{
			return false;
		}
	}

	/* now add it to the list */
	mNameIdList.pushBack(PX_NEW(AssetNameIDMapping)(assetName, iosTypeName));

	return true;
}

void ApexAssetTracker::removeAllAssetNames()
{
	/* Get the NRP */
	NiResourceProvider* nrp = mSdk->getInternalResourceProvider();

	/* release references to rendermesh assets */
	for (physx::PxU32 j = 0 ; j < mNameIdList.size() ; j++)
	{
		nrp->releaseResource(mNameIdList[j]->resID);
		delete mNameIdList[j];
	}
	mNameIdList.reset();
}

NxIosAsset* ApexAssetTracker::getIosAssetFromName(const char* iosTypeName, const char* assetName)
{
	/* This will actually call the NRP to force the asset to be loaded (if it isn't already loaded)
	 * loading the APS will cause the particle module to call setResource on the iosans
	 */
	void* asset = ApexAssetHelper::getAssetFromNameList(mSdk,
	              iosTypeName,
	              mNameIdList,
	              assetName);

	NxApexAsset* aa = static_cast<NxApexAsset*>(asset);
	return DYNAMIC_CAST(NxIosAsset*)(aa);
}

NxApexAsset* ApexAssetTracker::getAssetFromName(const char* assetName)
{
	/* handle the material namespace, which is different (not authorable) */
	NxResID resID = INVALID_RESOURCE_ID;
	if (mAuthoringTypeName == "")
	{
		resID = mSdk->getMaterialNameSpace();
	}

	void* tmp = ApexAssetHelper::getAssetFromNameList(mSdk,
	            mAuthoringTypeName.c_str(),
	            mNameIdList,
	            assetName,
	            resID);

	return static_cast<NxApexAsset*>(tmp);
}

NxApexAsset* ApexAssetTracker::getMeshAssetFromName(const char* assetName, bool isOpaqueMesh)
{
	PX_UNUSED(isOpaqueMesh);
	/* handle the material namespace, which is different (not authorable) */
	NxResID resID = INVALID_RESOURCE_ID;
	if (isOpaqueMesh)
	{
		resID = mSdk->getOpaqueMeshNameSpace();
	}
	else if (mAuthoringTypeName == "")
	{
		resID = mSdk->getMaterialNameSpace();
	}

	void* tmp = ApexAssetHelper::getAssetFromNameList(mSdk,
	            mAuthoringTypeName.c_str(),
	            mNameIdList,
	            assetName,
	            resID);

	return static_cast<NxApexAsset*>(tmp);
}


physx::PxU32 ApexAssetTracker::forceLoadAssets()
{
	if (mNameIdList.size() == 0)
	{
		return 0;
	}

	physx::PxU32 assetLoadedCount = 0;

	/* handle the material namespace, which is different (not authorable) */
	NxResID assetNsID = INVALID_RESOURCE_ID;
	if (mAuthoringTypeName == "")
	{
		assetNsID = mSdk->getMaterialNameSpace();
	}
	else
	{
		NiApexAuthorableObject* ao = mSdk->getAuthorableObject(mAuthoringTypeName.c_str());
		if (ao)
		{
			assetNsID = ao->getResID();
		}
		else
		{
			APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", mAuthoringTypeName.c_str());
		}
	}

	for (physx::PxU32 i = 0 ; i < mNameIdList.size() ; i++)
	{
		bool useIosNamespace = false;
		/* Check if we are using the special IOS namespace ID */
		if (!(mNameIdList[i]->iosAssetTypeName == ""))
		{
			NiApexAuthorableObject* ao = mSdk->getAuthorableObject(mNameIdList[i]->iosAssetTypeName.c_str());
			if (!ao)
			{
				APEX_INTERNAL_ERROR("Particles Module is not loaded, cannot create particle system asset.");
				return 0;
			}

			assetNsID = ao->getResID();

			useIosNamespace = true;
		}

		// check if the asset has loaded yet
		if (mNameIdList[i]->resID == INVALID_RESOURCE_ID)
		{
			// if not, go ahead and ask the NRP for them
			if (useIosNamespace)
			{
				getIosAssetFromName(mNameIdList[i]->iosAssetTypeName.c_str(), mNameIdList[i]->assetName.c_str());
			}
			else if (mNameIdList[i]->isOpaqueMesh)
			{
				getMeshAssetFromName(mNameIdList[i]->assetName.c_str(), true);
			}
			else
			{
				getAssetFromName(mNameIdList[i]->assetName.c_str());
			}
			assetLoadedCount++;
		}
	}

	return assetLoadedCount;
}

}
} // end namespace physx::apex

