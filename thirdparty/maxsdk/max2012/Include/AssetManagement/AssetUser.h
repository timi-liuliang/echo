//**************************************************************************/
// Copyright 2008 Autodesk, Inc.  All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.   

//**************************************************************************/
// DESCRIPTION: Contains definitions of classes used in asset management
// AUTHOR: Chloe Mignot
// DATE: 2008-07-14 (YYYY-MM-DD) 
//***************************************************************************/

#ifndef _MAXSDK_ASSET_USER_H_
#define _MAXSDK_ASSET_USER_H_


#include "..\maxheap.h"
#include "..\strclass.h"
#include "..\path.h"
#include "..\ifnpub.h"
#include "..\autoptr.h"
#include "AssetType.h"
#include "AssetId.h"
#include "assetmanagementExport.h"

// forward declarations
class FileResolutionMgrImp;

#define IID_IASSET_USER Interface_ID(0x32f7bccf, 0xf1881566)

namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser: public IObject, public FPMixinInterface
		{
		private:
			class AssetUserImp;
			AutoPtr<AssetUserImp> mImpl;
			unsigned int m_referencesHeld; // IObject ref count
			static UINT64 num_created;
			static UINT64 num_alive;
		public:

			//! \brief Create an empty AssetUser pointing to kInvalidId AssetId 
			CoreExport AssetUser();

			//! \brief Create an AssetUser pointing to assetId.
			//! The constructor will automatically add a reference to the asset.
			//! \param assetId on which the AssetUser will point.
			CoreExport AssetUser(const AssetId& assetId);

			//! \brief Create an AssetUser pointing to the same asset as assetU.
			//! \param AssetUser we want to copy.
			CoreExport AssetUser(const AssetUser& assetU);

			//! \brief This function will remove the reference to the Asset on which this asset User points 
			//! and add a reference to the asset corresponding to the AssetUser passed.
			CoreExport AssetUser& operator=(const AssetUser& assetU);

			//! \brief Gets the AssetId of the asset to which this AssetUser points. 
			//! \return The GUID identifying the asset.
			CoreExport const AssetId& GetId()const;

			//! \brief Gets the AssetId of the asset to which this AssetUser points. 
			//! \return A string representation of the GUID identifying the asset.
			CoreExport MSTR GetIdAsString()const;

			//! \brief Gets the type of the asset to which this AssetUser points. 
			//! \return The type of the asset. 
			CoreExport AssetType GetType() const;

			//! \brief Gets the current file name of the asset to which this AssetUser points. 
			//! \return The current file name of this asset. 
			CoreExport const MSTR& GetFileName()const;

			//! gets an absolute UNC path of the asset file
			//! \return a MSTR with an absolute path to where the asset is located if successful; otherwise
			//! returns an empty MSTR if it fails to find the asset file
			CoreExport MSTR GetFullFilePath()const;

			//! \brief resolves the physical location of the asset to which this AssetUser points.
			//!
			//! gets an absolute UNC path of an asset file
			//! \param[out] filePath : is set to an absolute UNC path if the asset file is found. If
			//! the asset file is not found it is left unchanged.
			//! 
			//! \return true if the asset file is located successful; otherwise returns false
			CoreExport bool GetFullFilePath(MaxSDK::Util::Path& filePath)const;

			//! \brief resolves the physical location of the asset to which this AssetUser points.
			//!
			//! gets an absolute UNC path of an asset file
			//! \param[out] filePath : is set to an absolute UNC path if the asset file is found. If
			//! the asset file is not found it is left unchanged.
			//! 
			//! \return true if the asset file is located successful; otherwise returns false
			CoreExport bool GetFullFilePath(MSTR& filePath)const;

			//! \brief Flags the asset as having been saved to a max scene file. 
			//! This method is called if the AsssetId of the asset is stored to the max scene file 
			//! (typically as a string) rather than the Save method being called on this AssetUser.
			//! This tells the Asset Manager that the metadata for the asset must be saved to the
			//! scene file (asset metadata is saved only for those assets actually written to the 
			//! scene file.
			//! Note: If you save the AssetId instead of the AssetUser, on scene load you must pass
			//! the AssetId read through IAssetManager::RemapLoadedAssetID before getting an AssetUser
			//! using the AssetId
			CoreExport void SetUsedInFileSave();

			//! \brief Resets the current asset id.  Remove the reference to the asset held by this AssetUser and sets the 
			//! internal value to kInvalidId.
			CoreExport void Reset();

			//! \brief Called for saving data.
			/*! Called to allow the asset to save its data.
			\param  isave - The ISave interface. 
			\return  One of the following values: <b>IO_OK, IO_ERROR</b>.
			*/
			CoreExport IOResult Save(ISave* isave) const;

			CoreExport ~AssetUser();

			//! \brief Gets the total number of AssetUsers created during this 3dsmax session. 
			//! \return The number of AssetUsers created. 
			static CoreExport UINT64 GetNumCreated();

			//! \brief Gets the current number of AssetUser instances. 
			//! \return The current number of AssetUsers.
			static CoreExport UINT64 GetNumAlive();


			// this will enable the FRMImp class to invoke the protected functions below
			friend class ::FileResolutionMgrImp;

			// IObject virtual methods
			BaseInterface* GetInterface(Interface_ID id);
			BaseInterface * GetInterfaceAt(int i)const;
			MCHAR * GetIObjectName() { return _M("AssetUser");}
			int NumInterfaces()const;
			void AcquireIObject();
			void ReleaseIObject();
			void DeleteIObject();

			// FPMixinInterface virtual methods
			LifetimeType	 LifetimeControl() { return wantsRelease; }
			FPInterfaceDesc* GetDescByID(Interface_ID id);
			Interface_ID	 GetID() { return IID_IASSET_USER; }
			FPInterfaceDesc* GetDesc() { return GetDescByID(IID_IASSET_USER); }

			// FP-published symbolic enumerations
			enum {  asset_type };

			// Function IDs for function publishing
			enum
			{
				kGetAssetId,
				kGetFileName,
				kGetType,
				kCanGetFullFilePath,
				kGetFullFilePath,
			};
			#pragma warning(push)
			#pragma warning(disable:4238)
			BEGIN_FUNCTION_MAP
				FN_0(kGetAssetId, TYPE_TSTR_BV, GetIdAsString);
				FN_0(kGetFileName, TYPE_TSTR_BV, GetFileName);
				FN_0(kGetType, TYPE_ENUM, GetType);
				FN_1(kCanGetFullFilePath, TYPE_bool, GetFullFilePath, TYPE_TSTR_BR);
				FN_0(kGetFullFilePath, TYPE_TSTR_BV, GetFullFilePath);
			END_FUNCTION_MAP
			#pragma warning(pop)

		protected:
			/*! \brief Gets the absolute path to the most recently resolved location of the asset that this AssetUser points to

			This function returns an empty string if the asset's location has not yet been resolved 
			by the file resolution manager.
			\return the absolute path to the most recently resolved location of the asset
			*/
			CoreExport const MSTR& GetResolvedAssetPath() const;

			/*! \brief Sets the absolute path to the most recently resolved location of the asset that this AssetUser points to

			This function only accepts an absolute path or an empty string. This function is usually invoked by 
			the file resolution manager after it attempts to resolve an asset's location. If the file
			resolution manager fails to resolve the physical location of an asset file, 'resolvedPath'
			is an empty string (i.e. resolvedPath equals "")

			param[in] resolvedPath the most recent resolved path of the asset file

			\return true if the new resolved path is set to parameter resolvedPath
			*/
			CoreExport bool SetResolvedAssetPath(const MSTR& resolvedPath);

			/* \brief Returns the time in milliseconds that the asset was last resolved by the file resolution manager

			This is the time since Windows started. 

			If the asset has not yet been resolved, it returns 0

			Note: the asset referred to here is the asset that this AssetUser instance points to

			\return the time in milliseconds that the asset was last resolved
			*/
			CoreExport DWORD GetTimePathWasResolved() const;

			/* \brief Sets the time that the asset's location was most recently resolved

			newTime can be obtained by a call to the Win32 function, timeGetTime(VOID). It
			is the time in milliseconds since Windows was started.

			param[in] newTime the time in milliseconds that the asset was most recently resolved

			Note: the asset referred to here is the asset that this AssetUser instance points to
			*/
			CoreExport void SetTimePathWasResolved(DWORD newTime);

		};

	}
}

#if !defined(NDEBUG) && defined(BUILD_WITH_IMPL)
// Debug and Hybrid builds can pull in implementation details
#include "..\..\..\core\AssetUserImp.h"
#endif

#endif