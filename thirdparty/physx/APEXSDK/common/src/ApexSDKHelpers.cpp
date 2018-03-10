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
#include "NiApexSDK.h"
#include "ApexInterface.h"
#include "ApexSDKHelpers.h"
#include "NiApexAuthorableObject.h"

#ifndef WITHOUT_PVD
#include "PVDBinding.h"
#include "PvdDataStream.h"
#endif

namespace physx
{
namespace apex
{

/*
	NxResourceList functions
 */
NxResourceList::~NxResourceList()
{
	clear();
}

void NxResourceList::clear()
{
	ScopedWriteLock scopedLock(mRWLock);

	physx::PxU32 s = mArray.size();
	while (s--)
	{
		mArray.back()->release();
		if (mArray.size() != s)
		{
			PX_ASSERT(!"Error - resource did not remove itself from list upon release\n");
			if (mArray.size())
			{
				mArray.popBack();	// Force removal
			}
		}
	}
}

#ifndef WITHOUT_PVD
void NxResourceList::setupForPvd(const void* owner, const char* listName, const char* entryName)
{
	mOwner = owner;
	mListName = listName;
	mEntryName = entryName;
}


void NxResourceList::initPvdInstances(physx::debugger::comm::PvdDataStream& pvdStream)
{
	ScopedWriteLock scopedLock(mRWLock);

	for (PxU32 i = 0; i < mArray.size(); ++i)
	{
		const void* entry = (const void*)mArray[i];
		pvdStream.createInstance(physx::debugger::NamespacedName(APEX_PVD_NAMESPACE, mEntryName.c_str()), entry);
		pvdStream.pushBackObjectRef(mOwner, mListName.c_str(), entry);
		mArray[i]->initPvdInstances(pvdStream);
	}
}
#endif

void NxResourceList::add(NxApexResource& resource)
{
	ScopedWriteLock scopedLock(mRWLock);

	if (resource.getListIndex() != 0xFFFFFFFF)
	{
		PX_ASSERT(!"Error - attempting to add a resource to a list twice");
		return;
	}
	resource.setListIndex(*this, mArray.size());
	mArray.pushBack(&resource);

#ifndef WITHOUT_PVD
	// add to pvd
	if (mOwner != NULL)
	{
		PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
		if (pvdBinding != NULL)
		{
			if (pvdBinding->getConnectionType() & physx::debugger::PvdConnectionType::eDEBUG)
			{
				pvdBinding->lock();
				physx::debugger::comm::PvdDataStream* pvdStream = pvdBinding->getDataStream();
				{
					if (pvdStream != NULL)
					{
						pvdStream->createInstance(physx::debugger::NamespacedName(APEX_PVD_NAMESPACE, mEntryName.c_str()), &resource);
						pvdStream->pushBackObjectRef(mOwner, mListName.c_str(), &resource);
						resource.initPvdInstances(*pvdStream);
					}
				}
				pvdBinding->unlock();
			}
		}
	}
#endif
}

void NxResourceList::remove(physx::PxU32 index)
{
	ScopedWriteLock scopedLock(mRWLock);

	PX_ASSERT(index < mArray.size());

#ifndef WITHOUT_PVD
	// remove from pvd
	if (mOwner != NULL)
	{
		PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
		if (pvdBinding != NULL)
		{
			if (pvdBinding->getConnectionType() & physx::debugger::PvdConnectionType::eDEBUG)
			{
				pvdBinding->lock();
				physx::debugger::comm::PvdDataStream* pvdStream = pvdBinding->getDataStream();
				{
					if (pvdStream != NULL)
					{
						// would be nice to be able to call resource->destroyPvdInstances() here,
						// but the resource has already been destroyed, so it's too late here
						NxApexResource* resource = mArray[index];
						pvdStream->removeObjectRef(mOwner, mListName.c_str(), resource);
						pvdStream->destroyInstance(resource);
					}
				}
				pvdBinding->unlock();
			}
		}
	}
#endif

	mArray.replaceWithLast(index);
	if (index < mArray.size())
	{
		mArray[index]->setListIndex(*this, index);
	}
}

#if 0
// these are poison
void writeStreamHeader(physx::PxFileBuf& stream, ApexSimpleString& streamName, physx::PxU32 versionStamp)
{
	physx::PxU32 streamStamp = GetStamp(streamName);

	stream.storeDword(versionStamp);
	stream.storeDword(streamStamp);
}

physx::PxU32 readStreamHeader(const physx::PxFileBuf& stream, ApexSimpleString& streamName)
{
	physx::PxU32 version = stream.readDword();

	physx::PxU32 streamStamp = stream.readDword();
	if (streamStamp != GetStamp(streamName))
	{
		APEX_INVALID_PARAMETER("Wrong input stream. The provided stream has to contain %s.", streamName.c_str());
		return (physx::PxU32) - 1;
	}

	return version;
}
#endif

/******************************************************************************
 * Helper function for loading assets
 *
 * This method's purpose is to generalize this process:
 *
 * 1. Get the module's namespace resource ID (this also checks that the module is loaded)
 * 2. If the asset has not been created yet, it will call createResource()
 * 3. It will call getResource() and return the result
 *
 * This allows both the asset's forceLoad method AND the actors init methods to get
 * an asset pointer.
 *
 * This also allows the forceLoad method to call this method repeatedly until getResource()
 * returns a valid pointer (for async loading).

 *****************************************************************************/
void* ApexAssetHelper::getAssetFromName(NiApexSDK*	sdk,
                                        const char*	authoringTypeName,
                                        const char*	assetName,
                                        NxResID&		inOutResID,
                                        NxResID		optionalNsID)
{
	/* Get the NRP */
	NiResourceProvider* nrp = sdk->getInternalResourceProvider();

	/* Get the asset namespace ID */
	NxResID typeNsID = INVALID_RESOURCE_ID;
	if (optionalNsID == INVALID_RESOURCE_ID)
	{
		NiApexAuthorableObject* ao = sdk->getAuthorableObject(authoringTypeName);
		if (ao)
		{
			typeNsID = ao->getResID();
		}
		else
		{
			APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", authoringTypeName);
			return NULL;
		}
	}
	else
	{
		typeNsID = optionalNsID;
	}

	if (inOutResID == INVALID_RESOURCE_ID)
	{
		if (optionalNsID == sdk->getOpaqueMeshNameSpace())
		{
			NiApexAuthorableObject* ao = sdk->getAuthorableObject(NX_RENDER_MESH_AUTHORING_TYPE_NAME);
			if (ao)
			{
				typeNsID = ao->getResID();
			}

			NxResID opaqueMesh = nrp->createResource(optionalNsID, assetName);
			inOutResID = nrp->createResource(typeNsID, assetName);
			if (!nrp->checkResource(inOutResID))
			{
				NxUserOpaqueMesh* om = (NxUserOpaqueMesh*)nrp->getResource(opaqueMesh);
				NxApexAsset* asset = sdk->createAsset(assetName, om);
				nrp->setResource(authoringTypeName, assetName, asset, true, false);
			}
		}
		else
		{
			inOutResID = nrp->createResource(typeNsID, assetName);
		}
	}

	// If resID is valid, get the resource
	if (inOutResID != INVALID_RESOURCE_ID)
	{
		return nrp->getResource(inOutResID);
	}
	else
	{
		APEX_DEBUG_INFO("ApexAssetHelper::getAssetFromName: Could not create resource ID asset: %s", assetName);
		return NULL;
	}

}



/* getAssetFromNameList
 *
 * This method's purpose is to generalize this process:
 *
 * 1. Find the asset name in this asset type's name list
 * 2. Call the SDK's helper method, getAssetFromName
 *
 * This allows both the asset's forceLoad method AND the actors init methods to get
 * an asset pointer.
 *
 * This also allows the forceLoad method to call this method repeatedly until getResource()
 * returns a valid pointer (for async loading).
 */
void* ApexAssetHelper::getAssetFromNameList(NiApexSDK*	sdk,
        const char* authoringTypeName,
        physx::Array<AssetNameIDMapping*>& nameIdList,
        const char* assetName,
        NxResID assetNsId)
{
	// find the index of the asset name in our list of name->resID maps
	physx::PxU32 assetIdx = 0;
	for (assetIdx = 0; assetIdx < nameIdList.size(); assetIdx++)
	{
		if (nameIdList[assetIdx]->assetName == assetName)
		{
			break;
		}
	}

	// This can't ever happen
	PX_ASSERT(assetIdx < nameIdList.size());

	if (assetIdx < nameIdList.size())
	{
		return ApexAssetHelper::getAssetFromName(sdk,
		        authoringTypeName,
		        assetName,
		        nameIdList[assetIdx]->resID,
		        assetNsId);
	}
	else
	{
		APEX_DEBUG_WARNING("Request for asset %s of type %s not registered in asset tracker's list", assetName, authoringTypeName);
		return NULL;
	}
}

}
} // end namespace physx::apex
