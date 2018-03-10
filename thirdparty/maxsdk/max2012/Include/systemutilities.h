//***************************************************************************
// Copyright 2008 Autodesk, Inc.  All rights reserved.
// Use of this software is subject to the terms of the Autodesk license agreement 
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.
//***************************************************************************

#pragma once

#ifndef _STRBASICS_
#	include "strbasic.h"				// MCHAR
#endif

#include "strclass.h"

/*! \brief Returns true if 3ds Max is running under a debbuger */
UtilExport bool	IsDebugging();

/*! \brief Returns the number of processors in the system */
UtilExport int	NumberOfProcessors();

/*! \brief Returns true if the current operating system is Windows 9x?, otherwise false */
UtilExport bool	IsWindows9x();

/*! \brief Returns true if the current operating system is Windows 98 or 2000, otherwise false */
UtilExport bool	IsWindows98or2000();

/*! \brief Returns the width of the screen (including multiple monitors) */
UtilExport int	GetScreenWidth();

/*! \brief Returns the height of the screen (including multiple monitors) */
UtilExport int GetScreenHeight();

/*! \brief Returns true if the current ANSI codepage is for an asian language */
UtilExport bool IsGetACPAsian();

namespace MaxSDK
{
namespace Util
{
	/*! \brief Returns the identifier of the language used by 3ds Max.
	 *
	 * This method complements MaxSDK::Util::GetLanguageTLA. 
	 *
	 * The language identifier is made up of a primary language identifier and a 
	 * sublanguage identifier, both of which are from a set pre-defined identifiers
	 * that are part of the Windows Platform SDK.
	 *
	 * Use WORD PRIMARYLANGID(WORD lgid) to retrieve the primary language identifier
	 * (such as LANG_ENGLISH, LANG_FRENCH, etc.), from the language identifier returned 
	 * by this method.
	 * Use WORD SUBLANGID(WORD lgid) to retrieve the sublanguage identifier
	 * (such as SUBLANG_DEFAULT, SUBLANG_CHINESE_SIMPLIFIED, etc.), from the language identifier 
	 * returned by this method.
	 * 
	 * For more information on language identifiers see 
	 * <a href="http://msdn.microsoft.com/en-us/library/ms776254(VS.85).aspx">National Language Support (on MSDN)</a>
	*/
	UtilExport WORD GetLanguageID();

	/*! \brief Returns the "tree letter acronym" (TLA) of the language used by 3ds Max.
	 *
	 * This method complements MaxSDK::Util::GetLanguageID. 
	 * For more information on TAL see 
	 * <a href="http://www.microsoft.com/globaldev/reference/winxp/langtla.mspx">List of Windows XP's Three Letter Acronyms for Languages</a>
	 *
	 * \return The TLA of the language used by 3ds Max.
	 */
	UtilExport const MCHAR* GetLanguageTLA();

	/*! \brief copies a string to the system wide clipboard
		\param text - The string to copy 
		\param hwnd - The Window that owns the original information to be copied*/
	UtilExport bool CopyToClipBoard(const MCHAR* text, HWND hwnd);

} // end namespace Util
} // end namespace MaxSDK

// CSIDL functions added 030110  --prs.

UtilExport HRESULT UtilGetFolderPath(
	HWND hwndOwner,	// just calls SFGetFolderPath()
	int nFolder, 
	HANDLE hToken, 
	DWORD dwFlags, 
	LPTSTR pszPath);

// negative indices, refer to specCSID[] array in systemutilities.cpp

//! \name AppPaths
//! <A name="app_paths"></A>
//! \brief These are the ID's of max's system paths
//@{
/*! System directory that points to \<max data\>/maps */
#define APP_MAP_DIR			   -1
#define APP_FOLIAGE_DIR		   -2
#define APP_XREF_DIR		   -3
#define APP_PLUGIN_INI_DIR	   -4
#define APP_STDPLUGS_DIR	   -5
#define APP_PLUGINS_DIR		   -6
#define APP_FILELINK_DIR	   -7
#define APP_CATALOGS_DIR	   -8
#define APP_CUI_SCRIPTS_DIR    -9	// added 030224  --prs.
// xavier robitaille | 03.02.05 | add textures dir. to bitmap paths
#ifdef TEXTURES_DIR_BMP_SEARCH_PATH
#define APP_INT_OFFSET		   -1
#define APP_TEXTURES_DIR	   -10
#else
#define APP_INT_OFFSET		   0
#endif
#define APP_CUI_DIR					-10 + APP_INT_OFFSET
#define APP_FX_DIR					-11 + APP_INT_OFFSET	//!< Returns the hardware shader directory set in Configure Paths
#define APP_CUI_WRITE_DIR			-12 + APP_INT_OFFSET
#define APP_MARKET_DEF_WRITE_DIR	-13 + APP_INT_OFFSET
#define APP_LAST_SPEC_DIR			-13 + APP_INT_OFFSET
//@}

//! \brief Returns a value indicating whether the application is configured to use User Profiles or not.
/*! The application can be configured to use User Profiles. 
 * \return true if the application uses User Profile directories
 */
UtilExport bool IsUsingProfileDirectories();

//! \brief Returns a value indicating whether the application is configured to use romaing User Profiles or not.
/*!  The application can be configured to use roaming User Profiles.  This value is dependent on 
	 whether the application uses User Profiles at all.  Even if this returns true, if IsUsingProfileDirectories()
	 returns false, then the application will default to the legacy path configuration locations under the application
	 root install directory. 
    \return true if the application uses User Profile directories */
UtilExport bool IsUsingRoamingProfiles();

//! \brief This function is used to convert a path relative to the 3ds max install to an absolute path. 
/*! This function is used to convert a path relative to the 3ds max install to an absolute path. 
	For instance, this will yield the absolute path of the downloads directory:
	\code 
	GetSpecDir(APP_DOWNLOAD_DIR, _M("downloads"), buf);\n 
	\endcode
	\param [in] index - The ID of the directory to return.  This value should be one of <a href="#app_paths">AppPaths</a>.
	\param [in] dirName - The subdirectory to look for
	\param [out] buf - The buffer to return the absolute path in.  This buffer should be large enough to 
		contain the maximum path length. */
UtilExport bool GetSpecDir(int index, const MCHAR *dirName, MCHAR *buf);	// get directory path

//! \brief This is a helper for creating standard windows CSIDL directories
/*! This function is a helper for creating standard windows CSIDL directories
	The list of possible root directories is shown in the windows header SHLOBJ.h. 
	In particular, the TryCSIDLDir() function can be used to create CSIDL_LOCAL_APPDATA 
	paths, where you can put application-specific data tailored to the current 
	windows user. 
	\param [in] index - The ID of the directory to return.  Should be one of the windows CSIDL defines.
	\param [in] dirName - specifies the relative pathname.
	\param [out] buf - The buffer to return the absolute path in.  This buffer should be large enough to 
		contain the maximum path length. */
UtilExport bool TryCSIDLDir(int csidl, const MCHAR *dirName, MCHAR *buf);	// create directory path

UtilExport BOOL IsSSE();

namespace MaxSDK
{
namespace Util
{
	/* \brief scans a parent folder and all sub-folders for a file
	   
	   this function scans a directory tree (a parent folder and all sub-folders) in search of a file with the file name 'filename'.
	   if filename is found, it returns true and an absolute path is saved in the parameter 'targetPath'. If 
	   'filename' is not found, it returns false and the parameter targetpath is left unchanged

	   param[in] treeroot absolute path to the parent directory
	   param[in] filename the file that you are searching for
	   param[out] the absolute path to the file you are searching for

	   /returns true if if filename is successfully found, false otherwise
	*/
	UtilExport bool ScanDirectoryTree(const MCHAR *treeroot, const MCHAR *filename, MSTR& targetpath );

} // end namespace Util
} // end namespace MaxSDK


