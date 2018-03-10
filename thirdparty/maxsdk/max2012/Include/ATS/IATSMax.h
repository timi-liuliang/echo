//**************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interfaces for ATSMax (Asset Tracking System)
// AUTHOR: Michael Russo - created December 15, 2004
//***************************************************************************/

#pragma once

#include "..\iFnPub.h"
#include "IATSProvider.h"
#include "..\containers\Array.h"
// forward declarations
class Animatable;

///////////////////////////////////////////////////////////////////////////////
//
// Consts and Typedefs
//
///////////////////////////////////////////////////////////////////////////////

//
// Local file system status flags
//
/*! Unknown file system status */
const DWORD kATSFSStatusUnknown				= 0x000;
/*! File found in specified location */
const DWORD kATSFSStatusOk					= 0x001;
/*! File not found in specified location */
const DWORD kATSFSStatusMissing				= 0x002;
/*! File not found in specified location, but found in search paths */
const DWORD kATSFSStatusResolved			= 0x004;
/*! File located on network drive and status check was optionally skipped */
const DWORD kATSFSStatusNetPathNoCheck		= 0x008;


///////////////////////////////////////////////////////////////////////////////
//
// class IATSMax
//
///////////////////////////////////////////////////////////////////////////////

//! ID for IATSMax interface
#define IATSMAX_INTERFACE	Interface_ID(0x20235822, 0x2632197)

//! \brief Interface to Asset Tracking System
/*! This interface gives access to the Asset Tracking System. This includes
control over the Asset Tracking dialog, support dialogs, and version control system
integration. It also provides access to the Asset Repathing UI functionality and
file dependency information. The version control system related methods (such as checkin,
checkout, etc.) incorporate the common dialogs used for all providers. For lower level
access to providers use the IATSProvider interface.
\see IATSProvider, ATSFileList
*/
class IATSMax : public FPStaticInterface {

public:
	//
	// Dialog Access
	//

	//! \name Dialog Access
	//@{
	//! \brief Get visibility state of dialog
	/*! \return true if visible, false if not
	*/
	virtual bool			GetVisible() = 0;
	//! \brief Set visibility state of dialog
	/*! \param[in] bVisible true to display dialog, false to hide dialog
	*/
	virtual void			SetVisible( bool bVisible ) = 0;

	//! \brief Number of files selected in dialog
	/*! \return number of files selected in dialog
	*/
	virtual int				NumFilesSelected() = 0;
	//! \brief Get selected files
	/*! \param[out] fileList reference to a Tab of TSTRs.  This parameter
	will be filled with a copy of the selected files names
	\return number of files selected in dialog
	*/
	virtual int				GetSelectedFiles( MaxSDK::Array<MSTR> &fileList ) = 0;

	//! \brief Get a tab of files based on their file system status
	/*! \param[in] dwFSStatus is a mask of kATSFSStatusXXX flags used to match
	against file entries.
	\param[out] fileList reference to a Tab of TSTRs.  This parameter
	will be filled with a copy of the files matching the dwFSStatus flags.
	\return number of files found
	*/
	virtual	int				GetFilesByFileSystemStatus( DWORD dwFSStatus, MaxSDK::Array<MSTR> &fileList ) = 0;

	//! \brief Return the file system status of the specified file
	/*! \param[in] szFilename string containing the filename to retrieve status from.
	\return file system status flag
	*/
	virtual DWORD			GetFileSystemStatus( const MCHAR* szFilename ) = 0;

	//! \brief Get the resolved file paths for a given list of files
	/*! \param[in] fileList reference to a Tab of TSTRs.
	\param[out] resolvedFileList reference to a Tab of TSTRs that contains the same number of entries
	in fileList.  If the file was found or is still missing the entry in resolvedFileList will be 
	the same as fileList.  If the file was resolved the entry in resolvedFileList will contains 
	the resolved file path.  Resolved files are files not found in the location specified within the
	scene file but are found using the standard and user defined search paths.
	\return the number of resolved paths
	*/
	virtual int				GetResolvedPaths( const MaxSDK::Array<MSTR> &fileList, MaxSDK::Array<MSTR> &resolvedFileList ) = 0;

	//! \brief Get the list of files that are used by the given Animatable.
	/*! \param[in] animatable The animatable for which the file dependencies are to be listed.
		\param[out] fileList The table through which the file list is returned.
		\param[in] includeRefHierarchy Set to true to list dependencies of the entire reference
		hierarchy; set to false to limit the list to 'animatable'.
	*/
	virtual int				GetDependencyFileList( Animatable* animatable, MaxSDK::Array<MSTR>& fileList, bool includeRefHierarchy ) = 0;

	//! \brief Number of files in dialog
	/*! \return number of files in dialog
	*/
	virtual int				NumFiles() = 0;
	//! \brief Get files
	/*! \param[out] fileList reference to a Tab of TSTRs.  This parameter
	will be filled with a copy of the files names in the dialog
	\return number of files in dialog
	*/
	virtual int				GetFiles( MaxSDK::Array<MSTR> &fileList ) = 0;
	//! \brief Populate an ATSFileList object
	/*! \see ATSFileList
	\param[out] atsFileList reference to an ATSFileList object.
	\param[in] bSetAllActive specifies if all of the files added to the ATSFileList
	object will be marked as active (kATSStatusActive)
	\param[in] bTagExcluded specifies if files excluded from the current provider
	should be marked as excluded (kATSStatusExcluded)
	\return number of files added to atsFileList
	*/
	virtual int				GetATSFileList( ATSFileList &atsFileList, bool bSetAllActive, bool bTagExcluded ) = 0;

	//! \brief Get list of dependent files
	/*! \param[in] szFile string containing the filename to list dependent files from
	\param[in] bRecurse specifies if method should include all dependent files
	\param[out] fileList reference to a Tab of TSTRs.  This parameter
	will be filled with a copy of the dependent files names
	\param[in] bIncludeOutputFiles specifies if output files should be included in dependency list
	\return number of files added to fileList
	*/
	virtual int				GetDependentFiles( const MCHAR* szFile, bool bRecurse, MaxSDK::Array<MSTR> &fileList, bool bIncludeOutputFiles = true ) = 0;

	//! \brief Determine if a file is an input file and a true dependent file
	/*! \param[in] szFile string that contains the file name
	\return true if the file is an input file, or false if it is an output or not found
	*/
	virtual bool			IsInputFile( const MCHAR* szFile ) = 0;

	//! \brief Select files
	/*! \param[in] fileList reference to a Tab of TSTRs.  This parameter
	specifies the list of files to select in the dialog.
	*/
	virtual void			SelectFiles( const MaxSDK::Array<MSTR> &fileList ) = 0;
	//! \brief Clears all selections in dialog
	virtual void			ClearSelection() = 0;

	//! \brief Refreshes the dialog
	/*! This method both updates the dependent file list and if
	connected to an ATS Provider it will query the server for
	updated version control status
	*/
	virtual void			Refresh() = 0;
	//@}

	//
	// Options
	//
	//! \name Options
	//@{
	//! \brief Get disabled state of dialog
	/*! \return true if disabled, false if not
	*/
	virtual bool			GetDisabled() = 0;
	//! \brief Set disabled state of dialog
	/*! \param[in] bDisabled true to disable, false to enable
	*/
	virtual void			SetDisabled( bool bDisabled ) = 0;

	//! \brief Get silent state of dialog
	/*! If connected to an ATS Provider, the provider might not
	respect the silent flag and display dialogs.
	\return true if silent, false if not
	*/
	virtual bool			GetSilent() = 0;
	//! \brief Set silent state of dialog
	/*! \param[in] bSilent true to silent, false to enable
	*/
	virtual void			SetSilent( bool bSilent ) = 0;

	//! \brief Get autologin state of dialog
	/*! This will attempt to login to all registered ATS Providers
	when a project (scene file) is open.
	\return true if autologin is enabled, false if not
	*/
	virtual bool			GetAutoLogin() = 0;
	//! \brief Set autologin state of dialog
	/*! \param[in] bAutoLogin true to auto login, false to enable
	*/
	virtual void			SetAutoLogin( bool bAutoLogin ) = 0;

	//! \brief Get state of tree view
	/*! \return true if tree view is display, false if not
	*/
	virtual bool			GetTreeView() = 0;
	//! \brief Set state of tree view
	/*! \param[in] bTreeView true to display tree view, false to display table view
	*/
	virtual void			SetTreeView( bool bTreeView ) = 0;

	//! \brief Get state of table view
	/*! \return true if table view is display, false if not
	*/
	virtual bool			GetTableView() = 0;
	//! \brief Set state of table view
	/*! \param[in] bTableView true to display table view, false to display tree view
	*/
	virtual void			SetTableView( bool bTableView ) = 0;
	//! \brief Get state of Check Network Path property.
	/*! \return true if the ATS system checks for the existence of files on network drives
	*/
	virtual bool			GetCheckNetworkPaths() const = 0;
	//! \brief Set state of Check Network Path property.
	/*! \param[in] bCheckPaths true if the ATS should check for the existence of files on network drives
	*/
	virtual void			SetCheckNetworkPaths(bool bCheckPaths) = 0;

	//! \brief Get display state of excluded files
	/*! \return true if excluded files are display, false if they are not
	*/
	virtual bool			GetDisplayExcluded() = 0;
	//! \brief Set display state of excluded files
	/*! \param[in] bDisplay true to display excluded, false to hide excluded files
	*/
	virtual void			SetDisplayExcluded( bool bDisplay ) = 0;

	//! \brief Get the exclusion state of output files
	/*! \return true if output files are excluded, false if they are not
	*/
	virtual bool			GetExcludeOutputFiles() = 0;
	//! \brief Set the exclusion state of output files
	/*! \param[in] bExclude true to exclude output files, false to include output files
	*/
	virtual void			SetExcludeOutputFiles( bool bExclude ) = 0;
	//@}

	//
	// Aux Dialogs
	//

	//! \name Aux Dialog
	//@{
	//! \brief Display Prompts dialog
	virtual void			ShowPromptsDialog() = 0;
	//! \brief Display Working Comment dialog
	virtual void			ShowWorkingCommentDialog() = 0;
	//! \brief Display Status Log dialog
	virtual void			ShowStatusLogDialog() = 0;
	//@}

	//
	// Providers
	//
	//! \name Providers
	//@{
	//! \brief Number of registered providers
	/*! \return number of registered ATS providers
	*/
	virtual int				NumProviders() = 0;
	//! \brief Get IATSProvider pointer
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\return pointer to IATSProvider interface
	*/
	virtual IATSProvider*	GetProvider( int iProviderIndex ) = 0;
	//! \brief Get ATS Provider name
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\return string that contains name of provider
	*/
	virtual const MCHAR*	GetProviderName( int iProviderIndex ) = 0;
	//! \brief Get active ATS Provider index
	/*! \return index of active ATS Provider
	*/
	virtual int				GetActiveProvider() = 0;
	//! \brief Set active ATS Provider
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	*/
	virtual void			SetActiveProvider( int iProviderIndex ) = 0;
	//! \brief Login to specified ATS Provider
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\return true if the login was successful
	*/
	virtual bool			LoginProvider( int iProviderIndex ) = 0;
	//! \brief Logout of specified ATS Provider
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\return true if the logout was successful
	*/
	virtual bool			LogoutProvider( int iProviderIndex ) = 0;
	//! \brief Determine if a file is excluded by an ATS Provider
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\param[in] szFile string that contains the file name
	\return true if the file is excluded by the provider, false if it is not
	*/
	virtual bool			IsFileExcluded( int iProviderIndex, const MCHAR* szFile ) = 0;
	//! \brief Determine if a file is share locked by an ATS Provider.
	/*! A file is considered share locked if the file is checked out by another user at a shared
	location.  Share locked files should be treated as read-only by users who are accessing the
	file and are not the owner of the lock.
	\param[in] iProviderIndex 0-based index of ATS Provider. If iProviderIndex is -1, the method
	will check all providers for a share locked status.
	\param[in] szFile string that contains the file name
	\return true if the file is share locked by the provider, false if it is not
	*/
	virtual bool			IsFileShareLocked( int iProviderIndex, const MCHAR* szFile ) = 0;
	//! \brief Checkin list of files to an ATS Provider
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\param[in] fileList reference to a Tab of TSTRs.
	\param[in] strComment reference to MSTR
	\return true if status from version control provider was successful, false if otherwise
	*/
	virtual bool			Checkin( int iProviderIndex, const MaxSDK::Array<MSTR> &fileList, MSTR &strComment  ) = 0;
	//! \brief Checkout list of files to an ATS Provider
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\param[in] fileList reference to a Tab of TSTRs.
	\param[in] strComment reference to MSTR
	\return true if status from version control provider was successful, false if otherwise
	*/
	virtual bool			Checkout( int iProviderIndex, const MaxSDK::Array<MSTR> &fileList, MSTR &strComment  ) = 0;
	//! \brief Undo Checkout for list of files fr an ATS Provider
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\param[in] fileList reference to a Tab of TSTRs.
	\return true if status from version control provider was successful, false if otherwise
	*/
	virtual bool			UnCheckout( int iProviderIndex, const MaxSDK::Array<MSTR> &fileList ) = 0;
	//! \brief Add list of files to an ATS Provider
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\param[in] fileList reference to a Tab of TSTRs.
	\param[in] strComment reference to MSTR
	\param[in] pIsHidden optional pointer to Tab<bool> that corresponds to the fileList. A true entry indicates that
	the file will be added with a hidden status (if supported by the provider).
	\return true if status from version control provider was successful, false if otherwise
	*/
	virtual bool			AddFiles( int iProviderIndex, const MaxSDK::Array<MSTR> &fileList, MSTR &strComment, Tab<bool> *pIsHidden = NULL ) = 0;
	//! \brief Get latest version of list of files for an ATS Provider
	/*! \param[in] iProviderIndex 0-based index of ATS Provider
	\param[in] fileList reference to a Tab of TSTRs.
	\return true if status from version control provider was successful, false if otherwise
	*/
	virtual bool			GetLatest( int iProviderIndex, const MaxSDK::Array<MSTR> &fileList ) = 0;
	//@}

	//
	// Policies
	//

	//! \name Policies
	//@{
	//! \brief Policy check for dependent files
	/*! This will verify that the dependent files, as reported by the ATS Provider,
	are up to date on the user's machine.
	\param[in] iProviderIndex 0-based index of ATS Provider
	\param[in] szFilename string that contains the file name to check dependent files for
	*/
	virtual void			CheckForDependentFiles( int iProviderIndex, MCHAR *szFilename ) = 0;
	//! \brief Policy check for checked out files
	/*! This will warn the user of any files that are currently checked out.  This is typically
	called when a scene is about to be closed and the user should check in any checked out files.
	\param[in] iProviderIndex 0-based index of ATS Provider
	*/
	virtual void			CheckForCheckedOutFiles( int iProviderIndex ) = 0;
	//! \brief Policy check for uncontrolled files
	/*! This will warn the user of any files that are not under version control.  This is typically
	called when a scene is about to be closed and the user should add any uncontrolled files
	to the ATS provider.
	\param[in] iProviderIndex 0-based index of ATS Provider
	*/
	virtual void			CheckForUnControlledFiles( int iProviderIndex ) = 0;
	//! \brief Policy check for out dated files
	/*! This will warn the user of any files that are not up to date.  This is typically
	called when a scene is loaded and the user should update outdated files.
	\param[in] iProviderIndex 0-based index of ATS Provider
	*/
	virtual void			CheckForOutDatedFiles( int iProviderIndex ) = 0;
	//! \brief Policy check scene file
	/*! This will warn the user if the scene file is under control and it is not checked out.
	This is typically called when a scene is loaded.
	\param[in] iProviderIndex 0-based index of ATS Provider
	*/
	virtual void			CheckForSceneFileCheckOut( int iProviderIndex ) = 0;
	//@}

	//
	// Misc
	//

	//! \name Misc
	//@{
	//! \brief Set the current Working Comment
	/*! \param[in] szComment string that contains the comment
	*/
	virtual void			SetWorkingComment( const MCHAR* szComment ) = 0;
	//! \brief Get the current Working Comment
	/*! \return string that contains the comment
	*/
	virtual const MCHAR*	GetWorkingComment() = 0;

	//! \brief Append status message to Status Log
	/*! \param[in] szStatus string that contains the status message
	*/
	virtual void			AppendStatusLog( const MCHAR* szStatus ) = 0;
	//! \brief Clear the contents of the Status Log
	virtual void			ClearStatusLog() = 0;
	//! \brief Get the conents of the Status Log
	/*! \return string that contains the contains the status log
	*/
	virtual const MCHAR*	GetStatusLog() = 0;
	//@}

	//! \name  Repathing functionality - via UI
	//@{
	//! \brief Sets the path on all listed assets to the passed in path

	/*! Sets the path component of all listed assets to the value passed
		in to this function.

		Note: If the Resolve files and folders to UNC paths option is turned on
		then this function will automatically convert mapped-drive paths to
		their UNC equivalent.
	 
	    \param[in] aNewPath The path value that will be applied to all
	    listed assets.
		\param[in] aCreateOutputFolder If repathing an output directory, and the path
		being repathed to does not exist, create path folder(s).  
	    \return true if the action succeeds and the asset(s) are repathed
	*/
	virtual bool SetPath(const MCHAR* aNewPath, bool aCreateOutputFolder = false) =0;
	//! \brief Sets the path on the asset selection set.

	/*! Sets the path component of selected assets to the value passed
		in to this function.

		Note: If the Resolve files and folders to UNC paths option is turned on
		then this function will automatically convert mapped-drive paths to
		their UNC equivalent.
	 
	    \pre 1+ assets should be selected in the ATS (see SelectFiles above)
	    \param[in] aNewPath The path value that will be applied to all
	    selected assets.
	    \param[in] aCreateOutputFolder If repathing an output directory, and the path
	    being repathed to does not exist, create path folder(s).   
	    \return true if the action succeeds and the asset(s) are repathed
	*/
	virtual bool SetPathOnSelection(const MCHAR* aNewPath, bool aCreateOutputFolder = false) =0;

	//! \brief Retargets the common-root portion of all assets listed in the ATS

	/*! Will retarget the common-root of all assets to the value passed in.  
		For example, if all assets belong to C:\ but are found in different sub-directories
		of C:\, then only the C:\ will be repathed.

		Note: If the Resolve files and folders to UNC paths option is turned on
		then this function will automatically convert mapped-drive paths to
		their UNC equivalent.
	 
	    \pre That all editable assets have some common-root portion
	    \param[in] aNewPath The path portion that will be prepended in the
	    place of the common-root.
	    \param[in] aCreateOutputFolder If repathing an output directory, and the path
	    being repathed to does not exist, create path folder(s).   
	    \return true if the action succeeds and the asset(s) are repathed
	*/
	virtual bool RetargetCommonRoot(const MCHAR* aNewPath, bool aCreateOutputFolder = false) =0;
	//! \brief Retargets the common-root portion of assets selected in the ATS

	/*! Will retarget the common-root of selected assets to the value passed in.  
		For example, if the selected assets belong to C:/resources/projectA/
		but are found in different sub-directories of that folder, then only
		the C:/resources/projectA/ portion will be repathed.

		Note: If the Resolve files and folders to UNC paths option is turned on
		then this function will automatically convert mapped-drive paths to
		their UNC equivalent.

	    \pre That all selected assets have some common-root portion
	    \param[in] aNewPath The path portion that will be prepended in the
	    place of the common-root.
	    \param[in] aCreateOutputFolder If repathing an output directory, and the path
	    being repathed to does not exist, create path folder(s).
	    \return true if the action succeeds and the asset(s) are repathed
	*/
	virtual bool RetargetCommonRootOfSelection(const MCHAR* aNewPath, bool aCreateOutputFolder = false) =0;
    //! \brief Retargets single-asset selection to the value passed in.

    /*! Retargets a single asset to the value passed-into this function.

		Note: If the Resolve files and folders to UNC paths option is turned on
		then this function will automatically convert mapped-drive paths to
		their UNC equivalent.
	 
        \pre That 1 asset only be selected in the ATS.
        \post The single asset is retargeted to the value passed into this function.
        \param[in] aNewPath The value (including filename) that the single asset should
	    be retargetd to.
	    \param[in] aCreateOutputFolder If repathing an output directory, and the path
	    being repathed to does not exist, create path folder(s).   
        \return true if the action succeeds and the asset(s) are repathed
	*/
	virtual bool RetargetSelection(const MCHAR* aNewPath, bool aCreateOutputFolder = false) =0;
	
	//! \brief Resolve the current selection set to its UNC equivalent,
	//	regardless of the current application setting.

	/*! Resolves the current selection set to UNC, regardless of the current
		application setting.

		\pre That one or more assets be selected in the ATS - one of the assets
		must be an absolute path
		\post One or more assets is resolved to UNC.
		\return true if the action succeeds and the asset(s) are resolved
	*/
	virtual bool ResolveSelectionToUNC() =0;

	//! \brief Converts the current selection set to relative paths.

	/*! Converts the current selection set to relative paths.  
		If the asset is Found (meaning that it is found in the search path) in
		a location other than that pointed to by the absolute path,
		then the relative path will be calculated based on this Found location.
		If not, then the absolute path is converted to an relative path relative
		to the current Project Folder setting, if possible.

		\pre That one or more assets be selected in the ATS - one of the assets
		must be an absolute path.
		\post One or more assets is resolved to a path relative to the current
		Project Folder setting, as long as the absolute path exists on the same
		drive (or UNC) as the current Project Folder.
		\return true if the action succeeds and the asset(s) are resolved
	*/
	virtual bool ResolveSelectionRelativeToProjectFolder() =0;

	//! \brief Converts the current selection set to absolute paths.

	/*! Converts the current selection set paths to absolute paths.
		If the asset is Found (meaning that it is found in the search path),
		then the absolute path will point to the found location.  If not, then
		the relative path is converted to an absolute path relative
		to the current Project Folder setting.

		\pre That one or more assets be selected in the ATS - one of the assets
		must be a relative path
		\post One or more assets is resolved to an absolute path.
		\return true if the action succeeds and the asset(s) are resolved
	*/
	virtual bool ResolveSelectionToAbsolute() =0;
	//@}

	//! \name  Repathing functionality - generic
	//@{
	//! \brief Generically remaps assets declared by a ReferenceTarget.

	/*! Generically remaps assets declared by ReferenceTarget whose path value
		is equal to aOldPath (case-insensitive comparison), to the value
		aNewPath passed-in.  This method calls the EnumAuxFiles implementation
		of ReferenceTarget to get a listing of assets.
	 
	    \param[in] refTarget The ReferenceTarget object from which assets are declared
	    \param[in] aOldPath Used to determine which asset to retarget.  Any
	    asset declared with this path will be retargeted to aNewPath.
	    \param[in] aNewPath The retarget value.
	    \param[in] aCreateOutputFolder If repathing an output directory, and the path
	    being repathed to does not exist, create path folder(s).    
	    \return true if the action succeeds and the asset(s) are repathed, false is
	    returned if no asset is repathed
	*/
	virtual bool RetargetAssets(ReferenceTarget& refTarget, const MCHAR* aOldPath, const MCHAR* aNewPath, bool aCreateOutputFolder = false) =0;
	//@}
};

//! Access to IATSMax interface
#define GetIATSMax() ((IATSMax*)GetCOREInterface(IATSMAX_INTERFACE))

