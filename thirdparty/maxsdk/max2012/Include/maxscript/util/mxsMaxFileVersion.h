//**************************************************************************/
// Copyright (c) 1998-2009 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/

#pragma once

#include "..\ScripterExport.h"
#include "..\..\plugapi.h"

/** 
 * \brief The 3ds Max file versions the maxscript save commands 
 * (saveMaxFile and saveNodes) work with.
 * These values are converted by GetMaxVerFromMxsMaxFileVer() to 
 * the ones in \ref 3dsMaxRelease such as MAX_RELEASE, etc
*/
enum EMxsMaxFileVersion 
{
	//! \brief Unsupported 3ds Max version, corresponds to MAX_RELEASE_UNSUPPORTED
	kMxsMaxFileVersion_Unsuported = -1,
	//! \brief A file version was not specified, 
	//! maxscript save commands default to saving to current file version (MAX_RELASE)
	kMxsMaxFileVersion_Unspecified = 0,
	//! \brief The current 3ds Max version, corresponds to MAX_RELEASE
	kMxsMaxFileVersion_Current = 1,
	//! \brief 3ds Max 2010 file version, corresponds to MAX_RELEASE_R12
	kMxsMaxFileVersion_2010 = 2010,
	//! \brief 3ds Max 2011 file version, corresponds to MAX_RELEASE_R13
	kMxsMaxFileVersion_2011 = 2011,
	//! \brief 3ds Max 2012 file version, corresponds to MAX_RELEASE_R14
	kMxsMaxFileVersion_2012 = 2012,
};

/** 
 * \brief Converts a "human readable" 3ds Max version to \ref 3dsMaxRelease.
 * \param mxsMaxFileVer 3ds Max version as it occurs in the product name
 * \return A value in \ref 3dsMaxRelease (such as MAX_RELEASE),
 * or MAX_RELEASE_UNSUPPORTED if a non-supported 3ds Max version is specified.
*/
ScripterExport long GetMaxVerFromMxsMaxFileVer(EMxsMaxFileVersion mxsMaxFileVer);

/** 
 * \brief Converts a given 3ds Max version number to a EMxsMaxFileVersion.
 * \param maxVersion 3ds Max version number
 * \return The version number as it occurs in the product name ,
 * or kMxsMaxFileVersion_Unsuported if a non-supported 3ds Max version is specified.
*/
ScripterExport EMxsMaxFileVersion GetMxsMaxFileVerFromMaxVer(long maxVersion);
