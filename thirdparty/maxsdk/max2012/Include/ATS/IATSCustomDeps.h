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
// DESCRIPTION: Interface for ATS Custom Dependencies
// AUTHOR: Michael Russo - created April 27, 2005
//***************************************************************************/

#pragma once

#include "..\iFnPub.h"
#include "..\containers\Array.h"
#include "..\FileEnumConstants.h"

class AssetEnumCallback;

//! ID for IATSCustomDeps interface
#define IATSCUSTOMDEPS_INTERFACE	Interface_ID(0x23b7a30, 0x18b368ba)

///////////////////////////////////////////////////////////////////////////////
//
// class IATSCustomDeps
//
///////////////////////////////////////////////////////////////////////////////

//! \brief Interface to Asset Tracking Custom Dependencies
/*! This interface gives access to the list of files defined as Custom
Dependencies.  Custom Dependencies are reported as external file
dependencies.  Any file can be added to the Custom Dependency list.
*/
class IATSCustomDeps : public FPStaticInterface {

public:

	//! \brief Launch Custom Dependency dialog
	/*! \param[in] hParent HWND of parent window
	\return true if success, false if failure
	*/
	virtual bool			LaunchDialog( HWND hParent ) = 0;

	//! \brief Number of Custom Dependency files
	/*! \return Number of files
	*/
	virtual int				NumFiles() = 0;

	//! \brief Get file by index
	/*! \param[in] iIndex 0-based index into list of files.
	\return string containing the filename
	*/
	virtual const MCHAR*	GetFile( UINT iIndex ) = 0;

	//! \brief Get list of files
	/*! \param[out] files reference to a Tab of TSTRs.  This parameter
	will be filled with a copy of the Custom Dependency files.
	\return number files added to Tab.
	*/
	virtual int				GetFiles( MaxSDK::Array<MSTR> &files ) = 0;

	//! \brief Add file to list
	/*! \param[in] szFile string that contains a file name.  If the file is
	already contained in the list, it will not be added.
	\return index of szFile in the file list
	*/
	virtual int				AddFile( const MCHAR* szFile ) = 0;

	//! \brief Set file name in list
	/*! \param[in] iIndex 0-based index into list of files.
	\param[in] szFile string that contains a file name.
	\return true if success, false if failure
	*/
	virtual bool			SetFile( UINT iIndex, const MCHAR* szFile ) = 0;

	//! \brief Remove file from list by index
	/*! \param[in] iIndex 0-based index into list of files.
	\return true if success, false if failure
	*/
	virtual bool			RemoveFile( UINT iIndex ) = 0;

	//! \brief Remove file from list by name
	/*! \param[in] szFile string that contains a file name.
	\return true if success, false if failure
	*/
	virtual bool			RemoveFile( const MCHAR* szFile ) = 0;

	//! \brief Remove all files from list
	/*! \return true if success, false if failure
	*/
	virtual bool			RemoveAll() = 0;

	//! \brief Internal EnumAuxFiles method
	/*! Used internally to allow Custom Dependencies to be
	reported as external scene file dependencies.
	*/
	virtual void			EnumAuxFiles(AssetEnumCallback& nameEnum, DWORD flags = FILE_ENUM_ALL) = 0;

};

//! Access to IATSCustomDeps interface
#define GetIATSCustomDeps() ((IATSCustomDeps*)GetCOREInterface(IATSCUSTOMDEPS_INTERFACE))

