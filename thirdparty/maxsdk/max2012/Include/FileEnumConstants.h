//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once

/*! \defgroup EnumAuxFiles Auxilliary File Enumeration Flags
Some scene entities depend on data files other then .max files. These files 
are referred to as "auxiliary" files. The following are flags that used to 
customize the enumeration of these files. See Animatable::EnumAuxFiles(), 
ReferenceMaker::EnumAuxFiles() and Interface::EnumAuxFiles() for more details. */
//@{

//! \brief Enumerate inactive files
/*! Inactive files are those that aren't being used currently. For instance, 
a texture map file that is present, but not activated in the materials editor 
user interface, is considered inactive. */
#define FILE_ENUM_INACTIVE 		(1<<0)
//! \brief Enumerate video post files.
#define FILE_ENUM_VP			(1<<1)
//! \brief Enumerate render files. 
#define FILE_ENUM_RENDER		(1<<2)
//! \brief Enumerate ALL files.
#define FILE_ENUM_ALL  (FILE_ENUM_INACTIVE|FILE_ENUM_VP|FILE_ENUM_RENDER)
//! \brief Enumerate references to be saved to scene files. 
#define FILE_ENUM_FILE_SAVE		(1<<3)
//! \brief Enumerate missing files only
#define FILE_ENUM_MISSING_ONLY	(1<<8)
//! \brief Just numerate 1st file named by an IFL (Image File List) if missing
/*!  If the flag is set, then any files that are needed only for viewport display 
(not for rendering) will be excluded from the enumeration. This is useful in 
render slave mode, when you do not want to consider a render a failure just 
because some viewport-only files are missing. */
#define FILE_ENUM_1STSUB_MISSING (1<<9)
//! \brief Do not enumerate references.
#define FILE_ENUM_DONT_RECURSE   (1<<10)
//! \brief Do not enumerate things with flag A_WORK1 set.
#define FILE_ENUM_CHECK_AWORK1   (1<<11)
//! \brief Do not enumerate custom attributes.
#define FILE_ENUM_DONTCHECK_CUSTATTR  (1<<12)
//! \brief Skip files needed only for viewport rendering.
#define FILE_ENUM_SKIP_VPRENDER_ONLY (1<<13)
//! \brief The callback object passed through is a IEnumAuxAssetsCallback derived object.
#define FILE_ENUM_ACCESSOR_INTERFACE (1<<14)
//! \brief Internal use only.
#define FILE_ENUM_RESERVED_1 (1<<15)
//! \brief To enumerate all active but missing files.
#define FILE_ENUM_MISSING_ACTIVE (FILE_ENUM_VP|FILE_ENUM_RENDER|FILE_ENUM_MISSING_ONLY)
//! \brief To enumerate all active but missing files.
/*! But only enumerate first subfile pointed to by an IFL (Image File List).
\note Enumerating all of them can be very slow. */
#define FILE_ENUM_MISSING_ACTIVE1 (FILE_ENUM_MISSING_ACTIVE|FILE_ENUM_1STSUB_MISSING )

//@}	END OF EnumAuxFiles