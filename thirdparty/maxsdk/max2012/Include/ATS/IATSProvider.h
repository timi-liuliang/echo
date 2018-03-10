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
// DESCRIPTION: Interface to ATS Providers 
// AUTHOR: Michael Russo - created December 15, 2004
//***************************************************************************/

#pragma once

#include <WTypes.h>
#include <string>
#include <vector>
#include "..\strbasic.h"
#include "..\MaxHeap.h"

#ifndef ATSExport
	#define ATSExport __declspec( dllexport )
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Consts and Typedefs
//
///////////////////////////////////////////////////////////////////////////////

/*! Result codes */
typedef int			ATSResult;
/*! Status flags */
typedef DWORD		ATSStatus;
/*! Option flags */
typedef DWORD		ATSOption;
/*! Client supplied pointers and values */
typedef LONG_PTR	ATSClientPtr;
/*! ATS file version type */
typedef DWORD		ATSVersion;
/*! ATS string type */
typedef std::basic_string<MCHAR> ATSString;

/*! Error test for ATSResults */
#define IS_ATS_ERROR(res)	( ((res) <  kATSResSuccess) ? true : false )
/*! Success test for ATSResults */
#define IS_ATS_SUCCESS(res)	( ((res) >= kATSResSuccess) ? true : false )

namespace ATS {

	//
	// ATS API Version
	//
	/*! Version number of ATS API */
	const DWORD		kATSAPIVersion				= 200;

	//
	// ATS Result Codes
	//
	/*! Error result. Feature not supported.  This error will be returned 
		for features not supported by the ATSProvider.
	*/
	const ATSResult kATSResErrorNotSupported	= -2;
	/*! General error result */
	const ATSResult kATSResError				= -1;
	/*! Successful result */
	const ATSResult kATSResSuccess				=  0;
	/*! Successful result. Last IATSProvider method call downloaded new files
		specified in the ATSFileList.
	*/
	const ATSResult kATSResSuccessReload		=  1;

	//
	// ATS File Status Flags Out
	//
	/*! No information available */
	const ATSStatus kATSStatusUnknown			= 0x00000000;
	/*! File is not under control */
	const ATSStatus kATSStatusNotControlled		= 0x00000001;
	/*! File is controlled */
	const ATSStatus kATSStatusControlled		= 0x00000002;
	/*! File is checked out by user */
	const ATSStatus kATSStatusCheckedOut		= 0x00000004;
	/*! File is checked out by another user */
	const ATSStatus kATSStatusCheckedOutOther	= 0x00000008;
	/*! File is out of date */
	const ATSStatus kATSStatusOutOfDate			= 0x00000010;
	/*! File has been deleted from server */
	const ATSStatus kATSStatusDeleted			= 0x00000020;
	/*! File does not exist locally */
	const ATSStatus kATSStatusMissing			= 0x00000040;
	/*! File has been locally modified */
	const ATSStatus kATSStatusModified			= 0x00000080;
	/*! File is hidden */
	const ATSStatus kATSStatusHidden			= 0x00000100;
	/*! File is shared and locked by another user */
	const ATSStatus kATSStatusShareLock			= 0x00000200;

	/*! File can be checked in */
	const ATSStatus kATSStatusCanCheckin		= 0x00010000;
	/*! File can be checked out */
	const ATSStatus kATSStatusCanCheckout		= 0x00020000;
	/*! Can undo checkout */
	const ATSStatus kATSStatusCanUndoCheckout	= 0x00040000;
	/*! Can get latest version of file */
	const ATSStatus kATSStatusCanGetLatest		= 0x00080000;
	/*! File can be added */
	const ATSStatus kATSStatusCanAddFile		= 0x00100000;
	/*! File history can be shown */
	const ATSStatus kATSStatusCanShowHistory	= 0x00200000;
	/*! File properties can be shown */
	const ATSStatus kATSStatusCanShowProperties	= 0x00400000;

	//
	// ATS File Status Flags In/Out
	//
	/*! File is marked as selected */
	const ATSStatus kATSStatusSelected			= 0x0001;
	/*! File is checked */
	const ATSStatus kATSStatusChecked			= 0x0002;
	/*! File is marked as active in ATSFileList */
	const ATSStatus kATSStatusActive			= 0x0004;
	/*! File is marked as excluded from Provider's control */
	const ATSStatus kATSStatusExcluded			= 0x0008;
	/*! File is marked for hidden status on Add action */
	const ATSStatus kATSStatusHide				= 0x0010;

	//
	// ATS Provider Options
	//
	/*! No options */
	const ATSOption	kATSOptNone					= 0x0000;
	/*! Keep file(s) checked out after checkin or add */
	const ATSOption	kATSOptKeepCheckedOut		= 0x0001;
	/*! Overwrite local after checkout or undo checkout */
	const ATSOption	kATSOptReplaceLocalCopy		= 0x0002;
	/*! Provider should suppress dialogs */
	const ATSOption	kATSOptSilent				= 0x0004;
	/*! Include children in file assoc */
	const ATSOption	kATSOptAssocChildren		= 0x0008;
	/*! Include parents in file assoc */
	const ATSOption	kATSOptAssocParents			= 0x0010;
	/*! Recurse file associations */
	const ATSOption	kATSOptAssocRecurse			= 0x0020;

	//
	// ATS Support option flags
	//
	/*! No options */
	const ATSOption	kATSSupportNone				= 0x0000;
	/*! Supports file associations */
	const ATSOption	kATSSupportFileAssoc		= 0x0001;
	/*! Supports browse provider files */
	const ATSOption	kATSSupportExploreProvider	= 0x0002;

	//
	// ATS Client Status Text flags
	//
	/*! Status not specified */
	const ATSStatus	kATSStatusTextNone			= 0x0000;
	/*! Informational message */
	const ATSStatus	kATSStatusTextInfo			= 0x0001;
	/*! Warning message */
	const ATSStatus	kATSStatusTextWarning		= 0x0002;
	/*! Error message */
	const ATSStatus	kATSStatusTextError			= 0x0004;
	/*! Display dialog with message */
	const ATSStatus	kATSStatusTextPrompt		= 0x0008;

	//
	// ATS Client Option flags
	//
	/*! No options */
	const ATSOption	kATSClientOptNone			= 0x0000;
	/*! All dialogs should be created with AlwaysOnTop state */
	const ATSOption	kATSClientOptUIAlwaysOnTop	= 0x0001;

};

///////////////////////////////////////////////////////////////////////////////
//
// class ATSFileEntry
//
///////////////////////////////////////////////////////////////////////////////

//! \brief ATS File Object
/*! ATSFileEntry describes a file monitored by the Asset Tracking System.
It contains information about the status, children (or dependent) files,
and any attachment files to this file.  Children define the file dependency
tree for a particular ATSFileEntry.  Attachments do not define a hierarchy
and are single entries associated with the ATSFileEntry.  Attachments are not
considered a dependency and are simply files associated with the ATSFileEntry.
\see ATSFileList
*/
class ATSFileEntry : public MaxHeapOperators
{
public:
	//! \brief Constructor
	ATSExport ATSFileEntry();
	//! \brief Destructor
	ATSExport virtual ~ATSFileEntry();

	//! \brief Get file name
	/*! \return string containing the filename
	*/
	ATSExport virtual const MCHAR*	GetFile();
	//! \brief Set file name
	/*! \param[in] szFile string containing the filename
	*/
	ATSExport virtual void			SetFile( const MCHAR *szFile );

	//! \brief Get the In Flags for this file
	/*! The In flags are typically used to select, activate, exclude, etc. an ATSFileEntry
	within a ATSFileList.
	\return In flags (ATSStatus)
	*/
	ATSExport virtual ATSStatus		GetFlagsIn();
	//! \brief Set the In Flags for this file
	/*! The In flags are typically used to select, activate, exclude, etc. an ATSFileEntry
	within a ATSFileList.
	\param[in] dwFlagsIn In flags
	*/
	ATSExport virtual void			SetFlagsIn( ATSStatus dwFlagsIn );

	//! \brief Get the client pointer associated with this file
	/*! Client pointer specified for this file
	\return client pointer (ATSClientPtr)
	*/
	ATSExport virtual ATSClientPtr	GetClientPtr();
	//! \brief Set the client pointer associated with this file
	/*! \param[in] pClientPtr client pointer
	*/
	ATSExport virtual void			SetClientPtr( ATSClientPtr pClientPtr );

	//! \brief Get the Out Flags for this file
	/*! The Out flags are typically set by an ATS Provider to specify the version
	control status of this file.
	\return Out flags
	*/
	ATSExport virtual ATSStatus		GetFlagsOut();
	//! \brief Set the Out Flags for this file
	/*! The Out flags are typically set by an ATS Provider to specify the version
	control status of this file.
	\param[in] dwFlagsOut Out flags (ATSStatus)
	*/
	ATSExport virtual void			SetFlagsOut( ATSStatus dwFlagsOut );

	//! \brief Add child
	/*! \param[in] pEntry Pointer to ATSFileEntry object to add as child
	\return New number of children
	*/
	ATSExport virtual UINT			AddChild( ATSFileEntry *pEntry );
	//! \brief Number of children
	/*! \return Number of children
	*/
	ATSExport virtual UINT			NumChildren();
	//! \brief Get child
	/*! \param[in] iIndex 0-based index of child
	\return pointer to ATSFileEntry object
	*/
	ATSExport virtual ATSFileEntry*	GetChild( UINT iIndex );
	//! \brief Remove child
	/*! \param[in] iIndex 0-based index of child
	\return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveChild( UINT iIndex );
	//! \brief Remove all child
	/*! \return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveAllChildren();

	//! \brief Add attachment
	/*! \param[in] szFile string that contains file name
	\param[in] pEntryParent default is NULL. Pointer to parent ATSFileEntry object
	\param[in] dwFlagsIn default is kATSStatusActive. ATSStatus value that specified initial In flags.
	\param[in] pClientPtr default is NULL. Client defined pointer.
	\return New number of attachments
	*/
	ATSExport virtual UINT			AddAttachment( const MCHAR *szFile, ATSFileEntry *pEntryParent = NULL, ATSStatus dwFlagsIn = ATS::kATSStatusActive, ATSClientPtr pClientPtr = NULL );
	//! \brief Add attachment
	/*! \param[in] pEntry Pointer to ATSFileEntry object to add as an attachment.  ATSFileEntry object will
	be copied.
	\return New number of attachments
	*/
	ATSExport virtual UINT			AddAttachment( ATSFileEntry *pEntry );
	//! \brief Number of attachments
	/*! \return number of attachments
	*/
	ATSExport virtual UINT			NumAttachments();
	//! \brief Get attachment
	/*! \param[in] iIndex 0-based index of attachment
	\return pointer to ATSFileEntry object
	*/
	ATSExport virtual ATSFileEntry*	GetAttachment( UINT iIndex );
	//! \brief Remove attachment
	/*! \param[in] iIndex 0-based index of attachment
	\return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveAttachment( UINT iIndex );
	//! \brief Remove all attachments
	/*! \return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveAllAttachments();

	//! \brief Set parent
	/*! \param[in] pParent Pointer to ATSFileEntry object to set as parent
	*/
	ATSExport virtual void			SetParent( ATSFileEntry *pParent );
	//! \brief Get parent
	/*! \return Pointer to ATSFileEntry object
	*/
	ATSExport virtual ATSFileEntry*	GetParent();

protected:
	ATSString						mFile;
	ATSStatus						mFlagsIn;
	ATSStatus						mFlagsOut;
	ATSClientPtr					mClientPtr;
	std::vector< ATSFileEntry* >	mChildren;
	ATSFileEntry*					mpParent;
	std::vector< ATSFileEntry* >	mAttachments;
};

///////////////////////////////////////////////////////////////////////////////
//
// class ATSFileList
//
///////////////////////////////////////////////////////////////////////////////

//! \brief ATS File List
/*! ATSFileList is a list of ATSFileEntry objects.  It maintains three lists:
1) List of ATSFileEntry objects added, 2) List of root ATSFileEntry objects (those without parents),
and 3) List of Active ATSFileEntry objects as specified and order by BuildActiveList method.
\see ATSFileEntry
*/
class ATSFileList : public MaxHeapOperators
{
public:
	//! \brief Constructor
	ATSExport ATSFileList();
	//! \brief Destructor
	ATSExport virtual ~ATSFileList();

	//
	// Access to all file entries
	//

	//! \brief Create and add new ATSFileEntry
	/*! \param[in] szFile string containing filename
	\param[in] pEntryParent default is NULL. Pointer to parent ATSFileEntry object
	\param[in] dwFlagsIn default is kATSStatusActive. ATSStatus value that specified initial In flags.
	\param[in] pClientPtr default is NULL. Client defined pointer.
	\return pointer to ATSFileEntry object or NULL if failure
	*/
	ATSExport virtual ATSFileEntry*	CreateEntry( const MCHAR *szFile, ATSFileEntry *pEntryParent = NULL, ATSStatus dwFlagsIn = ATS::kATSStatusActive, ATSClientPtr pClientPtr = NULL );
	//! \brief Copy and add new ATSFileEntry
	/*! \param[in] atsFileEntry This ATSFileEntry object is copied and added to the entry list.
	\return pointer to ATSFileEntry object or NULL if failure
	*/
	ATSExport virtual ATSFileEntry*	CreateEntry( ATSFileEntry &atsFileEntry );
	//! \brief Remove entry
	/*! \param[in] pEntry Pointer to ATSFileEntry object.
	\return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveEntry( ATSFileEntry *pEntry );
	//! \brief Number of entries
	/*! \return number of entries in file list
	*/
	ATSExport virtual UINT			NumEntries();
	//! \brief Remove entry by index
	/*! \param[in] iIndex 0-based index into file entry list
	\return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveEntry( UINT iIndex );
	//! \brief Remove all entries
	/*! \return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveAll();
	//! \brief Get entry
	/*! \param[in] iIndex 0-based index into file entry list
	\return pointer to ATSFileEntry object or NULL if failure
	*/
	ATSExport virtual ATSFileEntry*	GetFileEntry( UINT iIndex );

	//
	// Access to active file entries
	//

	//! \brief Build an active ATSFileEntry list
	/*! This method builds (or rebuilds) the active file list as defined by a status and traversal criteria.
	\param[in] dwFlagsIn default kATSStatusActive.  Only add file entries that contain this flag in their In flags.
	\param[in] bDepthFirstTraversal default false.  When false, the active file list will be generated by an inorder
	traversal of the file list.  When true, the list will be generated using a depth first traversal as defined by
	the ATSFileEntry hierarchy.  When performing IATSProvider in which dependent files need to be added or checked in
	first, the active file list should be built with bDepthFirstTraversal set to true.
	\return number of files in active list
	*/
	ATSExport virtual UINT			BuildActiveList( ATSStatus dwFlagsIn = ATS::kATSStatusActive, bool bDepthFirstTraversal = false );
	//! \brief Add entry to active list
	/*! \param[in] pEntry Pointer to ATSFileEntry object to add to active list.
	\return number of files in active list
	*/
	ATSExport virtual UINT			AddActiveEntry( ATSFileEntry *pEntry );
	//! \brief Remove active entry
	/*! \param[in] pEntry Pointer to ATSFileEntry object.
	\return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveActiveEntry( ATSFileEntry *pEntry );
	//! \brief Number of active entries
	/*! \return number of active entries in file list
	*/
	ATSExport virtual UINT			NumActiveEntries();
	//! \brief Get active entry
	/*! \param[in] iIndex 0-based index into active file entry list
	\return pointer to ATSFileEntry object or NULL if failure
	*/
	ATSExport virtual ATSFileEntry*	GetActiveFileEntry( UINT iIndex );

	//
	// Access to root level entries
	//

	//! \brief Number of root entries
	/*! \return number of root entries in file list
	*/
	ATSExport virtual UINT			NumRootEntries();
	//! \brief Get root entry
	/*! \param[in] iIndex 0-based index into root file entry list
	\return pointer to ATSFileEntry object or NULL if failure
	*/
	ATSExport virtual ATSFileEntry*	GetRootEntry( UINT iIndex );

protected:
	std::vector<ATSFileEntry*>		mATSFiles;
	std::vector<ATSFileEntry*>		mATSRoots;
	std::vector<ATSFileEntry*>		mATSActiveFiles;
};

///////////////////////////////////////////////////////////////////////////////
//
// class IATSClientAccess
//
///////////////////////////////////////////////////////////////////////////////

//! \brief Access to client of ATS Provider
/*! This interface provides runtime access to the client application.
\see ATSClientInfo
*/
class IATSClientAccess : public MaxHeapOperators
{
public:
	//! \brief Send status message to client
	/*! \param[in] szText string contaning status message
	\param[in] dwStatus Currently not used. status of message. 
	\return true if success, false if failure
	*/
	virtual bool		SendStatusText( const MCHAR *szText, ATSStatus dwStatus = ATS::kATSStatusTextNone ) = 0;
	//! \brief Window handle of client
	/*! \return HWND handle of parent window
	*/
	virtual HWND		GetParentWindow() = 0;
	//! \brief Get UI colors
	/*! Allows an ATS Provider to access custom application color settings.
	\param[in] iIndex same index used in Window's GetSysColor method.
	\return color value.
	*/
	virtual DWORD		GetUIColor( int iIndex ) = 0;
	//! \brief Get silent mode status
	/*! \return true if in silent mode, false if not.
	*/
	virtual bool		SilentMode() = 0;
	//! \brief Get ATS Client options
	/*! \return ATS option flags
	*/
	virtual ATSOption	GetClientOptions() = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// class ATSClientInfo
//
///////////////////////////////////////////////////////////////////////////////

//! \brief Client information for ATS Providers
/*! This object provides application specific information to the ATS Provider
*/
class ATSClientInfo : public MaxHeapOperators
{
public:
	//! \brief Constructor
	ATSExport ATSClientInfo();
	//! \brief Destructor
	ATSExport virtual ~ATSClientInfo();

	//! \brief Get client name
	/*! The client name is typically the name of the application utilizing the ATS Provider.
	\return string containing the name of the client.
	*/
	ATSExport virtual const MCHAR*	GetClientName();
	//! \brief Set client name
	/*! The client name is typically the name of the application utilizing the ATS Provider.
	\param[in] szClientName string containing the name of the client.
	*/
	ATSExport virtual void			SetClientName( const MCHAR *szClientName );

	//! \brief Get user name
	/*! This is optional and might be a blank or NULL string.
	\return string containing the name of the user.
	*/
	ATSExport virtual const MCHAR*	GetUserName();
	//! \brief Set user name
	/*! \param[in] szUserName string containing the name of the user.
	*/
	ATSExport virtual void			SetUserName( const MCHAR *szUserName );

	//! \brief Window handle of client
	/*! \return HWND handle of parent window
	*/
	ATSExport virtual HWND			GetParentWindow();
	//! \brief Set window handle of client
	/*! \param[in] hHwnd HWND handle of parent window
	*/
	ATSExport virtual void			SetParentWindow( HWND hHwnd );

	//! \brief Get Provider plugin path location
	/*! This is used to specify a location in which additional plugins would reside.
	\return string containing the path
	*/
	ATSExport virtual const MCHAR*	GetProviderPluginPath();
	//! \brief Set Provider plugin path location
	/*! This is used to specify a location in which additional plugins would reside.
	\param[in] szPath string containing the path
	*/
	ATSExport virtual void			SetProviderPluginPath( const MCHAR *szPath );

	//! \brief Get Provider configuration file path location
	/*! This is used to specify a location in which additional configuration files would reside.
	\return string containing the path
	*/
	ATSExport virtual const MCHAR*	GetConfigFilePath();
	//! \brief Set Provider configuration file path location
	/*! This is used to specify a location in which additional configuration files would reside.
	\param[in] szPath string containing the path
	*/
	ATSExport virtual void			SetConfigFilePath( const MCHAR *szPath );

	//! \brief Get IATSClientAccess interface
	/*! \see IATSClientAccess
	\return pointer to IATSClientAccess interface
	*/
	ATSExport virtual IATSClientAccess* GetIATSClientAccess();
	//! \brief Set IATSClientAccess interface
	/*! \see IATSClientAccess
	\param[in] pIATSClientAccess pointer to IATSClientAccess interface
	*/
	ATSExport virtual void			SetIATSClientAccess( IATSClientAccess* pIATSClientAccess );

protected:
	ATSString					mClientName;
	ATSString					mUserName;
	ATSString					mPluginPath;
	ATSString					mConfigPath;
	HWND						mHwnd;
	IATSClientAccess*			mpIATSClientAccess;
};

///////////////////////////////////////////////////////////////////////////////
//
// class ATSUserEntry
//
///////////////////////////////////////////////////////////////////////////////

//! \brief ATS User Object
/*! ATSUserEntry describes public data about a provider's user
\see ATSUserList
*/
class ATSUserEntry : public MaxHeapOperators
{
public:
	//! \brief Constructor
	ATSExport ATSUserEntry();
	//! \brief Destructor
	ATSExport virtual ~ATSUserEntry();

	//! \brief Get user name
	/*! \return string containing the user's name
	*/
	ATSExport virtual const MCHAR*	GetUserName();
	//! \brief Set user name
	/*! \param szUserName string containing the user's name
	*/
	ATSExport virtual void			SetUserName( const MCHAR *szUserName );

	//! \brief Get user's first name
	/*! \return string containing the user's first name
	*/
	ATSExport virtual const MCHAR*	GetFirstName();
	//! \brief Set user's first name
	/*! \param szFirstName string containing the user's first name
	*/
	ATSExport virtual void			SetFirstName( const MCHAR *szFirstName );

	//! \brief Get user's last name
	/*! \return string containing the user's last name
	*/
	ATSExport virtual const MCHAR*	GetLastName();
	//! \brief Set user's last name
	/*! \param szLastName string containing the user's last name
	*/
	ATSExport virtual void			SetLastName( const MCHAR *szLastName );

	//! \brief Get user's email
	/*! \return string containing the user's email
	*/
	ATSExport virtual const MCHAR*	GetEmail();
	//! \brief Set user's email
	/*! \param szEmail string containing the user's email
	*/
	ATSExport virtual void			SetEmail( const MCHAR *szEmail );

protected:
	ATSString						mUserName;
	ATSString						mFirstName;
	ATSString						mLastName;
	ATSString						mEmail;
};

///////////////////////////////////////////////////////////////////////////////
//
// class ATSUserList
//
///////////////////////////////////////////////////////////////////////////////

//! \brief ATS User List
/*! Maintains a list of ATSUserEntry objects
\see ATSUserEntry
*/
class ATSUserList : public MaxHeapOperators
{
public:
	//! \brief Constructor
	ATSExport ATSUserList();
	//! \brief Destructor
	ATSExport virtual ~ATSUserList();

	//! \brief Create and add new ATSUserEntry
	/*! \param[in] szUserName string containing user name
	\return pointer to ATSUserEntry object or NULL if failure
	*/
	ATSExport virtual ATSUserEntry*	CreateEntry( const MCHAR *szUserName );
	//! \brief Copy and add new ATSUserEntry
	/*! \param[in] atsUserEntry This ATSUserEntry object is copied and added to the entry list.
	\return pointer to ATSUserEntry object or NULL if failure
	*/
	ATSExport virtual ATSUserEntry*	CreateEntry( ATSUserEntry &atsUserEntry );
	//! \brief Remove entry
	/*! \param[in] pEntry Pointer to ATSUserEntry object.
	\return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveEntry( ATSUserEntry *pEntry );
	//! \brief Number of entries
	/*! \return number of entries in file list
	*/
	ATSExport virtual UINT			NumEntries();
	//! \brief Remove entry by index
	/*! \param[in] iIndex 0-based index into user entry list
	\return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveEntry( UINT iIndex );
	//! \brief Remove all entries
	/*! \return true if success, false if failure
	*/
	ATSExport virtual bool			RemoveAll();
	//! \brief Get entry
	/*! \param[in] iIndex 0-based index into user entry list
	\return pointer to ATSUserEntry object or NULL if failure
	*/
	ATSExport virtual ATSUserEntry*	GetUserEntry( UINT iIndex );

protected:
	std::vector<ATSUserEntry*>		mATSUsers;
};

///////////////////////////////////////////////////////////////////////////////
//
// class ATSExploreProviderParams
//
///////////////////////////////////////////////////////////////////////////////

//! \brief ATS Explore Provider Object
/*! ATSExploreProviderParams describes the parameters for the IATSProvider::ExploreProvider method
\see IATSProvider
*/
class ATSExploreProviderParams : public MaxHeapOperators
{
public:
	//! \brief Constructor
	ATSExport ATSExploreProviderParams();
	//! \brief Destructor
	ATSExport virtual ~ATSExploreProviderParams();

	//! \brief Get title
	/*! Defines the title for the explore provider dialog
	\return string containing the title
	*/
	ATSExport virtual const MCHAR*	GetTitle();
	//! \brief Set title
	/*! Defines the title for the explore provider dialog
	\param[in] szTitle string containing the filename
	*/
	ATSExport virtual void			SetTitle( const MCHAR* szTitle );

	//! \brief Get multiple selections state
	/*! Defines if multiple selections should be allowed within the explorer dialog
	\return true if multiple selections are allowed, false if not
	*/
	ATSExport virtual bool			GetMultipleSelections();
	//! \brief Set multiple selections state
	/*! Defines if multiple selections should be allowed within the explorer dialog
	\param[in] bMultipleSelections true if multiple selections are allowed, false if not
	*/
	ATSExport virtual void			SetMultipleSelections( bool bMultipleSelections );

	//! \brief Get download files state
	/*! Defines if the provider should automatically download the files selected
	\return true if download should occur, false if not
	*/
	ATSExport virtual bool			GetDownloadFiles();
	//! \brief Set download files state
	/*! Defines if the provider should automatically download the files selected
	\param[in] bDownload true if download should occur, false if not
	*/
	ATSExport virtual void			SetDownloadFiles( bool bDownload );

	//! \brief Add filter for explorer dialog
	/*! A filter defines the "Type of File" that should be available for selection and/or display.
	Some examples:
	For all files:  AddFilter( "All files (*.*)", "*.*" );
	For jpeg files:  AddFilter( "JPEG (*.jpg,*.jpeg)", "*.jpg;*.jpeg" );
	\param[in] szName name of filter as it should be displayed in the dialog.  Example: "Add files (*.*)"
	\param[in] szExtensions List of extensions.  Example: "*.jpg;*.jpeg"
	\return new number of filters
	*/
	ATSExport virtual UINT			AddFilter( const MCHAR* szName, const MCHAR* szExtensions );
	//! \brief Number of filters
	/*! \return number of filters
	*/
	ATSExport virtual UINT			NumFilters();
	//! \brief Get filter name
	/*! \param[in] iIndex 0-based index of filters
	\return string containing filter name
	*/
	ATSExport virtual const MCHAR*	GetFilterName( UINT iIndex );
	//! \brief Get filter extensions
	/*! \param[in] iIndex 0-based index of filters
	\return string containing filter extensions
	*/
	ATSExport virtual const MCHAR*	GetFilterExtensions( UINT iIndex );

protected:
	ATSString					mTitle;
	DWORD						mState;
	std::vector< ATSString >	mFilters;
	std::vector< ATSString >	mExtensions;
};

///////////////////////////////////////////////////////////////////////////////
//
// class IATSProvider
//
///////////////////////////////////////////////////////////////////////////////

//! \brief ATS Provider Interface
/*! This is the main interface for interacting with an ATS Provider.  It provides for
basic version control functionality, retrieval of status, connections, dependency
reporting, etc.
\see IATSMax, ATSFileList, ATSClientInfo, ATSUserList
*/
class IATSProvider : public MaxHeapOperators
{
public:
	//! \brief Destructor
	virtual ~IATSProvider() {}

	//! \brief Get provider name
	/*! \return string containing provider name
	*/
	virtual const MCHAR*	GetProviderName() = 0;

	//! \brief Get provider's support options
	/*! This allows the provider to specify whether or not they support some of the more
	advanced version control method, such as dependency reporting and exploring.
	See kATSSupport* flags.
	\return Support option flags (ATSOption)
	*/
	virtual ATSOption		GetSupportOptions() = 0;

	//! \brief Get initialization status
	/*! \return true if the provider has been initialized, false if it has not.
	*/
	virtual bool			IsInitialized() = 0;
	//! \brief Get project status
	/*! \return true if the provider has a project open, false if it has not.
	*/
	virtual bool			IsProjectOpen() = 0;

	//! \brief Initialize
	/*! This is called when the user has decided to connect or logon to the provider.  At this point,
	the provider should be prepared to open a project, launch provider, explorer provider, or
	show options.  The provider might also ask the user to login if neccessary, although that
	can be delayed until a project is opened.
	\param[in] atsClientInfo reference to ATSClientInfo object.  Any data of interested should be copied
	locally to the provider.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent is valid.
	\return ATSResult value
	*/
	virtual ATSResult		Initialize( ATSClientInfo &atsClientInfo, ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Uninitialize
	/*! This is called when the user has decided to disconnect or logout of the provider.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent is valid.
	\return ATSResult value
	*/
	virtual ATSResult		Uninitialize( ATSOption atsOptions = ATS::kATSOptNone ) = 0;

	//! \brief Open project
	/*! This is called when the client has opened a project.  At this point a project path has been determined from
	document or document related project files.  For certain providers, the specified path might be used to determine
	if the project can be resolved to a workspace by the provider.
	\param[in,out] szPath string containing the path to the project.  This typically is the path in which the document
	was opened.
	\param[in] szComment string containing any initial or default comment for the project.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent is valid.
	\return ATSResult value
	*/
	virtual ATSResult		OpenProject( MCHAR *szPath, const MCHAR *szComment, ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Close project
	/*! This is called when the client has closed a project.  This occurs when the user has opened a new document or is
	closing down the client application.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent is valid.
	\return ATSResult value
	*/
	virtual ATSResult		CloseProject( ATSOption atsOptions = ATS::kATSOptNone ) = 0;

	//! \brief Get working folder
	/*! \return string containing current working folder used to resolve project files
	*/
	virtual const MCHAR*	GetWorkingFolder() = 0;

	//! \brief Set working folder
	/*! The working folder for most providers is used to map the root folder in the version provider's file system
	to the local file system.
	\param[in] szWorkingFolder string containing the new working folder.
	\return ATSResult value
	*/
	virtual ATSResult		SetWorkingFolder( const MCHAR *szWorkingFolder ) = 0;

	//! \brief Checkin files
	/*! Active files in the ATSFileList object should be checked in to the provider.
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[in] szComment string containing the comment to include with this version.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent and kATSOptKeepCheckedOut are valid.
	\return ATSResult value
	*/
	virtual ATSResult		Checkin( ATSFileList &atsFileList, const MCHAR *szComment, ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Checkout files
	/*! Active files in the ATSFileList object should be checked out from the provider.
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[in] szComment string containing the comment to include with this version.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent and kATSOptReplaceLocalCopy are valid.
	\return ATSResult value
	*/
	virtual ATSResult		Checkout( ATSFileList &atsFileList, const MCHAR *szComment, ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Undo checkout
	/*! Active files in the ATSFileList object should no longer be checked out by the provider.
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent and kATSOptReplaceLocalCopy are valid.
	\return ATSResult value
	*/
	virtual ATSResult		UndoCheckout( ATSFileList &atsFileList, ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Get latest
	/*! Active files in the ATSFileList object should have their latest version downloaded.
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent is valid.
	\return ATSResult value
	*/
	virtual ATSResult		GetLatest( ATSFileList &atsFileList, ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Get version range
	/*! Get the range of version numbers.  If multiple active file entries are in atsFileList, the verMin
	is the minimum of all the files, and verMax is the maximum of all the files.
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[out] verMin minimum version number
	\param[out] verMax maximum version number
	\param[in] atsOptions default value kATSOptNone.
	\return ATSResult value
	*/
	virtual ATSResult		GetVersionRange( ATSFileList &atsFileList, ATSVersion &verMin, ATSVersion &verMax, ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Get version
	/*! Downloads the specified version for the active files in the ATSFileList object.
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[in] ver version number
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent is valid.
	\return ATSResult value
	*/
	virtual ATSResult		GetVersion( ATSFileList &atsFileList, ATSVersion ver, ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Add files
	/*! Active files in the ATSFileList object should be added to the provider.
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[in] szComment string containing the comment to include with this version.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent and kATSOptKeepCheckedOut are valid.
	\return ATSResult value
	*/
	virtual ATSResult		AddFiles( ATSFileList &atsFileList, const MCHAR *szComment, ATSOption atsOptions = ATS::kATSOptNone ) = 0;

	//! \brief Show version history
	/*! The version history of the active files in the ATSFileList object should be shown.  If multiple histories can not
	be displayed, the first active file should be displayed.
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent is valid.
	\return ATSResult value.  If the history dialog resulted in a version change of one of the file, kATSResSuccessReload
	should be returned.
	*/
	virtual ATSResult		History( ATSFileList &atsFileList, ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Show file properties
	/*! The properties of the active files in the ATSFileList object should be shown.  If multiple file properties can not
	be displayed, the first active file should be displayed.
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent is valid.
	\return ATSResult value
	*/
	virtual ATSResult		Properties( ATSFileList &atsFileList, ATSOption atsOptions = ATS::kATSOptNone ) = 0;

	//! \brief Get file status
	/*! The status of the active files in the ATSFileList object should be updated.  The status should be placed
	in the Out flags of the ATSFileEntry.
	\see ATSFileEntry
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent is valid.
	\return ATSResult value
	*/
	virtual ATSResult		GetFileStatus( ATSFileList &atsFileList, ATSOption atsOptions = ATS::kATSOptNone ) = 0;

	//! \brief Get name of users that have files locked
	/*! For each active file in the ATSFileList object, a corresponding atsUserList entry will be created.
	The provider will fill in an ATSUserEntry object with information about the user that has the
	active file locked or checked out.  If a file in the active list is not locked, the provider will
	create a blank ATSUserEntry object in place.
	\see ATSUserEntry
	\param[in,out] atsFileList reference to ATSFileList object.  The active file list should be used.
	\param[out] atsUserList reference to ATSUserList object.  This will be populated by the provider.
	\param[in] atsOptions default value kATSOptNone.
	\return ATSResult value
	*/
	virtual ATSResult		GetLockedFileUserList( ATSFileList &atsFileList, ATSUserList &atsUserList, ATSOption atsOptions = ATS::kATSOptNone ) = 0;

	//! \brief Explore provider
	/*! If supported, the provider should display a dialog to allow the user to explorer the contents of the
	provider.  The file or files selected by the user will be returned and optionally downloaded.
	\see ATSExploreProviderParams
	\param[in,out] atsFileList reference to ATSFileList object.  Active entries should be created for the
	selected files.
	\param[in] exploreParams reference to ATSExploreProviderParams object that defines the parameters for the
	explorer provider dialog.
	\param[in] atsOptions default value kATSOptNone.
	\return ATSResult value
	*/
	virtual ATSResult		ExploreProvider( ATSFileList &atsFileList, ATSExploreProviderParams &exploreParams, ATSOption atsOptions = ATS::kATSOptNone ) = 0;

	//! \brief Show provider options
	/*! The provider should optional display a dialog containing any options.
	\param[in] atsOptions default value kATSOptNone.
	\return ATSResult value.  If any change in options might affect the status of the files, kATSResSuccessReload should be returned.
	*/
	virtual ATSResult		ShowProviderOptions( ATSOption atsOptions = ATS::kATSOptNone ) = 0;
	//! \brief Launch provider
	/*! The provider should optional launch an associated client application.
	\param[in] atsOptions default value kATSOptNone.
	\return ATSResult value.  If any change in options might affect the status of the files, kATSResSuccessReload should be returned.
	*/
	virtual ATSResult		LaunchProvider( ATSOption atsOptions = ATS::kATSOptNone ) = 0;

	//! \brief Get associated files
	/*! This method provides for specifying file associations.  If supported, the provider should add the specified associated files
	for each file in the list as child for that ATSFileEntry.  It should clear any children already in the list before adding its own.
	\param[in,out] atsFileList reference to ATSFileList object.  If supported, the provider should add the specified associated files
	for each file in the list as child for that ATSFileEntry.  It should clear any children already in the list before adding its own.
	\param[in] atsOptions default value kATSOptNone.  kATSOptSilent, kATSOptAssocChildren, kATSOptAssocParents, kATSOptAssocRecurse and are valid.
	\return ATSResult value.
	*/
	virtual ATSResult		GetAssociatedFiles( ATSFileList &atsFileList, ATSOption atsOptions = ATS::kATSOptNone ) = 0;

	//! \brief Is provider using MSSCC interface
	/*! This specifies whether an ATS Provider is directly supporting the IATSProvider interface or if the ATS core is supplying
	IATSProvider support through an MSSCC interface.
	\return true if MSSCC dll, false if not.
	*/
	virtual bool			IsATSMSSCCWrapper() = 0;
};

