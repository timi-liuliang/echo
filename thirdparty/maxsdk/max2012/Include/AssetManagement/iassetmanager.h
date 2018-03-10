//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface for the asset manager.
// DATE: June.10.2008
// AUTHOR: Chloe Mignot
//***************************************************************************/

#pragma once

#include "..\maxheap.h"
#include "..\iFnPub.h"
#include "IAssetAccessor.h"
#include "..\Path.h"
#include "..\Noncopyable.h"
#include "AssetType.h"
#include "AssetUser.h"
#include "AssetId.h"
#include "assetmanagementExport.h"


#define IID_ASSET_MANAGER	Interface_ID(0x35331479, 0x539570e8)

namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetManagerInterfaceImp;

		class IAssetManager: public FPStaticInterface, public MaxSDK::Util::Noncopyable
		{
		public:
			//! \brief Return an AssetUser object given a file name and an asset type. 
			//! Automatically adds a reference to the asset identified by the AssetId, this reference will 
			//! be removed at the destruction of the AssetUser.
			//! \param[in] filename A file name which can be either a full path ("C:\bitmaps\a.bmp"), 
			//! a relative path("..\bitmaps\a.bmp"), or just a filename("a.bmp").
			//! \param[in] type The asset type, see AssetType.
			//! \param[in] autoAcquire if true, if an asset corresponding to the parameters does not exist, a new asset will be created. If false, a new asset will not be created.
			//! \return returns AssetUser instance created.
			virtual AssetUser GetAsset(const MaxSDK::Util::Path& filename, AssetType type, bool autoAcquire=true) = 0;

			//! \brief Return an AssetUser object given a file name and an asset type. 
			//! Automatically adds a reference to the asset identified by the AssetId, this reference will 
			//! be removed at the destruction of the AssetUser.
			//! \param[in] filename A file name which can be either a full path ("C:\bitmaps\a.bmp"), 
			//! a relative path("..\bitmaps\a.bmp"), or just a filename("a.bmp").
			//! \param[in] type The asset type, see AssetType.
			//! \param[in] autoAcquire if true, if an asset corresponding to the parameters does not exist, a new asset will be created. If false, a new asset will not be created.
			//! \return returns AssetUser instance created.
			virtual AssetUser GetAsset(const MCHAR* filename, AssetType type, bool autoAcquire=true) = 0;

			//! \brief Return an AssetUser object given a file name and an asset type. 
			//! Automatically adds a reference to the asset identified by the AssetId, this reference will 
			//! be removed at the destruction of the AssetUser.
			//! \param[in] filename A file name which can be either a full path ("C:\bitmaps\a.bmp"), 
			//! a relative path("..\bitmaps\a.bmp"), or just a filename("a.bmp").
			//! \param[in] type The asset type, see AssetType.
			//! \param[in] autoAcquire if true, if an asset corresponding to the parameters does not exist, a new asset will be created. If false, a new asset will not be created.
			//! \return returns AssetUser instance created.
			virtual AssetUser GetAsset(const MSTR& filename, AssetType type, bool autoAcquire=true) = 0;

			//! \brief  Return an AssetUser object pointing to the id passed
			//! Automatically adds a reference to the asset identified by the AssetId, this reference will 
			//! be removed at the destruction of the AssetUser.
			//! \param[in] assetId the asset of the asset
			//! \return returns AssetUser instance created. 
			virtual AssetUser GetAsset(const AssetId& assetId)const = 0;

			//! \brief Release the reference to the asset identified by the AssetId
			//! If the asset doesn't have any other reference it will be removed from the asset manager
			//! \param[in] assetId asset id to be released
			//! \return true if the asset was successfully found and released.
			virtual bool ReleaseReference(const AssetId& assetid) = 0;

			//! \brief Release the reference to the asset identified by the AssetId string
			//! \param[in] guid_string the asset id string to be released
			//! \return true if the asset was successfully found and released.
			virtual bool ReleaseReference(const MCHAR* guid_string) = 0;

			//! \brief Add a reference to the asset identified by the AssetId
			//! \param[in] assetid asset id for which we want to add a reference
			//! \return true if the asset was successfully found and a reference added.
			virtual bool AddReference(const AssetId& assetid) = 0;

			//! \brief Add a reference to the asset identified by the AssetId string
			//! \param[in] guid_string asset id string for which we want to add a reference
			//! \return true if the asset was successfully found and a reference added.
			virtual bool AddReference(const MCHAR* guid_string) = 0;

			//! \brief Convert an AssetId object to a string
			//! \param[in] assetid assetid that will be converted
			//! \return the asset id converted to a string
			virtual MSTR AssetIdToString(const AssetId& assetId)const = 0;

			//! \brief Try to convert an a string to an AssetId
			//! \param[in] string string to be converted
			//! \param[out] assetid will be filled with the AssetId if the method is successful
			//! \return true if the string was successfully converted , false otherwise (if the string wasnt a valid AssetId)
			virtual bool StringToAssetId(const MSTR& string, AssetId& assetId)const = 0;

			//! \brief Load an asset from the scene file
			//! \param[in] iload the ILoad interface
			//! \param[out] assetUser will be filled with the AssetUser if the method is successful
			//! \return IO_OK if the asset was successfully loaded , IO_ERROR otherwise
			virtual IOResult LoadAsset(ILoad* iload, AssetUser &assetUser) = 0;

			//! \brief Remaps asset id loaded from scene file to new asset id for the asset.
			//! The asset id in the file being loaded may conflict with an existing asset id. Therefor,
			//! immediately after loading an asset id it must be passed through this method to get the
			//! new asset id
			//! \param[in,out] assetid asset id to remap
			virtual void RemapLoadedAssetID(AssetId& assetId) = 0;

			//! \brief Called by system while loading the 3ds Max file. Registers asset metadata loaded from file
			//! \param[in] assetid asset id for asset metadata
			//! \param[in] name A file name which can be either a full path ("C:\bitmaps\a.bmp"), 
			//! a relative path("..\bitmaps\a.bmp"), or just a filename("a.bmp").
			//! \param[in] type The asset type, see AssetType.
			virtual void RegisterAssetMetaData(const AssetId& assetId, const MSTR& filename, AssetType type) = 0;

			//! \brief Called by system to save the metadata for the active assets to the 3ds Max file.
			//! \param[in] isave the ISave interface
			virtual IOResult Save(ISave *isave) = 0;

			//! \brief Called by system to load the metadata for all assets from the 3ds Max file.
			//! \param[in] iload the ILoad interface
			virtual IOResult Load(ILoad *iload) = 0;

			//! \brief Called by system before saving to the 3ds Max file. Used to identify assets to store metadata for.
			//! \param[in] isave the ISave interface
			virtual void PreSave(ISave *isave) = 0;

			//! \brief Called by system before loading a 3ds Max file. Preps the asset manager for loading assets from the file.
			//! \param[in] iload the ILoad interface
			virtual void PreLoad(ILoad *iload) = 0;
			
			//! \brief Called by system after loading the 3ds Max file. Removes unused asset metadata.
			//! \param[in] iload the ILoad interface
			virtual void PostLoad(ILoad *iload) = 0;

			//! \brief Returns the only instance of this manager.	
			AssetMgmntExport static IAssetManager* GetInstance();
		
		};
	}//End of namespace AssetManagement
}

