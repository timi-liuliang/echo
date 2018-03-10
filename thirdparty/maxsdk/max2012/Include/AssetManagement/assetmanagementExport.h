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
// DESCRIPTION: used for importing and exporting functions in assetmanagement.dll
// AUTHOR: Peter Ochodo
// DATE: 2008-07-16 
//***************************************************************************/

#pragma once

#ifdef BLD_ASSETMANAGEMENT
#define AssetMgmntExport __declspec( dllexport )
#else
#define AssetMgmntExport __declspec( dllimport )
#endif

