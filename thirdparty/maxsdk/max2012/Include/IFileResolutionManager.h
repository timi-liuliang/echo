//**************************************************************************/
// Copyright 2008 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.   

//**************************************************************************/
// DESCRIPTION: IFileResolutionManager.h - Public interface for resolving the 
// physical location of asset files
// AUTHOR: Peter Ochodo
// DATE: 2008-06-09 
//***************************************************************************/


#pragma once

#include "iFnPub.h"
#include "maxtypes.h"
#include "Path.h"
#include "AssetManagement\AssetType.h"
#include "AssetManagement\assetmanagementExport.h"
#include "Noncopyable.h"
#include "windows.h"
#include "AssetManagement\AssetUser.h"

#define FILERESOLUTIONMGR_INTERFACE	Interface_ID(0x675b2cab, 0x19fe1fad)

/* \brief Finds the physical location of an asset file
	
	The File Resolution Manager(FRM) returns an absolute file path as soon as it finds the physical location of the asset file.
	This file path is converted to an absolute UNC path	if the user preference "Resolve Path to UNC" 
	is turned on. See the IPathConfigMgr::SetResolveUNC(bool aFlag)

	If the asset filename is an absolute path:
	- the FRM first checks the absolute path for the asset file
	- then, if a scenefile is currently open, it checks whether the file is in the same 
	  directory as the current scenefile 
	- then it checks the User Paths directories associated with the asset's AssetType
	- then, if a scenefile is currently open, it checks all the subdirectories of the folder that 
	  contains this current scenefile
	- then it gives up

	If the asset filename is a relative path:
	- the FRM roots the relative path to the 3dsmax project folder and checks whether the file is there
	- then, if a scenefile is currently open, the FRM roots the relative path to the folder that
	  holds this current scenefile, then checks if the file exists there
	- then, if a scenefile is currently open, the FRM checks if the asset filename (without the relative
	  path) exists in the folder that holds this current scenefile
	- then, the FRM roots the relative path to the User Paths directories associated with the asset's AssetType
	  and checks whether the file exists there
	- then, if a scenefile is currently open, the FRM checks for the asset filename (without relative path) in
	  all the subdirectories of the folder that contains this current scenefile.
	- then it gives up

*/
class IFileResolutionManager : public FPStaticInterface, public MaxSDK::Util::Noncopyable
{
public:

	//! \brief Returns the only instance of this manager.	
	AssetMgmntExport static IFileResolutionManager* GetInstance();

	/*! \brief Resolves the physical location of an asset file

		Gets an absolute UNC path of an asset file

		\param[in] filePath : is a possible location of the asset file. This location 
		may be a relative or absolute file path. If filePath is a relative file path, it
		may rooted at the max project folder(by default the folder that contains the 3dsmax.exe file) or
		it may be rooted at the folder holds the scenefile, or
		it may be rooted at one of the User Paths for the specified assetType, in this order.
		filePath includes the asset's filename. 
		
		\param[in] assetType: e.g. kXRefAsset, kBitmapAsset, kVideoPost, etc

		\param[in] byPassCache : If set to true, the file resolution manager bypasses the cache and 
		attempts to find the asset file on disk or network.

		\return a MSTR with an absolute path or the where the asset is located if successful; otherwise
		returns an empty MSTR if it fails to find the asset file
	*/
	virtual MSTR GetFullFilePath(const MCHAR* filePath,MaxSDK::AssetManagement::AssetType assetType, bool byPassCache = false) = 0;

	/*! \brief Resolves the physical location of an asset file

		Gets an absolute UNC path of an asset file

		\param[in] filePath : is a possible location of the asset file. This location 
		may be a relative or absolute file path. If filePath is a relative file path, it
		may rooted at the max project folder(by default the folder that contains the 3dsmax.exe file) or
		it may be rooted at the folder holds the scenefile , or
		it may be rooted at one of the User Paths for the specified assetType, in this order.
		filePath includes the asset's filename. 
		
		\param[out] filePath : is set to an absolute UNC path if the asset file is found. If
		the asset file is not found it is left unchanged.
		
		\param[in] assetType: e.g. kXRefAsset, kBitmapAsset, kVideoPost, etc

		\param[in] byPassCache: If set to true, the file resolution manager bypasses the cache and 
		attempts to find the asset file on disk or network.

		\return true if the asset file is located successful; otherwise returns false
	*/
	virtual bool GetFullFilePath(MaxSDK::Util::Path& filePath, MaxSDK::AssetManagement::AssetType assetType, bool byPassCache = false) = 0;

	/*! \brief Resolves the physical location of an asset file

		Gets an absolute UNC path of an asset file
		\param[in] assetUser : the AssetUser to retrieve the asset file information from 
		
		\param[out] filePath : is set to an absolute UNC path if the asset file is found. If
		the asset file is not found it is left unchanged.

		\param[in] byPassCache: If set to true, the file resolution manager bypasses the cache and 
		attempts to find the asset file on disk or network.
		
		\return true if the asset file is located successful; otherwise returns false
	*/
	virtual bool GetFullFilePath(const MaxSDK::AssetManagement::AssetUser& assetUser, MaxSDK::Util::Path& filePath,
								 bool byPassCache = false) = 0;

	/*! \brief Resolves the physical location of an asset file

		Gets an absolute UNC path of an asset file
		\param[in] assetUser : the AssetUser to retrieve the asset file information from 
		
		\param[out] filePath : is set to an absolute UNC path if the asset file is found. If
		the asset file is not found it is left unchanged.

		\param[in] byPassCache: If set to true, the file resolution manager bypasses the cache and 
		attempts to find the asset file on disk or network.
		
		\return true if the asset file is located successful; otherwise returns false
	*/
	virtual bool GetFullFilePath(const MaxSDK::AssetManagement::AssetUser& assetUser, MSTR& filePath,
								bool byPassCache = false) = 0;
	
	/*! \brief Resolves the physical location of an asset file

		Gets an absolute UNC path of an asset file
		\param[in] assetID : a unique ID that specifies a particular asset
		
		\param[out] filePath : is set to an absolute UNC path if the asset file is found. If
		the asset file is not found it is left unchanged.

		\param[in] byPassCache: If set to true, the file resolution manager bypasses the cache and 
		attempts to find the asset file on disk or network.
		
		\return true if the asset file is located successful; otherwise returns false
	*/
	virtual bool GetFullFilePath(const MaxSDK::AssetManagement::AssetId& assetID, MaxSDK::Util::Path& filePath,
								 bool byPassCache = false) = 0;

	/*! \brief Resolves the physical location of an asset file

	Gets an absolute UNC path of an asset file
	\param[in] assetID : a unique ID that specifies a particular asset

	\param[out] filePath : is set to an absolute UNC path if the asset file is found. If
	the asset file is not found it is left unchanged.

	\param[in] byPassCache: If set to true, the file resolution manager bypasses the cache and 
	attempts to find the asset file on disk or network.

	\return true if the asset file is located successful; otherwise returns false
	*/
	virtual bool GetFullFilePath(const MaxSDK::AssetManagement::AssetId& assetID, MSTR& filePath, bool byPassCache = false) = 0;

	/*! \brief Resolves the physical location of an asset file

		Gets an absolute UNC path of an asset file
		\param[in] assetUser : the AssetUser to retrieve the asset file information from 
		
		\param[in] byPassCache: If set to true, the file resolution manager bypasses the cache and 
		attempts to find the asset file on disk or network.

		\return a MSTR with an absolute path or the where the asset is located if successful; otherwise
		returns an empty MSTR if it fails to find the asset file
	*/
	virtual MSTR GetFullFilePath(const MaxSDK::AssetManagement::AssetUser& assetUser, bool byPassCache = false) = 0;

	/*! \brief Resolves the physical location of an asset file

		Gets an absolute UNC path of an asset file
		\param[in] assetUser : the AssetUser to retrieve the asset file information from
		
		\param[in] byPassCache: If set to true, the file resolution manager bypasses the cache and 
		attempts to find the asset file on disk or network.

		\return a MSTR with an absolute path or the where the asset is located if successful; otherwise
		returns an empty MSTR if it fails to find the asset file
	*/
	virtual MSTR GetFullFilePath(const MaxSDK::AssetManagement::AssetId& assetId, bool byPassCache = false) = 0;
	
	/*! \brief Returns the maximum age of cached data in milliseconds for it to be valid
	Cache data that is older that than this duration is not used of file asset resolution.
	\return the maximum age of cached data in milliseconds for it to be valid
	*/
	virtual DWORD GetMaximumCacheDuration() = 0;

	/*! \brief Sets the maximum age of valid cached data in milliseconds
	Cache data that is older that than this duration is not used of file asset resolution.
	\param[in] newMaxDuration : maximum age of valid cached data in milliseconds.
	*/
	virtual void SetMaximumCacheDuration(DWORD newMaxDuration) = 0;

	/*! \brief Returns true if the file resolution manager is using cached data to resolve files
	\return true if caching is used
	*/	
	virtual bool IsCachingEnabled() = 0;

	//! \brief This makes the file resolution manager use cached results when resolving files
	virtual void EnableCaching() = 0;

	//! \brief This makes the file resolution manager not use cached results when resolving files
	virtual void DisableCaching() = 0;

	/*! \brief Pushes current state and sets whether caching of unresolved results is permitted
	Normally caching of unresolved results is off, and turned on only when loading a lot of asset 
	files quickly, like at the beginning of a render
	\param[in] newVal : whether caching of unresolved results is permitted.
	*/
	virtual void PushAllowCachingOfUnresolvedResults(bool newVal) = 0;

	//! \brief Pops state of whether caching of unresolved results is permitted. 
	virtual void PopAllowCachingOfUnresolvedResults() = 0;

	/*! \brief Transforms an absolute file path to a UNC path

	Transforms an absolute file path to a UNC path
	\param[in] aFullFilePath : an absolute file path.

	\param[out] aFullFilePath : a UNC path. It is only converted to a UNC path
	if the user preference "Resolve Path to UNC" is turned on
	*/	
	virtual void DoGetUniversalFileName(MaxSDK::Util::Path& aFullFilePath) const = 0;

	/*! \brief Transforms an absolute file path to a UNC path

	Transforms an absolute file path to a UNC path
	\param[in] fullFilePath : an absolute file path.

	\param[out] fullFilePath : a UNC path. It is only converted to a UNC path
	if the user preference "Resolve Path to UNC" is turned on
	*/ 
	virtual void DoGetUniversalFileName(MSTR& fullFilePath) const = 0;

	/* \brief Writes File Resolution Manager's settings to the 3dsmax's .ini file

	Writes whether or not the FRM has caching enabled. Also writes the maximum caching duration
	that the FRM uses to validate cached data 
	*/
	virtual void UpdateDotIniFile() = 0;
	
	//! \brief Return the number of asset types currently registered
	//! \return number of currently registered types
	virtual int GetNumAssetTypes()const = 0;

	/*! \brief Returns the asset type ID for the asset type name
	If the asset type does not exist, it registers the new asset type identified by the string 
	passed in parameter and returns the asset type ID.
	If the asset type is already registered, it returns the ID corresponding to the asset type name.
	\param[in] assetType : the asset type name.
	\return the asset type ID
	*/
	virtual MaxSDK::AssetManagement::AssetType GetAssetType(const MSTR& assetType)const = 0;

	/*! \brief Returns the asset type name for the asset type
	If the asset type does not exist, a null pointer is returned.
	\param[in] assetType : the asset type ID to return a name for
	\return the asset type name
	*/
	virtual const MCHAR* GetAssetTypeString(MaxSDK::AssetManagement::AssetType assetType)const = 0;


protected:
	// virtual destructor
	virtual ~IFileResolutionManager(){}

};

