/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEXAUTHORABLEOBJECT_H__
#define __APEXAUTHORABLEOBJECT_H__

#include "NxApexAsset.h"

#include "NiApexAuthorableObject.h"
#include "NxParameterizedTraits.h"
#include "NxParameterized.h"

namespace physx
{
namespace apex
{

class NxResourceList;

/**
 *	ApexAuthorableObject
 *	This class is the implementation for NiApexAuthorableObject (except for the
 *	ApexResource stuff).  It's disappointing that it has to be templated like
 *	this, but there were issues with multiple inheritance (an NxApexResource ptr
 *	cannot be cast to an NxApexAsset ptr - NxApexAsset should inherit from NxApexResource
 *	in the future.
 *
 *	Template expectations:
 *	T_Module -	must inherit from NiModule
 *				the T_Asset type typically uses T_Module->mSdk
 *
 *	T_Asset -	T_Asset( T_Module *m, NxResourceList &list, const char *name )
 *				must inherit from NxApexAsset
 *
 *	T_AssetAuthoring -	T_AssetAuthoring( T_Module *m, NxResourceList &list )
 *						must inherit from NxApexAssetAuthoring
 */

template <class T_Module, class T_Asset, class T_AssetAuthoring>
class ApexAuthorableObject : public NiApexAuthorableObject
{
public:
	ApexAuthorableObject(NiModule* m, NxResourceList& list)
		:	NiApexAuthorableObject(m, list, T_Asset::getClassName())
	{
		// Register the authorable object type name in the NRP
		mAOResID = NiGetApexSDK()->getInternalResourceProvider()->createNameSpace(mAOTypeName.c_str());
		mAOPtrResID = NiGetApexSDK()->registerAuthObjType(mAOTypeName.c_str(), this);

		PX_ASSERT(!"This constructor is no longer valid, you MUST provide a parameterizedName!");
	}

	// This constructor is for assets that are based on NxParameterized, they provide the string
	// defined in the NxParameterized structure.  This will be used to map the NxParameterized object
	// to the AuthorableObject class to create the assets after they are deserialized
	ApexAuthorableObject(NiModule* m, NxResourceList& list, const char* parameterizedName)
		:	NiApexAuthorableObject(m, list, T_Asset::getClassName())
	{
		mParameterizedName = parameterizedName;

		// Register the authorable object type name in the NRP
		mAOResID = NiGetApexSDK()->getInternalResourceProvider()->createNameSpace(mAOTypeName.c_str());
		mAOPtrResID = NiGetApexSDK()->registerAuthObjType(mAOTypeName.c_str(), this);

		// Register the parameterized name in the NRP to point to this authorable object
		NiGetApexSDK()->registerNxParamAuthType(mParameterizedName.c_str(), this);
	}

	virtual NxApexAsset* 			createAsset(NxApexAssetAuthoring& author, const char* name);
	virtual NxApexAsset* 			createAsset(NxParameterized::Interface* params, const char* name);
	virtual void					releaseAsset(NxApexAsset& nxasset);

	virtual NxApexAssetAuthoring* 	createAssetAuthoring();
	virtual NxApexAssetAuthoring* 	createAssetAuthoring(const char* name);
	virtual NxApexAssetAuthoring* 	createAssetAuthoring(NxParameterized::Interface* params, const char* name);
	virtual void					releaseAssetAuthoring(NxApexAssetAuthoring& nxauthor);

	virtual physx::PxU32					forceLoadAssets();

	virtual physx::PxU32					getAssetCount()
	{
		return mAssets.getSize();
	}
	virtual bool					getAssetList(NxApexAsset** outAssets, physx::PxU32& outAssetCount, physx::PxU32 inAssetCount);

	virtual NxResID					getResID()
	{
		return mAOResID;
	}

	virtual ApexSimpleString&		getName()
	{
		return mAOTypeName;
	}

	// NxApexResource methods
	virtual void					release();

	virtual void					destroy()
	{
		delete this;
	}

};

template <class T_Module, class T_Asset, class T_AssetAuthoring>
NxApexAsset* ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
createAsset(NxApexAssetAuthoring& author, const char* name)
{
	if (mParameterizedName.len())
	{
		NxParameterized::Interface* params = 0;
		NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
		params = traits->createNxParameterized(mParameterizedName.c_str());
		PX_ASSERT(params);
		if (params)
		{
			NxParameterized::Interface* authorParams = author.getNxParameterized();
			PX_ASSERT(authorParams);
			if (authorParams)
			{
				if (NxParameterized::ERROR_NONE != authorParams->callPreSerializeCallback())
				{
					return NULL;
				}

				NxParameterized::ErrorType err = params->copy(*authorParams);

				PX_ASSERT(err == NxParameterized::ERROR_NONE);

				if (err == NxParameterized::ERROR_NONE)
				{
					return createAsset(params, name);
				}
				else
				{
					return NULL;
				}
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		APEX_INVALID_OPERATION("Authorable Asset needs a parameterized name");
		return NULL;
	}
}

template <class T_Module, class T_Asset, class T_AssetAuthoring>
void ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
release()
{
	// test this by releasing the module before the individual assets

	// remove all assets that we loaded (must do now else we cannot unregister)
	mAssets.clear();
	mAssetAuthors.clear();

	// remove this AO's name from the authorable namespace
	NiGetApexSDK()->unregisterAuthObjType(mAOTypeName.c_str());

	if (mParameterizedName.len())
	{
		NiGetApexSDK()->unregisterNxParamAuthType(mParameterizedName.c_str());
	}
	destroy();
}

template <class T_Module, class T_Asset, class T_AssetAuthoring>
NxApexAsset* ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
createAsset(NxParameterized::Interface* params, const char* name)
{
	T_Asset* asset = PX_NEW(T_Asset)(DYNAMIC_CAST(T_Module*)(mModule), mAssets, params, name);
	if (asset)
	{
		NiGetApexSDK()->getNamedResourceProvider()->setResource(mAOTypeName.c_str(), name, asset);
	}
	return asset;
}

template <class T_Module, class T_Asset, class T_AssetAuthoring>
void ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
releaseAsset(NxApexAsset& nxasset)
{
	T_Asset* asset = DYNAMIC_CAST(T_Asset*)(&nxasset);

	NiGetApexSDK()->getInternalResourceProvider()->setResource(mAOTypeName.c_str(), nxasset.getName(), NULL, false, false);
	asset->destroy();
}

#ifdef WITHOUT_APEX_AUTHORING

// this should no longer be called now that we're auto-assigning names in createAssetAuthoring()
template <class T_Module, class T_Asset, class T_AssetAuthoring>
NxApexAssetAuthoring* ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
createAssetAuthoring()
{
	APEX_INVALID_OPERATION("Asset authoring has been disabled");
	return NULL;
}

template <class T_Module, class T_Asset, class T_AssetAuthoring>
NxApexAssetAuthoring* ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
createAssetAuthoring(const char*)
{
	APEX_INVALID_OPERATION("Asset authoring has been disabled");
	return NULL;
}

template <class T_Module, class T_Asset, class T_AssetAuthoring>
NxApexAssetAuthoring* ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
createAssetAuthoring(NxParameterized::Interface*, const char*)
{
	APEX_INVALID_OPERATION("Asset authoring has been disabled");
	return NULL;
}

#else // WITHOUT_APEX_AUTHORING

// this should no longer be called now that we're auto-assigning names in createAssetAuthoring()
template <class T_Module, class T_Asset, class T_AssetAuthoring>
NxApexAssetAuthoring* ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
createAssetAuthoring()
{
	return PX_NEW(T_AssetAuthoring)(DYNAMIC_CAST(T_Module*)(mModule), mAssetAuthors);
}

template <class T_Module, class T_Asset, class T_AssetAuthoring>
NxApexAssetAuthoring* ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
createAssetAuthoring(const char* name)
{
	T_AssetAuthoring* assetAuthor = PX_NEW(T_AssetAuthoring)(DYNAMIC_CAST(T_Module*)(mModule), mAssetAuthors, name);

	if (assetAuthor)
	{
		NiGetApexSDK()->getNamedResourceProvider()->setResource(mAOTypeName.c_str(), name, assetAuthor);
	}
	return assetAuthor;
}

template <class T_Module, class T_Asset, class T_AssetAuthoring>
NxApexAssetAuthoring* ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
createAssetAuthoring(NxParameterized::Interface* params, const char* name)
{
	T_AssetAuthoring* assetAuthor = PX_NEW(T_AssetAuthoring)(DYNAMIC_CAST(T_Module*)(mModule), mAssetAuthors, params, name);

	if (assetAuthor)
	{
		NiGetApexSDK()->getNamedResourceProvider()->setResource(mAOTypeName.c_str(), name, assetAuthor);
	}
	return assetAuthor;
}

#endif // WITHOUT_APEX_AUTHORING

template <class T_Module, class T_Asset, class T_AssetAuthoring>
void ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
releaseAssetAuthoring(NxApexAssetAuthoring& nxauthor)
{
	T_AssetAuthoring* author = DYNAMIC_CAST(T_AssetAuthoring*)(&nxauthor);

	NiGetApexSDK()->getInternalResourceProvider()->setResource(mAOTypeName.c_str(), nxauthor.getName(), NULL, false, false);
	author->destroy();
}

template <class T_Module, class T_Asset, class T_AssetAuthoring>
physx::PxU32 ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
forceLoadAssets()
{
	physx::PxU32 loadedAssetCount = 0;

	for (physx::PxU32 i = 0; i < mAssets.getSize(); i++)
	{
		T_Asset* asset = DYNAMIC_CAST(T_Asset*)(mAssets.getResource(i));
		loadedAssetCount += asset->forceLoadAssets();
	}
	return loadedAssetCount;
}

template <class T_Module, class T_Asset, class T_AssetAuthoring>
bool ApexAuthorableObject<T_Module, T_Asset, T_AssetAuthoring>::
getAssetList(NxApexAsset** outAssets, physx::PxU32& outAssetCount, physx::PxU32 inAssetCount)
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
		T_Asset* asset = DYNAMIC_CAST(T_Asset*)(mAssets.getResource(i));
		outAssets[i] = static_cast<NxApexAsset*>(asset);
	}

	return true;
}

}
} // end namespace physx::apex

#endif	// __APEXAUTHORABLEOBJECT_H__
